#pragma once
/** The parser parses messages... No, really.
 *
 * The query syntax is quite simple:
 *      <query>       ::= <add_query>|<get_query>|<dump_query>|<save_query>|<load_query>|<ping_query> <EOL>
 *
 *      <add_query>   ::= "ADD " <user_add>|<post_add>|<follow_add>
 *      <user_add>    ::= "USER " <username_hexencoded> " " <hashed_pass>
 *      <post_add>    ::= "POST " <username_hexencoded> " " <content_hexencoded> " " <timestamp>
 *      <follow_add>  ::= "FOLLOW " <followed_id> " " <follower_id>
 *      
 *      <get_query>   ::= "GET " <user_get>|<post_get>|<follow_get>
 *      <user_get>    ::= "USER BY " <by_username>
 *      <by_username> ::= "USERNAME " <username_hexencoded>
 *      <post_get>    ::= "POST BY " <by_username>
 *      <follow_get>  ::= "FOLLOWS BY " <follower_get>|<followed_get>
 *      follower_get  ::= "FOLLOWER " <username_hexencoded>
 *      followed_get  ::= "FOLLOWED " <username_hexencoded>
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
