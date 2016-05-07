#include "macros.h"
#include "postman/postman.h"
#include "parser/parser.h"
#include "storage/storage.h"
#include "globals.h"
#include <signal.h>
#include <fstream>

std::shared_ptr<postman::Postman> postmaster_general;
std::shared_ptr<storage::DB> mt_gox; // the global db

void sigint_handler(UNUSED int sig) {
    LOG("SIGINT recieved... taking down services...\n");
    LOG("Saving DB...\n");
    mt_gox->save();
    LOG("Saved DB!\n");
    LOG("Assassinating the postmaster general...\n");
    postmaster_general->stop();
    LOG("Postmaster general assassinated. Have a nice day!\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    long port;
    try {
        port = std::stoi(argv[1]);
    } catch (std::invalid_argument ie) {
        ERR("Hey! %s isn't a valid port!\n", argv[1]);
        return 1;
    }
     
    postmaster_general = postman::spawn_postman(port);
    LOG("Postmaster general has been spawned.\n");
    signal(SIGINT, sigint_handler);

    mt_gox = std::make_shared<storage::DB>();

    mt_gox->myself_host = std::make_shared<storage::Host>(0, port); // we can assume our ip is 0 because 0.0.0.0
    mt_gox->myself_host->next = mt_gox->myself_host;

#ifdef DEBUG_MODE
    std::ofstream logfile("querylog");
    // Register callback to log queries
    postmaster_general->register_callback([&] (postman::Message msg) {
        logfile << msg.data << '\n';
        logfile.flush();
    });
#endif

    // Register callback to parse messages
    postmaster_general->register_callback([&] (postman::Message msg) {
        std::vector<std::string> dataz = util::split(msg.data, '\n');
        std::vector<postman::Message> msgs;
        for_each(dataz.begin(), dataz.end()-1, [&] (std::string s) {
            if (s != "") {
                msgs.push_back(postman::Message(msg.sockfd, msg.sockaddr, (char *) s.c_str(), s.length()));
            }
        });
        parser::parse_and_execute(msgs, mt_gox);
    });



    while(postmaster_general->listen_thread.joinable()) {};
    DEBUG("Exiting...\n");
    return 0;
}

