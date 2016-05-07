#pragma once
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <thread>
#include <system_error>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "storage/storage.h"
#include "macros.h"

/**
 * The Postman is responsible for handling incoming messages and
 * delegating them out, message-passing style
 */


// TODO: Make this part of the Postman class instead of a static
// Was having issues with Postman locals not being properly updated when set in
// a different thread, so this is the temp workaround... weird...
UNUSED static bool alive = true;

namespace postman {
    // dummy class to be thrown for socket connection failures
    class ConnectionFailure {
    };

    /* A message */
    struct Message {
        Message(int sfd, struct sockaddr_in sa, char *d, size_t l) : sockfd(sfd), sockaddr(sa), data(d), length(l) {};
        int sockfd;
        struct sockaddr_in sockaddr;
        char  *data;
        size_t length;
    };

    struct ClusterUpdate {
        storage::Host initiator;

        std::vector<storage::User>     new_users;
        std::vector<storage::Post>     new_posts;
        std::vector<storage::Follow>   new_follows;

        std::vector<storage::Host>     new_hosts;
        std::vector<storage::Host>     del_hosts;
    };

    std::string serialize_cluster_update(ClusterUpdate cu);
    ClusterUpdate deserialize_cluster_update(std::string s);

    storage::Host next_host(std::shared_ptr<storage::DB> db);

    void apply_cluster_update(ClusterUpdate cu, std::shared_ptr<storage::DB> db);

    typedef std::function<void(Message)> callback_t;

    /* The actual class which encapsulates the postman's behavior */
    class Postman {
    public:
        Postman(int _listen_port) : listen_port(_listen_port) {};
        void start(); // Listen for new connections
        void stop(); // wait for connections to close and shutdown
        void register_callback(callback_t cb);

        long get_port() { return this->listen_port; }

        // establish connection to cluster via a known node, ask for info and ship it off to the parser to populate the db
        void connect_to_cluster(std::string hostname, long port, std::shared_ptr<storage::DB> db);

        std::thread listen_thread;
    private:
        int  listen_port;
        int  listen_sock;
        std::vector<callback_t> callbacks;
    };

    /** Spawn a new postman in the background
     */
    std::shared_ptr<Postman> spawn_postman(long port);

    void pass_update(ClusterUpdate cu, std::shared_ptr<storage::DB> db);
}
