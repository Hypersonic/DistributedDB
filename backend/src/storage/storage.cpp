#include "storage.h"

using namespace storage;

User DB::add_user(std::string username, std::string hashed_pass) {
    std::lock_guard<std::mutex>(this->users_mtx);
    User u;

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
