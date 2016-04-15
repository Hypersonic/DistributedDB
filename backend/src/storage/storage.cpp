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

    u.user_id = users.size(); // pick user_id as the number of items in the vector
                              // TODO: Don't do this when we need multiple hosts, 
                              // as it is not guaranteed to be unique amongst all hosts
    u.username = username;
    u.hashed_pass = hashed_pass;
    users.push_back(u);
    return u;
}

User DB::get_user_by_user_id(size_t user_id) {
    std::lock_guard<std::mutex>(this->users_mtx);
    auto user = find_if(this->users.begin(), this->users.end(), [&] (User u) {
        return u.user_id == user_id;
    });
    if (user != this->users.end()) {
        return *user;
    } else {
        throw NotFound();
    }
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

Post DB::add_post(size_t user_id, std::string content, size_t timestamp) {
    std::lock_guard<std::mutex>(this->posts_mtx);
    Post p;

    p.post_id = posts.size(); // pick post_id as the number of items in the vector
                              // TODO: Don't do this when we need multiple hosts, 
                              // as it is not guaranteed to be unique amongst all hosts
    p.user_id = user_id;
    p.content = content;
    p.timestamp = timestamp;

    posts.push_back(p);
    return p;
}

Post DB::get_post_by_post_id(size_t post_id) {
    std::lock_guard<std::mutex>(this->posts_mtx);
    auto post = find_if(this->posts.begin(), this->posts.end(), [&] (Post p) {
        return p.post_id == post_id;
    });
    if (post != this->posts.end()) {
        return *post;
    } else {
        throw NotFound();
    }
}

std::vector<Post> DB::get_posts_by_user(User user) {
    std::lock_guard<std::mutex>(this->posts_mtx);
    std::vector<Post> results;

    // predicate indicating a post is relevant to us
    auto post_matches = [&] (Post p) {
        return p.user_id == user.user_id;
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

Follow DB::add_follow(size_t followed_id, size_t follower_id) {
    std::lock_guard<std::mutex>(this->follows_mtx);

    // check if the follow already exists -- we can't re-add
    auto matched_follow = find_if(this->follows.begin(), this->follows.end(), [&] (Follow f) {
        return f.followed_id == followed_id && f.follower_id == follower_id;
    });
    if (matched_follow == this->follows.end()) {
        throw AlreadyExists();
    }

    Follow f(followed_id, follower_id);
    this->follows.push_back(f);
    return f;
}

std::vector<Follow> DB::get_follows_by_followed_id(size_t followed_id) {
    std::lock_guard<std::mutex>(this->follows_mtx);
    std::vector<Follow> results;

    // predicate indicating a follow is relevant to us
    auto follow_matches = [&] (Follow f) {
        return f.followed_id == followed_id;
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

std::vector<Follow> DB::get_follows_by_follower_id(size_t follower_id) {
    std::lock_guard<std::mutex>(this->follows_mtx);

    std::vector<Follow> results;

    // predicate indicating a follow is relevant to us
    auto follow_matches = [&] (Follow f) {
        return f.follower_id == follower_id;
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

    auto serialize_user = [] (User u) {
        return std::to_string(u.user_id) + ' ' +
               util::hexencode(u.username) + ' ' +
               u.hashed_pass;
    };
    auto serialize_post = [] (Post p) {
        return std::to_string(p.post_id) + ' ' +
               std::to_string(p.user_id) + ' ' +
               util::hexencode(p.content) + ' ' +
               std::to_string(p.timestamp);
    };
    auto serialize_follow = [] (Follow f) {
        return std::to_string(f.followed_id) + ' ' + 
               std::to_string(f.follower_id);
    };

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

    auto deserialize_user = [] (std::string line) {
        User u;
        auto ds = util::split_by_space(line);
        u.user_id = std::stoi(ds[0]);
        u.username = util::hexdecode(ds[1]);
        u.hashed_pass = ds[2];
        return u;
    };
    auto deserialize_post = [] (std::string line) {
        Post p;
        auto ds = util::split_by_space(line);
        p.post_id = std::stoi(ds[0]);
        p.user_id = std::stoi(ds[1]);
        p.content = util::hexdecode(ds[2]);
        p.timestamp = std::stoi(ds[3]);
        return p;
    };
    auto deserialize_follow = [] (std::string line) {
        Follow f;
        auto ds = util::split_by_space(line);
        f.followed_id = std::stoi(ds[0]);
        f.follower_id = std::stoi(ds[1]);
        return f;
    };
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
