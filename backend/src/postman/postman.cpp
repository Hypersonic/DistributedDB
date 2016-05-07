#include "postman.h"

#include <cerrno>
#include <cstdio>

std::shared_ptr<postman::Postman> postman::spawn_postman(long port) {
    std::shared_ptr<Postman> pm = std::make_shared<Postman>(port);
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
        struct sockaddr sockaddr;
        socklen_t socklen;
        conn = accept(listen_sock, &sockaddr, &socklen);
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
        auto connection_handler = [&sockaddr] (Postman* pmp, int conn_fd) {
            std::string contents = "";
            char buf[4096]; // TODO: dynamically allocate this or be cleverer
            int read = 0;
            while ((read = recv(conn_fd, buf, sizeof(buf)-1, 0)) > 0) {
                contents += buf;
                memset(buf, 0, sizeof(buf));
                if (util::contains(contents, "DONE")) {
                    break;
                }
            }
            struct sockaddr_in sin;
            memcpy(&sin, &sockaddr, sizeof(struct sockaddr_in)); // HACK HACK HACK
            postman::Message m(conn_fd, sin, (char *) contents.c_str(), contents.length());
            // callback to everyone who has registered with us
            DEBUG("Message is: %s\n", m.data);
            for_each(pmp->callbacks.begin(), pmp->callbacks.end(), [&](callback_t cb) {
                DEBUG("Calling a callback\n");
                cb(m);
            });
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

void postman::Postman::connect_to_cluster(std::string hostname, long port, std::shared_ptr<storage::DB> db) {
    DEBUG("Connecting to cluster @ %s:%d\n", hostname.c_str(), port);
    char buf[1024]; // generic buffer for sprintfs and such recvs
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        ERR("Failed to create socket: %s\n", strerror(errno));
        throw ConnectionFailure();
    }
    DEBUG("Socket Created!\n");
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if (inet_aton(hostname.c_str(), (in_addr *) &dest.sin_addr.s_addr) == 0) {
        ERR("Bad server address: %s\n", hostname.c_str());
        throw ConnectionFailure();
    }
    if (connect(sockfd, (sockaddr *) &dest, sizeof(dest)) != 0) {
        ERR("Connect failure: %s\n", strerror(errno));
        throw ConnectionFailure();
    }

    sprintf(buf, "MGMT\nADD_NODE\n%d:%d\nDONE\n", this->listen_port, db->myself_host.id);
    std::string initiator_cmd = buf;
    send(sockfd, initiator_cmd.c_str(), initiator_cmd.length(), 0);

    DEBUG("Sent initiator\n");

    std::string resp = "";
    memset(buf, 0, sizeof(buf));
    while (recv(sockfd, buf, sizeof(buf), 0) > 0) {
        resp += buf;
        memset(buf, 0, sizeof(buf));
        if (util::contains(resp, "DONE\n")) {
            break;
        }
    }

    DEBUG("Recieved graph info, parsing...\n");

    DEBUG("Resp: %s\n", resp.c_str());
    auto unparsed_hosts = util::split(resp, '\n');
    for_each(unparsed_hosts.begin(), unparsed_hosts.end()-2, [&] (std::string unparsed_host) {
        DEBUG("Adding host to ring: %s\n", unparsed_host.c_str());
        auto host = storage::deserialize_host(unparsed_host);
        db->hosts.insert(host);
    });

    // At this point, the new machine is in the cluster, and will recieve
    //  updates.
    
    // Let's let the server know that we're ready for the data
    std::string ready_msg = "READY\n";
    send(sockfd, ready_msg.c_str(), ready_msg.length(), 0);

    // Now, we're gonna recieve the existing data from the database.

    DEBUG("Recieving data from database\n");
    resp = "";
    memset(buf, 0, sizeof(buf));
    while (recv(sockfd, buf, sizeof(buf), 0) > 0) {
        resp += buf;
        memset(buf, 0, sizeof(buf));
        if (util::contains(resp, "DONE\n")) {
            break;
        }
    }

    DEBUG("Got existing data: %s\n", resp.c_str());

    auto cu = postman::deserialize_cluster_update(resp);

    postman::apply_cluster_update(cu, db);

    close(sockfd);
}

std::string postman::serialize_cluster_update(ClusterUpdate cu) {
    std::string serialized = "";
    serialized += cu.initiator.serialize() + '\n';

    serialized += "NEW_USERS\n";
    for_each(cu.new_users.begin(), cu.new_users.end(), [&] (storage::User user) {
        serialized += storage::serialize_user(user) + '\n';
    });

    serialized += "NEW_POSTS\n";
    for_each(cu.new_posts.begin(), cu.new_posts.end(), [&] (storage::Post post) {
        serialized += storage::serialize_post(post) + '\n';
    });

    serialized += "NEW_FOLLOWS\n";
    for_each(cu.new_follows.begin(), cu.new_follows.end(), [&] (storage::Follow follow) {
        serialized += storage::serialize_follow(follow) + '\n';
    });

    serialized += "NEW_HOSTS\n";
    for_each(cu.new_hosts.begin(), cu.new_hosts.end(), [&] (storage::Host host) {
        serialized += host.serialize() + '\n';
    });
    
    serialized += "DEL_HOSTS\n";
    for_each(cu.del_hosts.begin(), cu.del_hosts.end(), [&] (storage::Host host) {
        serialized += host.serialize() + '\n';
    });

    return serialized;
}

postman::ClusterUpdate postman::deserialize_cluster_update(std::string s) {
    ClusterUpdate cu;
    auto lines = util::split(s, '\n');

    cu.initiator = storage::Host(storage::deserialize_host(lines[0]));

    enum Mode {
        NEW_USERS,
        NEW_POSTS,
        NEW_FOLLOWS,
        NEW_HOSTS,
        DEL_HOSTS,
        ERROR
    };

#define DECODER(a) do { \
    if (modeline == #a) return a; \
} while(0)
    auto decode_mode = [] (std::string modeline) {
        DECODER(NEW_USERS);
        DECODER(NEW_POSTS);
        DECODER(NEW_FOLLOWS);
        DECODER(NEW_HOSTS);
        DECODER(DEL_HOSTS);
        return ERROR;
    };
#undef DECODER

    Mode curr_mode = ERROR;
    for (size_t curr_line = 1; curr_line < lines.size()-1; curr_line++) {
        if (lines[curr_line] == "DONE") break;
        if (decode_mode(lines[curr_line]) != ERROR) {
            DEBUG("Switching mode to %s\n", lines[curr_line].c_str());
            curr_mode = decode_mode(lines[curr_line]);
        } else {
            switch (curr_mode) {
            case NEW_USERS:
                cu.new_users.push_back(storage::deserialize_user(lines[curr_line]));
                break;
            case NEW_POSTS:
                cu.new_posts.push_back(storage::deserialize_post(lines[curr_line]));
                break;
            case NEW_FOLLOWS:
                cu.new_follows.push_back(storage::deserialize_follow(lines[curr_line]));
                break;
            case NEW_HOSTS:
                cu.new_hosts.push_back(storage::deserialize_host(lines[curr_line]));
                break;
            case DEL_HOSTS:
                cu.del_hosts.push_back(storage::deserialize_host(lines[curr_line]));
                break;
            case ERROR:
                ERR("This should never be hit!!!\n");
                break;
            }
        }
    }

    return cu;
}

storage::Host postman::next_host(std::shared_ptr<storage::DB> db) {
    auto myself_host = db->myself_host;
    auto itr = find_if(db->hosts.begin(), db->hosts.end(), [&] (storage::Host host) {
        return myself_host.id < host.id;
    });
    if (itr != db->hosts.end()) {
        return *itr;
    } else {
        return *db->hosts.begin();
    }
}

void postman::pass_update(ClusterUpdate cu, std::shared_ptr<storage::DB> db) {

    ClusterUpdate dead_hosts; // hosts that fail to respond to pings are "dead", and we'll send this update around the cluster to indicate that they should be removed.
    dead_hosts.initiator = db->myself_host;
    bool need_to_kill_nodes = false;


    while (true) {
        int sockfd;
        if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
            ERR("Failed to create socket: %s\n", strerror(errno));
            throw ConnectionFailure();
        }
        auto next_host = postman::next_host(db);
        DEBUG("Socket Created!\n");
        struct sockaddr_in dest;
        memset(&dest, 0, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_port = htons(next_host.port);
        dest.sin_addr.s_addr = next_host.ip;
        // If a host fails to connect, we consider it dead and try the next one. It should then be removed from the cluster
        if (connect(sockfd, (sockaddr *) &dest, sizeof(dest)) != 0) {
            dead_hosts.del_hosts.push_back(next_host);
            ERR("Noticed a dead node: %s\n", next_host.serialize().c_str());
            db->hosts.erase(next_host);
            need_to_kill_nodes = true;
            continue;
        }
        DEBUG("Passing update to host %s\n", next_host.serialize().c_str()); 
        if (need_to_kill_nodes) {
            int sockfd2;
            if ((sockfd2 = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
                ERR("Failed to create socket: %s\n", strerror(errno));
                throw ConnectionFailure();
            }
            DEBUG("Socket Created!\n");
            if (connect(sockfd2, (sockaddr *) &dest, sizeof(dest)) != 0) {} // TODO error here

            std::string update_msg = "MGMT\nCLUSTER_UPDATE\n" + serialize_cluster_update(dead_hosts) + "DONE\n";
            send(sockfd2, update_msg.c_str(), update_msg.length(), 0); // rip dead nodes
            apply_cluster_update(dead_hosts, db); // remove the nodes from ourselves
        }


        std::string update_msg = "MGMT\nCLUSTER_UPDATE\n" + serialize_cluster_update(cu) + "DONE\n";
        send(sockfd, update_msg.c_str(), update_msg.length(), 0); // send along actual update

        return;
    }
}

void postman::apply_cluster_update(postman::ClusterUpdate cu, std::shared_ptr<storage::DB> db) {
    std::lock_guard<std::mutex>(db->sync_mutex);

    DEBUG("Applying user adds\n");
    for_each(cu.new_users.begin(), cu.new_users.end(), [&] (storage::User user) {
        db->add_user(user);
    });
    DEBUG("Applying post adds\n");
    for_each(cu.new_posts.begin(), cu.new_posts.end(), [&] (storage::Post post) {
        db->add_post(post);
    });
    DEBUG("Applying follow adds\n");
    for_each(cu.new_follows.begin(), cu.new_follows.end(), [&] (storage::Follow follow) {
        db->add_follow(follow);
    });
    
    DEBUG("Applying host adds\n");
    for_each(cu.new_hosts.begin(), cu.new_hosts.end(), [&] (storage::Host host) {
        db->hosts.insert(host);
    });

    DEBUG("Applying host dels\n");
    for_each(cu.del_hosts.begin(), cu.del_hosts.end(), [&] (storage::Host host) {
        DEBUG("Removing dead host: %s\n", host.serialize().c_str());
        db->hosts.erase(host);
    });

    DEBUG("Done applying cluster update!\n");
}

