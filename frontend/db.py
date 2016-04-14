import socket

def send_request(req):
    s = socket.socket()
    host = 'localhost'
    port = 3001
    s.connect((host, port))
    s.send(req + '\n')
    print "Sending request: {}".format(req)
    resp = ""
    while not resp.endswith("DONE\n"):
        resp += s.recv(1024)
    s.close()
    r = '\n'.join(resp.split('\n')[:-2])
    return r

def parse_user_reply(s):
    if s == "NOT FOUND":
        raise KeyError("No such user!!")
    elif s == "ALREADY EXISTS":
        raise ValueError("User already exists!!");
    uuid, username, hashed_pass = s.strip().split(' ')
    uuid = int(uuid);
    username = username.decode('hex')
    return (uuid, username, hashed_pass)

def parse_post_reply(s):
    if s == "NOT FOUND":
        raise KeyError("No such post!!")
    try:
        post_id, user_id, content, timestamp = s.strip().split(' ')
        post_id = int(post_id);
        user_id = int(user_id);
        content = content.decode('hex')
        timestamp = int(timestamp)
        return (post_id, user_id, content, timestamp)
    except:
        raise ValueError("no >:(");

def parse_follow_reply(s):
    if s == "NOT FOUND":
        raise KeyError("No such follow!!")
    elif s == "ALREADY EXISTS":
        raise ValueError("Follow already exists!!");
    follower_id, followed_id = s.strip().split(' ')
    follower_id = int(follower_id);
    followed_id = int(followed_id);
    return (follower_id, followed_id)

def add_user(username, hashed_pass):
    """
    Add a user with a given username and hashed password
    
    Returns the tuple (uuid, username, hashed_pass)

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
    u = parse_user_reply(send_request("ADD USER " + username.encode('hex') + " " + hashed_pass))
    return u

def get_user(uuid=None, username=None):
    """
    Get a user from the database with either a matching uuid or username.

    raises a KeyError if the user cannot be found
    """
    assert(uuid is not None or username is not None)
    match_uuid = uuid is not None
    print 'match_uuid:',match_uuid,'uuid:',uuid
    if match_uuid:
        resp = send_request("GET USER BY USER_ID " + str(uuid) + '\n')
    else:
        resp = send_request("GET USER BY USERNAME " + username.encode('hex') + '\n')
    try:
        chosen_user = parse_user_reply(resp)
    except KeyError:
        raise KeyError("User {} not found in DB".format(uuid if match_uuid else username))
    return chosen_user

def add_post(user_id, post_content, timestamp):
    """
    Add a post with user's uuid, some content, and a unix time it was created.
    
    Returns the tuple (post_id, user_id, post_content, timestamp)

    WARNING: Currently, this is not threadsafe. Once there is a proper backend,
             this must be fixed.
    """
    p = parse_post_reply(send_request("ADD POST " + str(user_id) + " " + post_content.encode('hex') + " " + str(timestamp) + '\n'))
    return p

def get_post(post_id=None):
    """
    Get a post with a given id.
    
    Raises a KeyError if the id does not exist in the database
    """
    try:
        return parse_post_reply(send_request("GET POST BY POST_ID " + str(post_id)))
    except:
        raise KeyError("No post found with id: {}".format(post_id))

def get_users_post_ids(uuid=None):
    post_ids = send_request("GET POST BY USER_ID " + str(uuid))
    return [parse_post_reply(x)[0] for x in post_ids.split("\n")]

def add_follow(followed_id, follower_id):
    return parse_follow_reply(send_request("ADD FOLLOW " + str(followed_id) + " " + str(follower_id) + "\n"))

def get_users_followed_ids(follower_id):
    follows = send_request("GET FOLLOWS BY FOLLOWER_ID " + str(follower_id) + "\n")
    print follows.split('\n')
    return [parse_follow_reply(x)[0] for x in follows.split("\n") if x]

def get_users_follower_ids(followed_id):
    follows = send_request("GET FOLLOWS BY FOLLOWED_ID " + str(followed_id) + "\n")
    print follows.split('\n')
    return [parse_follow_reply(x)[1] for x in follows.split("\n") if x]
