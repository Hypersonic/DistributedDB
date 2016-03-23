#pragma once
#include <cstdlib>
#include <functional>
#include <vector>
#include <string>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>

#include "macros.h"

/**
 * The Postman is responsible for handling incoming messages and
 * delegating them out, message-passing style
 */


// TODO: Make this part of the Postman class instead of a static
// Was having issues with Postman locals not being properly updated when set in
// a different thread, so this is the temp workaround... weird...
static bool alive = true;

namespace postman {

    /* A message */
    struct Message {
        Message(int sfd, char *d, size_t l) : sockfd(sfd), data(d), length(l) {};
        int sockfd;
        char  *data;
        size_t length;
    };

    typedef std::function<void(Message)> callback_t;

    /* The actual class which encapsulates the postman's behavior */
    class Postman {
    public:
        Postman(int _listen_port) : listen_port(_listen_port) {};
        void start(); // Listen for new connections
        void stop(); // wait for connections to close and shutdown
        void register_callback(callback_t cb);

        std::thread listen_thread;
    private:
        int  listen_port;
        int  listen_sock;
        std::vector<callback_t> callbacks;
    };

    /** Spawn a new postman in the background
     */
    std::shared_ptr<Postman> spawn_postman();

}
