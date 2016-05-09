#include "parser.h"

void dump_db(postman::Message query, std::shared_ptr<storage::DB> db) {
    std::string users_msg = "USERS:\n", posts_msg = "POSTS:\n", follows_msg = "FOLLOWS:\n", done_msg = "DONE\n";
    auto users = db->get_users();
    auto posts = db->get_posts();
    auto follows = db->get_follows();
    send(query.sockfd, users_msg.c_str(), users_msg.length(), 0);
    for_each(users.begin(), users.end(), [&] (storage::User user) {
        std::string user_str = storage::serialize_user(user) + '\n';
        send(query.sockfd, user_str.c_str(), user_str.length(), 0);
    });
    send(query.sockfd, posts_msg.c_str(), posts_msg.length(), 0);
    for_each(posts.begin(), posts.end(), [&] (storage::Post post) {
        std::string post_str = storage::serialize_post(post) + '\n';
        send(query.sockfd, post_str.c_str(), post_str.length(), 0);
    });
    send(query.sockfd, follows_msg.c_str(), follows_msg.length(), 0);
    for_each(follows.begin(), follows.end(), [&] (storage::Follow follow) {
        std::string follow_str = storage::serialize_follow(follow) + '\n';
        send(query.sockfd, follow_str.c_str(), follow_str.length(), 0);
    });
    send(query.sockfd, done_msg.c_str(), done_msg.length(), 0);
}

void execute_single_query(postman::Message query, std::shared_ptr<storage::DB> db) {
    DEBUG("Parsing Query: %s\n", query.data);
    postman::ClusterUpdate cu;
    cu.initiator = db->myself_host;
    auto ds = util::split(query.data, ' ');
    if (ds[0] == "ADD") { // ADD query
        try {
            if (ds[1] == "USER") {
                auto username = util::hexdecode(ds[2]);
                auto hashed_pass = ds[3];
                auto user = db->add_user(username, hashed_pass);
                cu.new_users.push_back(user);
                auto reply = storage::serialize_user(user) + "\nDONE\n";
                DEBUG("Replying with: %s\n", reply.c_str());
                send(query.sockfd, reply.c_str(), reply.length(), 0);
            } else if (ds[1] == "POST") {
                auto username = util::hexdecode(ds[2]);
                auto content = util::hexdecode(ds[3]);
                auto timestamp = std::stoi(ds[4]);
                auto post = db->add_post(username, content, timestamp);
                cu.new_posts.push_back(post);
                auto reply = storage::serialize_post(post) + "\nDONE\n";
                DEBUG("Replying with: %s\n", reply.c_str());
                send(query.sockfd, reply.c_str(), reply.length(), 0);
            } else if (ds[1] == "FOLLOW") {
                auto followed_username = util::hexdecode(ds[2]);
                auto follower_username = util::hexdecode(ds[3]);
                auto follow = db->add_follow(followed_username, follower_username);
                cu.new_follows.push_back(follow);
                auto reply = storage::serialize_follow(follow) + "\nDONE\n";
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
                    auto username = util::hexdecode(ds[4]);
                    auto user = db->get_user_by_username(username);
                    auto reply = storage::serialize_user(user) + "\nDONE\n";
                    DEBUG("Replying with: %s\n", reply.c_str());
                    send(query.sockfd, reply.c_str(), reply.length(), 0);
                } else {
                    ERR("Invalid field to GET USER BY: %s\n", ds[3].c_str());
                    return;
                }
            } else if (ds[1] == "POST" && ds[2] == "BY") {
                if (ds[3] == "USERNAME") {
                    auto username = util::hexdecode(ds[4]);
                    auto posts = db->get_posts_by_user(db->get_user_by_username(username));
                    for_each(posts.begin(), posts.end(), [&] (storage::Post post) {
                        auto reply = storage::serialize_post(post) + '\n';
                        DEBUG("Replying with: %s\n", reply.c_str());
                        send(query.sockfd, reply.c_str(), reply.length(), 0);
                    });
                    std::string donemsg = "DONE\n";
                    send(query.sockfd, donemsg.c_str(), donemsg.length(), 0);
                } else {
                    ERR("Invalid field to GET POST BY: %s\n", ds[3].c_str());
                    // TODO: reply with info
                    return;
                }
            } else if (ds[1] == "FOLLOWS" && ds[2] == "BY") {
                if (ds[3] == "FOLLOWER") {
                    auto follower_username = util::hexdecode(ds[4]);
                    auto follows = db->get_follows_by_follower_username(follower_username);
                    for_each(follows.begin(), follows.end(), [&] (storage::Follow follow) {
                            auto reply = storage::serialize_follow(follow) + '\n';
                            DEBUG("Replying with: %s\n", reply.c_str());
                            send(query.sockfd, reply.c_str(), reply.length(), 0);
                            });
                    std::string donemsg = "DONE\n";
                    send(query.sockfd, donemsg.c_str(), donemsg.length(), 0);
                } else if (ds[3] == "FOLLOWED") {
                    auto followed_username = util::hexdecode(ds[4]);
                    auto follows = db->get_follows_by_followed_username(followed_username);
                    for_each(follows.begin(), follows.end(), [&] (storage::Follow follow) {
                            auto reply = storage::serialize_follow(follow) + '\n';
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
    } else if (ds[0] == "DUMP") {
        dump_db(query, db);
    } else if (ds[0] == "PING") {
        std::string pong_msg = "PONG\nDONE\n";
        send(query.sockfd, pong_msg.c_str(), pong_msg.length(), 0);
    } else {
        ERR("Invalid query type %s\n", ds[0].c_str());
        // TODO: reply with info
        return;
    }
    postman::pass_update(cu, mt_gox);
}

enum Mgmt_Type {
    ADD_NODE,
    CONNECT,
    GET_HOSTS,
    CLUSTER_UPDATE,
    SAVE,
    LOAD,
    MGMT_INVALID
};

#define DECODER(a) do { \
    if (s_type == #a) { \
        return a; \
    } \
} while(0);
Mgmt_Type decode_mgmt_type(std::string s_type) {
    DECODER(ADD_NODE);
    DECODER(CONNECT);
    DECODER(GET_HOSTS);
    DECODER(CLUSTER_UPDATE);
    DECODER(SAVE);
    DECODER(LOAD);
    return MGMT_INVALID;
}
#undef DECODER

void parse_and_execute_mgmt(std::vector<postman::Message> messages, std::shared_ptr<storage::DB> db) {
    auto update_type = decode_mgmt_type(messages[1].data);
    switch (update_type) {
        case ADD_NODE:
            {
                std::lock_guard<std::mutex>(db->sync_mutex);

                uint32_t addr = messages[2].sockaddr.sin_addr.s_addr;
                auto msg_split = util::split(messages[2].data, ':');
                uint32_t port = std::stoi(msg_split[0]);
                uint32_t id = std::stoi(msg_split[1]);
                storage::Host new_host =  storage::Host(addr, port, id);
                LOG("Adding node: %s\n", new_host.serialize().c_str());

                postman::ClusterUpdate cu;
                cu.initiator = db->myself_host;
                cu.new_hosts.push_back(new_host);

                // if we have an existing thing on this ip:port, we can assume the
                // node died and came back up with a different number, and we never
                // pinged it while it was down. Let's remove it now, otherwise bad
                // things will happen :(
                auto itr = find_if(db->hosts.begin(), db->hosts.end(), [&] (storage::Host host) {
                    return new_host.ip == host.ip && new_host.port == host.port; 
                });
                if (itr != db->hosts.end()) {
                    cu.del_hosts.push_back(*itr);
                    db->hosts.erase(itr);
                }

                // send along the cluster toplogy (each node on a line, linked list from n -> n+1
                for_each(db->hosts.begin(), db->hosts.end(), [&] (storage::Host host) {
                    auto host_msg = host.serialize() + '\n';
                    send(messages[0].sockfd, host_msg.c_str(), host_msg.length(), 0);
                });
                std::string done_msg = "DONE\n";
                send(messages[0].sockfd, done_msg.c_str(), done_msg.length(), 0);

                DEBUG("Sent Cluster topology\n");


                postman::pass_update(cu, db);


                // add new host to the cluster
                db->hosts.insert(new_host);

                // the client will send a message indicating that it's ready to proceed
                char buf[1024];
                recv(messages[0].sockfd, buf, sizeof(buf), 0);

                postman::ClusterUpdate existing_data;
                existing_data.initiator = db->myself_host;

                // Propogate data to client
                auto users = db->get_users();
                for_each(users.begin(), users.end(), [&] (storage::User user) {
                    existing_data.new_users.push_back(user);
                });

                auto posts = db->get_posts();
                for_each(posts.begin(), posts.end(), [&] (storage::Post post) {
                    existing_data.new_posts.push_back(post);
                });

                auto follows = db->get_follows();
                for_each(follows.begin(), follows.end(), [&] (storage::Follow follow) {
                    existing_data.new_follows.push_back(follow);
                });


                auto serialized_data = postman::serialize_cluster_update(existing_data) + "DONE\n";
                send(messages[0].sockfd, serialized_data.c_str(), serialized_data.length(), 0);
                DEBUG("Sent existing data!\n");
                LOG("Node added successfully!\n");
            }
            break;
        case CONNECT:
            {
                std::lock_guard<std::mutex>(db->sync_mutex);
                LOG("Connecting to cluster...\n");
                auto splits = util::split(messages[2].data, ':');
                auto host = splits[0];
                auto port = std::stoi(splits[1]);
                try {
                    postmaster_general->connect_to_cluster(host, port, db);
                } catch (postman::ConnectionFailure cf) {
                    ERR("Error connecting to %s:%d\n", host.c_str(), port);
                    std::string error_msg = "ERROR\n";
                    send(messages[0].sockfd, error_msg.c_str(), error_msg.length(), 0);
                    return;
                }
                LOG("Connected!\n");
            }
            break;
        case GET_HOSTS:
            {
                DEBUG("Sending host list...\n");
                for_each(db->hosts.begin(), db->hosts.end(), [&] (storage::Host host) {
                    auto host_str = host.serialize() + '\n';
                    send(messages[0].sockfd, host_str.c_str(), host_str.length(), 0);
                });
                std::string done_msg = "DONE\n";
                send(messages[0].sockfd, done_msg.c_str(), done_msg.length(), 0);
            }
            break;
        case CLUSTER_UPDATE:
            {
                DEBUG("Cluster update recieved...\n");
                std::vector<std::string> cup;
                for_each(messages.begin()+2, messages.end(), [&] (postman::Message msg) {
                        cup.push_back(msg.data);
                        });
                auto update = postman::deserialize_cluster_update(util::join(cup, '\n'));


                // If we are the initiator, do nothing.
                if (update.initiator == db->myself_host) break;
                // apply update
                DEBUG("Applying recieved update\n");
                postman::apply_cluster_update(update, db);
                DEBUG("Passing query down\n");
                // pass the update down the chain
                postman::pass_update(update, db);
            }
            break;
        case SAVE:
            DEBUG("Saving DB to disk...\n");
            db->save();
            break;
        case LOAD:
            DEBUG("Loading DB from disk...\n");
            db->load();
            break;
        case MGMT_INVALID:
            {
                ERR("Invalid update type: %s\n", messages[1].data);
                // TODO: return error to client
            }
            return;
    }
}

// TODO: don't assume well-formed queries...
void parser::parse_and_execute(std::vector<postman::Message> messages, std::shared_ptr<storage::DB> db) {
    if (std::string(messages[0].data) == "QUERY") {
        for_each(messages.begin()+1, messages.end()-1, [&] (postman::Message query) {
                execute_single_query(query, db);
        });
    } else if (std::string(messages[0].data) == "MGMT") {
        parse_and_execute_mgmt(messages, db);
    } else {
        ERR("Invalid message type: %s\n", messages[0].data);
    }
}
