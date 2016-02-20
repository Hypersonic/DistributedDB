# "Table" of users. A list of tuples (uuid, username, hashed_pass, [post_ids], [follower_ids], [following_ids])
_users = []

# "Table" of posts. A list of tuples (post_id, user_id, content, timestamp)
_posts = []

def add_user(username, hashed_pass):
    """
    Add a user with a given username and hashed password
    
    Returns the tuple (uuid, username, hashed_pass, [post_ids], [follower_ids])

    raises a KeyError if the user already exists in the database

    WARNING: Currently, this is not threadsafe. Once there is a proper backend,
             this must be fixed.
    """
    u = None
    try:
        u = get_user(username=username)
    except KeyError: # if we get a KeyError it means the user does not exist, and we can continue
        pass
    if u is not None:
        raise KeyError("User already exists: {}".format(username))
    uuid = len(_users) # pick uuid to be 1 after the last thing in the list
    post_ids = [] # obviously a new user doesn't have any posts
    follower_ids = [] # obviously a new user doesn't have any followers
    following_ids = [] # obviously a new user doesn't have anyone to follow
    u = (uuid, username, hashed_pass, post_ids, follower_ids, following_ids)
    _users.append(u)
    return u

def get_user(uuid=None, username=None):
    """
    Get a user from the database with either a matching uuid or username.
    Intentionally implemented inefficiently at the expense of most closely
    emulating an RDBMS

    raises a KeyError if the user cannot be found
    """
    assert(uuid is not None or username is not None)
    chosen_user = None
    match_uuid = uuid is not None
    print 'match_uuid:',match_uuid,'uuid:',uuid
    for user in _users:
        if match_uuid:
            if uuid == user[0]:
                chosen_user = user
                break
        elif username == user[1]:
            chosen_user = user
            break
    if chosen_user is None:
        raise KeyError("User {} not found in DB".format(uuid if match_uuid else username))
    return chosen_user

def update_user(user):
    idx = 0
    for i, u in enumerate(_users):
        if u[0] == user.uuid:
            idx = i
            break
    _users[idx] = \
        (user.uuid, user.username, user.hashed_pass, \
        user.post_ids, user.follower_ids, user.following_ids)

def add_post(user_id, post_content, timestamp):
    """
    Add a post with user's uuid, some content, and a unix time it was created.
    
    Returns the tuple (post_id, user_id, post_content, timestamp)

    WARNING: Currently, this is not threadsafe. Once there is a proper backend,
             this must be fixed.
    """
    post_id = len(_posts)
    p = (post_id, user_id, post_content, timestamp)
    _posts.append(p)
    return p

def get_post(post_id=None):
    """
    Get a post with a given id.
    
    Raises a KeyError if the id does not exist in the database
    """
    for post in _posts:
        if post[0] == post_id:
            return post
    raise KeyError("No post found with id: {}".format(post_id))
