#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <algorithm>
#include <fstream>
#include <memory>
#include <set>

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
        User(std::string uname, std::string pass) :
             username(uname), hashed_pass(pass) {};
        std::string username;
        std::string hashed_pass;
    };

    class Post {
    public:
        Post() {};
        Post(std::string uname, std::string cont, size_t ts) :
             username(uname), content(cont), timestamp(ts) {};
        std::string username;
        std::string content;
        size_t timestamp;
    };

    class Follow {
    public:
        Follow() {};
        Follow(std::string _followed_username, std::string _follower_username) : followed_username(_followed_username), follower_username(_follower_username) {};
        std::string followed_username;
        std::string follower_username;
    };

    struct Host {
    public:
        uint32_t ip;
        uint32_t port; 
        uint32_t id; // unique id for me

        Host() {}
        Host(uint32_t _ip, uint32_t _port, uint32_t _id) : ip(_ip), port(_port), id(_id) {}

        std::string serialize() {
            char ip_buf[INET_ADDRSTRLEN];
            struct in_addr addr;
            addr.s_addr = ip;
            inet_ntop(AF_INET, (void *) &addr, ip_buf, INET_ADDRSTRLEN);
            std::string ip_s(ip_buf);
            std::string port_s = std::to_string(port);
            return ip_s + ':' + port_s + ':' + std::to_string(id);
        }

        bool operator==(Host rhs) {
            return this->id == rhs.id;
        }

        bool operator!=(Host rhs) {
            return !(*this == rhs);
        }

    };

    bool operator<(Host lhs, Host rhs);

    std::string serialize_user(User user);
    User deserialize_user(std::string line);

    Post deserialize_post(std::string line);
    std::string serialize_post(Post post);

    Follow deserialize_follow(std::string line);
    std::string serialize_follow(Follow follow);

    Host deserialize_host(std::string line);

    class DB {
    public:
        DB(Host host) : myself_host(host) {};
        User add_user(std::string username, std::string hashed_pass);
        User add_user(User user);

        User get_user_by_username(std::string username);

        Post add_post(std::string username, std::string content, size_t timestamp);
        Post add_post(Post post);

        std::vector<Post> get_posts_by_user(User user);

        Follow add_follow(std::string followed_username, std::string follower_username);
        Follow add_follow(Follow follow);
        std::vector<Follow> get_follows_by_followed_username(std::string followed_username);
        std::vector<Follow> get_follows_by_follower_username(std::string follower_username);


        void save(std::string user_file_name="users", std::string post_file_name="posts", std::string follow_file_name="follows");
        void load(std::string user_file_name="users", std::string post_file_name="posts", std::string follow_file_name="follows");

        const std::vector<User>   get_users()   { return this->users;   }
        const std::vector<Post>   get_posts()   { return this->posts;   }
        const std::vector<Follow> get_follows() { return this->follows; }

        Host myself_host;
        std::set<Host> hosts; // everyone, sorted by id

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
