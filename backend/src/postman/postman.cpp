#include "postman.h"

#include <cerrno>
#include <cstdio>

std::shared_ptr<postman::Postman> postman::spawn_postman() {
    std::shared_ptr<Postman> pm = std::make_shared<Postman>(3001);
    std::thread tr([=] () {
            pm->start();
        });
    pm->listen_thread = std::move(tr);
    DEBUG("Postman spawned :D\n");
    return pm;
}

void postman::Postman::register_callback(callback_t cb) {
    this->callbacks.push_back(cb);
}

void postman::Postman::start() {
    struct sockaddr_in serv_addr;

    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        ERR("Failed to create socket: %s\n", strerror(errno));
        abort();
    }
    DEBUG("Socket Created!\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // bind errywhere
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(this->listen_port);

    if ((bind(listen_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) == -1) {
        ERR("Unable to bind port: %s\n", strerror(errno));
        abort();
    }
    DEBUG("Socket bound!\n");

    if (listen(listen_sock, 1024) == -1) {
        ERR("Unable to listen: %s\n", strerror(errno));
        abort();
    }
    DEBUG("Socket listening!\n");

    while (alive) {
        DEBUG("Waiting for connection...\n");

        int conn;
        conn = accept(listen_sock, nullptr, nullptr);
        if (conn == -1) {
            if (errno == ECONNABORTED) { // We'll get this when the socket is closed
                DEBUG("Listening thread shutting down...\n");
                return;
            } else {
                ERR("Accept failed: %s\n", strerror(errno));
                abort();
            }
        }
        DEBUG("Connection recieved!\n");
        auto connection_handler = [] (Postman* pmp, int conn_fd) {
            char buf[4096]; // TODO: dynamically allocate this or be cleverer
            int read = 0;
            while ((read = recv(conn_fd, buf, sizeof(buf)-1, 0)) > 0) {
                postman::Message m(conn_fd, buf, read);
                // callback to everyone who has registered with us
                for_each(pmp->callbacks.begin(), pmp->callbacks.end(), [&](callback_t cb) {
                        DEBUG("Calling a callback\n");
                        cb(m);
                });
            }
            DEBUG("Connection closed!\n");
            close(conn_fd);
        };
        std::thread handler_thread(connection_handler, this, conn);
        handler_thread.detach();
    }
    close(listen_sock);
}

void postman::Postman::stop() {
    alive = false;
    close(this->listen_sock);
    try {
        this->listen_thread.join();
    } catch (std::system_error e) {
        ERR("Could not join postman listen thread! Aborting!\n");
        abort();
    }
    DEBUG("Postman Closed!\n");
}
