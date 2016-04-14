#include "parser.h"

std::vector<std::string> split_by_space(std::string in) {
    std::vector<std::string> vec;
    std::string curr = "";
    for_each(in.begin(), in.end(), [&] (char c) {
        if (c == ' ') {
            vec.push_back(curr);
            curr = "";
        } else if (c != '\n'){
            curr += c;
        }
    });
    vec.push_back(curr);
    return vec;
}

std::string hexdecode(std::string in) {
    std::string res = "";
    for (size_t i = 0; i < in.length(); i+=2) {
        std::string hexstr = "";
        hexstr += in[i];
        hexstr += in[i+1];
        if (hexstr.length() == 2) {
            res += std::stoi(hexstr, nullptr, 16);
        } else {
            ERR("wtf, this isn't hex: %s\n", hexstr.c_str());
        }
    }
    return res;
}

std::string hexencode(std::string in) {
    std::string res = "";
    char buf[3];
    for_each(in.begin(), in.end(), [&] (char c) {
        sprintf(buf, "%02x", c);
        res += buf;
    });
    return res;
}

// TODO: don't assume well-formed queries...
// TODO: parse will a FSM instead of this spaghetti
void parser::parse_and_execute(postman::Message query, std::shared_ptr<storage::DB> db) {
    DEBUG("Parsing Query: %s\n", query.data);
    auto ds = split_by_space(query.data);
    if (ds[0] == "ADD") { // ADD query
        try {
            if (ds[1] == "USER") {
                auto username = hexdecode(ds[2]);
                auto hashed_pass = ds[3];
                auto user = db->add_user(username, hashed_pass);
                auto reply = std::to_string(user.user_id) + ' ' +
                             hexencode(user.username) + ' ' +
                             user.hashed_pass + '\n' +
                             "DONE\n";
                DEBUG("Replying with: %s\n", reply.c_str());
                send(query.sockfd, reply.c_str(), reply.length(), 0);
            } else if (ds[1] == "POST") {
                auto user_id = std::stoi(ds[2]);
                auto content = hexdecode(ds[3]);
                auto timestamp = std::stoi(ds[4]);
                auto post = db->add_post(user_id, content, timestamp);
                auto reply = std::to_string(post.post_id) + ' ' +
                             std::to_string(post.user_id) + ' ' +
                             hexencode(post.content) + ' ' +
                             std::to_string(post.timestamp) + '\n' + 
                             "DONE\n";
                DEBUG("Replying with: %s\n", reply.c_str());
                send(query.sockfd, reply.c_str(), reply.length(), 0);
            } else if (ds[1] == "FOLLOW") {
                auto followed_id = std::stoi(ds[2]);
                auto follower_id = std::stoi(ds[3]);
                auto follow = db->add_follow(followed_id, follower_id);
                auto reply = std::to_string(follow.followed_id) + ' ' +
                             std::to_string(follow.follower_id) + '\n' + 
                             "DONE\n";
                DEBUG("Replying with: %s\n", reply.c_str());
                send(query.sockfd, reply.c_str(), reply.length(), 0);
            } else {
                ERR("Invalid table for ADD: %s\n", ds[1].c_str());
                // TODO: reply with info
                return;
            }
        } catch (storage::AlreadyExists ae) {
            std::string reply = "ALREADY EXISTS\nDONE\n"; // error, say already exists
            send(query.sockfd, reply.c_str(), reply.length(), 0);
        }
    } else if (ds[0] == "GET") {
        try {
            if (ds[1] == "USER" && ds[2] == "BY")  {
                if (ds[3] == "USERNAME") {
                    auto username = hexdecode(ds[4]);
                    auto user = db->get_user_by_username(username);
                    auto reply = std::to_string(user.user_id) + ' ' +
                                 hexencode(user.username) + ' ' +
                                 user.hashed_pass + '\n' + 
                                 "DONE\n";
                    DEBUG("Replying with: %s\n", reply.c_str());
                    send(query.sockfd, reply.c_str(), reply.length(), 0);
                } else if (ds[3] == "USER_ID") {
                    auto user_id = std::stoi(ds[4]);
                    auto user = db->get_user_by_user_id(user_id);
                    auto reply = std::to_string(user.user_id) + ' ' +
                                 hexencode(user.username) + ' ' +
                                 user.hashed_pass + '\n' + 
                                 "DONE\n";
                    DEBUG("Replying with: %s\n", reply.c_str());
                    send(query.sockfd, reply.c_str(), reply.length(), 0);
                } else {
                    ERR("Invalid field to GET USER BY: %s\n", ds[3].c_str());
                    return;
                }
            } else if (ds[1] == "POST" && ds[2] == "BY") {
                if (ds[3] == "USER_ID") {
                    auto user_id = std::stoi(ds[4]);
                    auto posts = db->get_posts_by_user(db->get_user_by_user_id(user_id));
                    for_each(posts.begin(), posts.end(), [&] (storage::Post post) {
                        auto reply = std::to_string(post.post_id) + ' ' +
                                     std::to_string(post.user_id) + ' ' +
                                     hexencode(post.content) + ' ' +
                                     std::to_string(post.timestamp) + '\n'; 
                        DEBUG("Replying with: %s\n", reply.c_str());
                        send(query.sockfd, reply.c_str(), reply.length(), 0);
                    });
                    std::string donemsg = "DONE\n";
                    send(query.sockfd, donemsg.c_str(), donemsg.length(), 0);
                } else if (ds[3] == "POST_ID") {
                    auto post_id = std::stoi(ds[4]);
                    auto post = db->get_post_by_post_id(post_id);
                    auto reply = std::to_string(post.post_id) + ' ' +
                                 std::to_string(post.user_id) + ' ' +
                                 hexencode(post.content) + ' ' +
                                 std::to_string(post.timestamp) + '\n' + 
                                 "DONE\n";
                    DEBUG("Replying with: %s\n", reply.c_str());
                    send(query.sockfd, reply.c_str(), reply.length(), 0);
                } else {
                    ERR("Invalid field to GET POST BY: %s\n", ds[3].c_str());
                    // TODO: reply with info
                    return;
                }
            } else if (ds[1] == "FOLLOWS" && ds[2] == "BY") {
                if (ds[3] == "FOLLOWER_ID") {
                    auto follower_id = std::stoi(ds[4]);
                    auto follows = db->get_follows_by_follower_id(follower_id);
                    for_each(follows.begin(), follows.end(), [&] (storage::Follow follow) {
                        auto reply = std::to_string(follow.followed_id) + ' ' +
                                     std::to_string(follow.follower_id) + '\n';
                        DEBUG("Replying with: %s\n", reply.c_str());
                        send(query.sockfd, reply.c_str(), reply.length(), 0);
                    });
                    std::string donemsg = "DONE\n";
                    send(query.sockfd, donemsg.c_str(), donemsg.length(), 0);
                } else if (ds[3] == "FOLLOWED_ID") {
                    auto followed_id = std::stoi(ds[4]);
                    auto follows = db->get_follows_by_followed_id(followed_id);
                    for_each(follows.begin(), follows.end(), [&] (storage::Follow follow) {
                        auto reply = std::to_string(follow.followed_id) + ' ' +
                                     std::to_string(follow.follower_id) + '\n';
                        DEBUG("Replying with: %s\n", reply.c_str());
                        send(query.sockfd, reply.c_str(), reply.length(), 0);
                    });
                    std::string donemsg = "DONE\n";
                    send(query.sockfd, donemsg.c_str(), donemsg.length(), 0);
                } else {
                    ERR("Invalid field to GET FOLLOWS BY: %s\n", ds[3].c_str());
                    // TODO: reply with info
                    return;
                }
            } else {
                ERR("Invalid table for GET: %s\n", ds[1].c_str());
                // TODO: reply with info
                return;
            }
        } catch (storage::NotFound nf) {
            std::string reply = "NOT FOUND\nDONE\n"; // error, say not found
            send(query.sockfd, reply.c_str(), reply.length(), 0);
        } catch (std::out_of_range oor) {
            ERR("Out of range: %s\n", oor.what());
            std::string reply = "DB ERROR\nDONE\n"; // database error
            send(query.sockfd, reply.c_str(), reply.length(), 0);
        }
    } else {
        ERR("Invalid query type %s\n", ds[0].c_str());
        // TODO: reply with info
        return;
    }
}
