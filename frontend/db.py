import socket
import random

known_dbs = [('localhost', 3004)] # need to have at least 1 initial known db to seed from

def update_dbs():
    global known_dbs
    serv = None
    while not serv:
        s = socket.socket()
        serv = random.choice(known_dbs)
        try:
            s.connect(serv)
        except socket.error: # connection issue
            print "Server is dead, removing:",serv
            known_dbs.remove(serv)
            serv = None
    s.send("MGMT\nGET_HOSTS\nDONE\n")
    resp = ""
    while not resp.endswith("DONE\n"):
        resp += s.recv(1024)
    s.close()
    unparsed = resp.split('\n')[:-2]
    new_dbs = []
    for serv in unparsed:
        ip, port, s_id = serv.split(':')
        new_dbs.append((ip, int(port)))
    known_dbs = new_dbs
    return new_dbs


def send_request(req):
    #there's a potential race condition here where we can update dbs,
    # then a node can go down, and we try to query that node.
    # oh well :/
    dbs = update_dbs() # we'll update dbs before every request because why not
    s = socket.socket()
    serv = random.choice(dbs)
    print "Making request to:",serv
    s.connect(serv)
    s.send("QUERY\n" + req + '\nDONE\n')
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
    username, hashed_pass = s.strip().split(' ')
    username = username.decode('hex')
    return (username, hashed_pass)

def parse_post_reply(s):
    if s == "NOT FOUND":
        raise KeyError("No such post!!")
    try:
        post_user, content, timestamp = s.strip().split(' ')
        post_user = post_user.decode('hex')
        content = content.decode('hex')
        timestamp = int(timestamp)
        return (post_user, content, timestamp)
    except:
        raise ValueError("no >:(");

def parse_follow_reply(s):
    if s == "NOT FOUND":
        raise KeyError("No such follow!!")
    elif s == "ALREADY EXISTS":
        raise ValueError("Follow already exists!!");
    follower_username, followed_username = s.strip().split(' ')
    follower_username = follower_username.decode('hex')
    followed_username = followed_username.decode('hex')
    return (follower_username, followed_username)

def add_user(username, hashed_pass):
    """
    Add a user with a given username and hashed password
    
    Returns the tuple (username, hashed_pass)

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

def get_user(username):
    """
    Get a user from the database with a matching username.

    raises a KeyError if the user cannot be found
    """
    resp = send_request("GET USER BY USERNAME " + username.encode('hex') + '\n')
    try:
        chosen_user = parse_user_reply(resp)
    except KeyError:
        raise KeyError("User {} not found in DB".format(username))
    return chosen_user

def add_post(username, post_content, timestamp):
    """
    Add a post with user's username, some content, and a unix time it was created.
    
    Returns the tuple (post_id, user_id, post_content, timestamp)

    WARNING: Currently, this is not threadsafe. Once there is a proper backend,
             this must be fixed.
    """
    p = parse_post_reply(send_request("ADD POST " + username.encode('hex') + " " + post_content.encode('hex') + " " + str(timestamp) + '\n'))
    return p

def get_users_posts(username):
    posts = send_request("GET POST BY USERNAME " + username.encode('hex'))
    try:
        return [parse_post_reply(x) for x in posts.split("\n")]
    except ValueError: # no posts :(
        return []

def add_follow(followed_username, follower_username):
    return parse_follow_reply(send_request("ADD FOLLOW " + followed_username.encode('hex') + " " + follower_username.encode('hex') + "\n"))

def get_users_followed_usernames(follower_username):
    follows = send_request("GET FOLLOWS BY FOLLOWER " + follower_username.encode('hex') + "\n")
    return [parse_follow_reply(x)[0] for x in follows.split("\n") if x]

def get_users_follower_usernames(followed_username):
    follows = send_request("GET FOLLOWS BY FOLLOWED " + followed_username.encode('hex') + "\n")
    return [parse_follow_reply(x)[1] for x in follows.split("\n") if x]
