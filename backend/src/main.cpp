#include "macros.h"
#include "postman/postman.h"
#include "parser/parser.h"
#include "storage/storage.h"
#include <signal.h>

std::shared_ptr<postman::Postman> postmaster_general;

void sigint_handler(UNUSED int sig) {
    LOG("SIGINT recieved, assassinating the postmaster general...\n");
    postmaster_general->stop(); // TODO: make this work instead of crashing
    LOG("Postmaster general assassinated. Have a nice day!\n");
}

int main(UNUSED int argc, UNUSED char **argv) {
    postmaster_general = postman::spawn_postman();
    LOG("Postmaster general has been spawned.\n");
    signal(SIGINT, sigint_handler);

    std::shared_ptr<storage::DB> db = std::make_shared<storage::DB>();

    // Register callback to parse messages
    postmaster_general->register_callback([&] (postman::Message msg) {
        parser::parse_and_execute(msg, db);
    });



    while(postmaster_general->listen_thread.joinable()) {};
    DEBUG("Exiting...\n");
    return 0;
}

