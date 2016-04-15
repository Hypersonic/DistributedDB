#pragma once
/** The parser parses queries... No, really.
 *
 * The query syntax is quite simple:
 *      <query>       ::= <add_query>|<get_query> <EOL>
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
 *
 */

#include <functional>
#include <string>
#include <memory>

#include "util.h"
#include "postman/postman.h"
#include "storage/storage.h"

namespace parser {
    void parse_and_execute(postman::Message query, std::shared_ptr<storage::DB> db);
}
