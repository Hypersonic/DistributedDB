#include "storage.h"

using namespace storage;

User DB::add_user(std::string username, std::string hashed_pass) {
    std::lock_guard<std::mutex>(this->users_mtx);
    User u;

    // check if the user already exists -- we can't re-add
    auto matched_user = find_if(this->users.begin(), this->users.end(), [&] (User uu) {
        return uu.username == username;
    });
    if (matched_user != this->users.end()) {
        throw AlreadyExists();
    }

    u.username = username;
    u.hashed_pass = hashed_pass;
    users.push_back(u);
    return u;
}

User DB::add_user(User user) {
    std::lock_guard<std::mutex>(this->users_mtx);
    users.push_back(user);
    return user;
}

User DB::get_user_by_username(std::string username) {
    std::lock_guard<std::mutex>(this->users_mtx);
    auto user = find_if(this->users.begin(), this->users.end(), [&] (User u) {
        return u.username == username;
    });
    if (user != this->users.end()) {
        return *user;
    } else {
        throw NotFound();
    }
}

Post DB::add_post(std::string username, std::string content, size_t timestamp) {
    std::lock_guard<std::mutex>(this->posts_mtx);
    Post p;

    p.username = username;
    p.content = content;
    p.timestamp = timestamp;

    posts.push_back(p);
    return p;
}

Post DB::add_post(Post post) {
    std::lock_guard<std::mutex>(this->posts_mtx);
    posts.push_back(post);
    return post;
}

std::vector<Post> DB::get_posts_by_user(User user) {
    std::lock_guard<std::mutex>(this->posts_mtx);
    std::vector<Post> results;

    // predicate indicating a post is relevant to us
    auto post_matches = [&] (Post p) {
        return p.username == user.username;
    };

    auto lo = this->posts.begin();
    auto hi = this->posts.end();

    while (lo != hi) {
        lo = find_if(lo, hi, post_matches);
        results.push_back(*lo);
        // The predicate for this loop is probably redundant, because
        // this will be hit... oh well
        if (lo == hi) {
            break;
        }
        lo++;
    }

    return results;
}

Follow DB::add_follow(std::string followed_username, std::string follower_username) {
    std::lock_guard<std::mutex>(this->follows_mtx);

    // check if the follow already exists -- we can't re-add
    auto matched_follow = find_if(this->follows.begin(), this->follows.end(), [&] (Follow f) {
        return f.followed_username == followed_username && f.follower_username == follower_username;
    });
    if (matched_follow != this->follows.end()) {
        throw AlreadyExists();
    }

    Follow f(followed_username, follower_username);
    this->follows.push_back(f);
    return f;
}

Follow DB::add_follow(Follow follow) {
    std::lock_guard<std::mutex>(this->follows_mtx);
    follows.push_back(follow);
    return follow;
}

std::vector<Follow> DB::get_follows_by_followed_username(std::string followed_username) {
    std::lock_guard<std::mutex>(this->follows_mtx);
    std::vector<Follow> results;

    // predicate indicating a follow is relevant to us
    auto follow_matches = [&] (Follow f) {
        return f.followed_username == followed_username;
    };

    auto lo = this->follows.begin();
    auto hi = this->follows.end();

    while (lo != hi) {
        lo = find_if(lo, hi, follow_matches);
        results.push_back(*lo);
        // The predicate for this loop is probably redundant, because
        // this will be hit... oh well
        if (lo == hi) {
            break;
        }
        lo++;
    }

    return results;
}

std::vector<Follow> DB::get_follows_by_follower_username(std::string follower_username) {
    std::lock_guard<std::mutex>(this->follows_mtx);

    std::vector<Follow> results;

    // predicate indicating a follow is relevant to us
    auto follow_matches = [&] (Follow f) {
        return f.follower_username == follower_username;
    };

    auto lo = this->follows.begin();
    auto hi = this->follows.end();

    while (lo != hi) {
        lo = find_if(lo, hi, follow_matches);
        results.push_back(*lo);
        // The predicate for this loop is probably redundant, because
        // this will be hit... oh well
        if (lo == hi) {
            break;
        }
        lo++;
    }

    return results;
}

void DB::save(std::string user_file_name, std::string post_file_name, std::string follow_file_name) {
    std::ofstream user_file(user_file_name);
    std::ofstream post_file(post_file_name);
    std::ofstream follow_file(follow_file_name);

    std::lock(this->users_mtx, this->posts_mtx, this->follows_mtx);
    std::lock_guard<std::mutex>(this->users_mtx, std::adopt_lock);
    std::lock_guard<std::mutex>(this->posts_mtx, std::adopt_lock);
    std::lock_guard<std::mutex>(this->follows_mtx, std::adopt_lock);

    for_each(this->users.begin(), this->users.end(), [&] (User u) {
        user_file << serialize_user(u) << '\n';
    });
    for_each(this->posts.begin(), this->posts.end(), [&] (Post p) {
        post_file << serialize_post(p) << '\n';
    });
    for_each(this->follows.begin(), this->follows.end(), [&] (Follow f) {
        post_file << serialize_follow(f) << '\n';
    });

    user_file.close();
    post_file.close();
    follow_file.close();
}

void DB::load(std::string user_file_name, std::string post_file_name, std::string follow_file_name) {
    std::ifstream user_file(user_file_name);
    std::ifstream post_file(post_file_name);
    std::ifstream follow_file(follow_file_name);

    std::lock(this->users_mtx, this->posts_mtx, this->follows_mtx);
    std::lock_guard<std::mutex>(this->users_mtx, std::adopt_lock);
    std::lock_guard<std::mutex>(this->posts_mtx, std::adopt_lock);
    std::lock_guard<std::mutex>(this->follows_mtx, std::adopt_lock);

    std::string line;
    while (!getline(user_file, line).eof()) {
        this->users.push_back(deserialize_user(line));
    }
    while (!getline(post_file, line).eof()) {
        this->posts.push_back(deserialize_post(line));
    }
    while (!getline(follow_file, line).eof()) {
        this->follows.push_back(deserialize_follow(line));
    }
    user_file.close();
    post_file.close();
    follow_file.close();
}

std::string storage::serialize_user(User user) {
    return util::hexencode(user.username) + ' ' +
           user.hashed_pass;
}

User storage::deserialize_user(std::string line) {
    User u;
    auto ds = util::split(line, ' ');
    u.username = util::hexdecode(ds[0]);
    u.hashed_pass = ds[1];
    return u;
}

Post storage::deserialize_post(std::string line) {
    Post p;
    auto ds = util::split(line, ' ');
    p.username = util::hexdecode(ds[0]);
    p.content = util::hexdecode(ds[1]);
    p.timestamp = std::stoi(ds[2]);
    return p;
}

std::string storage::serialize_post(Post post) {
    return util::hexencode(post.username) + ' ' +
           util::hexencode(post.content)  + ' ' +
           std::to_string(post.timestamp);
}

Follow storage::deserialize_follow(std::string line) {
    Follow f;
    auto ds = util::split(line, ' ');
    f.followed_username = util::hexdecode(ds[0]);
    f.follower_username = util::hexdecode(ds[1]);
    return f;
}

std::string storage::serialize_follow(Follow follow) { 
    return util::hexencode(follow.followed_username) + ' ' +
           util::hexencode(follow.follower_username);
}

Host storage::deserialize_host(std::string line) {
    auto host_split = util::split(line, ':');
    auto port = std::stoi(host_split[1]);

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if (inet_aton(host_split[0].c_str(), (in_addr *) &dest.sin_addr.s_addr) == 0) {
        ERR("Bad server address: %s\n", host_split[0].c_str());
        throw std::invalid_argument("Bad server address!");
    }

    auto id = std::stoi(host_split[2]);
    return Host(dest.sin_addr.s_addr, port, id);
}

bool storage::operator<(Host lhs, Host rhs) {
    return lhs.id < rhs.id;
}
