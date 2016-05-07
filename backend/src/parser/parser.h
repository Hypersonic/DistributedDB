#pragma once
/** The parser parses messages... No, really.
 *
 * The query syntax is quite simple:
 *      <query>       ::= <add_query>|<get_query>|<dump_query>|<save_query>|<load_query>|<ping_query> <EOL>
 *
 *      <add_query>   ::= "ADD " <user_add>|<post_add>|<follow_add>
 *      <user_add>    ::= "USER " <username_hexencoded> " " <hashed_pass>
 *      <post_add>    ::= "POST " <user_id> " " <content_hexencoded> " " <timestamp>
 *      <follow_add>  ::= "FOLLOW " <followed_id> " " <follower_id>
 *      
 *      <get_query>   ::= "GET " <user_get>|<post_get>|<follow_get>
 *      <user_get>    ::= "USER BY " <by_username>|<by_user_id>
 *      <by_username> ::= "USERNAME " <username_hexencoded>
 *      <by_user_id>  ::= "USER_ID " <user_id>
 *      <post_get>    ::= "POST BY " <by_user_id>|<by_post_id>
 *      <by_post_id>  ::= "POST_ID " <post_id>
 *      <follow_get>  ::= "FOLLOWS BY " <follower_get>|<followed_get>
 *      follower_get  ::= "FOLLOWER_ID " <user_id>
 *      followed_get  ::= "FOLLOWED_ID " <user_id>
 *
 *      <dump_query>  ::= "DUMP"
 *      
 *      <save_query>  ::= "SAVE"
 *      
 *      <load_query>  ::= "LOAD"
 *      
 *      <ping_query>  ::= "PING"
 *
 *
 * The MGMT syntax:
 *
 *      <mgmt>        ::= <add_node_query>
 *
 *      <add_node_query> ::= "ADD_NODE" <EOL> <node_info> <EOL>
 *      <node_info>      ::= <port>
 *
 */

#include <functional>
#include <string>
#include <memory>

#include "util.h"
#include "globals.h"
#include "postman/postman.h"
#include "storage/storage.h"

namespace parser {
    void parse_and_execute(std::vector<postman::Message> messages, std::shared_ptr<storage::DB> db);
}
