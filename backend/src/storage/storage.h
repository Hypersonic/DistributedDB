#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <algorithm>

namespace storage {
    // Dummy class thrown to indicate badness
    class NotFound {
    };
    // Dummy class thrown to indicate duplicate
    class AlreadyExists {
    };
    
    class User {
    public:
        User() {};
        User(size_t uid, std::string uname, std::string pass) :
            user_id(uid), username(uname), hashed_pass(pass) {};
        size_t user_id;
        std::string username;
        std::string hashed_pass;
    };

    class Post {
    public:
        Post() {};
        Post(size_t pid, size_t uid, std::string cont, size_t ts) :
            post_id(pid), user_id(uid), content(cont), timestamp(ts) {};
        size_t post_id;
        size_t user_id;
        std::string content;
        size_t timestamp;
    };

    class Follow {
    public:
        Follow() {};
        Follow(size_t _followed_id, size_t _follower_id) : followed_id(_followed_id), follower_id(_follower_id) {};
        size_t followed_id;
        size_t follower_id;
    };

    class DB {
    public:
        DB() {};
        User add_user(std::string username, std::string hashed_pass);

        User get_user_by_user_id(size_t user_id);
        User get_user_by_username(std::string username);

        Post add_post(size_t user_id, std::string content, size_t timestamp);

        Post get_post_by_post_id(size_t post_id);
        std::vector<Post> get_posts_by_user(User user);

        Follow add_follow(size_t followed_id, size_t follower_id);
        std::vector<Follow> get_follows_by_followed_id(size_t followed_id);
        std::vector<Follow> get_follows_by_follower_id(size_t follower_id);

        // TODO: Implement saving and loading
        void save(std::string user_file, std::string post_file);
        void load(std::string user_file, std::string post_file);
    private:
        std::vector<User> users;
        std::mutex users_mtx;
        std::vector<Post> posts;
        std::mutex posts_mtx;
        std::vector<Follow> follows;
        std::mutex follows_mtx;
    };
}
