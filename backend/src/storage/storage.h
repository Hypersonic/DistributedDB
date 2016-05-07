#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <algorithm>
#include <fstream>
#include <memory>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "util.h"

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

    struct Host {
    public:
        uint32_t ip;
        uint32_t port; 
        std::shared_ptr<Host> next;

        Host(uint32_t _ip, uint32_t _port) : ip(_ip), port(_port), next(nullptr) {}

        std::string serialize() {
            char ip_buf[INET_ADDRSTRLEN];
            struct in_addr addr;
            addr.s_addr = ip;
            inet_ntop(AF_INET, (void *) &addr, ip_buf, INET_ADDRSTRLEN);
            std::string ip_s(ip_buf);
            std::string port_s = std::to_string(port);
            return ip_s + ':' + port_s;
        }

        bool operator==(Host rhs) {
            return this->ip == rhs.ip && this->port == rhs.port;
        }
    };

    std::string serialize_user(User user);
    User deserialize_user(std::string line);

    Post deserialize_post(std::string line);
    std::string serialize_post(Post post);

    Follow deserialize_follow(std::string line);
    std::string serialize_follow(Follow follow);

    Host deserialize_host(std::string line);

    class DB {
    public:
        DB() {};
        User add_user(std::string username, std::string hashed_pass);
        User add_user(User user);

        User get_user_by_user_id(size_t user_id);
        User get_user_by_username(std::string username);

        Post add_post(size_t user_id, std::string content, size_t timestamp);
        Post add_post(Post post);

        Post get_post_by_post_id(size_t post_id);
        std::vector<Post> get_posts_by_user(User user);

        Follow add_follow(size_t followed_id, size_t follower_id);
        Follow add_follow(Follow follow);
        std::vector<Follow> get_follows_by_followed_id(size_t followed_id);
        std::vector<Follow> get_follows_by_follower_id(size_t follower_id);


        void save(std::string user_file_name="users", std::string post_file_name="posts", std::string follow_file_name="follows");
        void load(std::string user_file_name="users", std::string post_file_name="posts", std::string follow_file_name="follows");

        const std::vector<User>   get_users()   { return this->users;   }
        const std::vector<Post>   get_posts()   { return this->posts;   }
        const std::vector<Follow> get_follows() { return this->follows; }

        std::shared_ptr<Host> myself_host; // our index into the cyclicly linked list of hosts, starting with ourself, of course

        std::mutex sync_mutex; // mutex to prevent from syncing (for instance, while we're adding new nodes to the cluster)
    private:
        std::vector<User> users;
        std::mutex users_mtx;
        std::vector<Post> posts;
        std::mutex posts_mtx;
        std::vector<Follow> follows;
        std::mutex follows_mtx;

    };
}
