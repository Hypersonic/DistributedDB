from db import add_user, get_user, add_post, add_follow, get_users_posts, get_users_followed_usernames, get_users_follower_usernames
from time import time
from hashlib import sha1

class User(object):
    """
    Model for a user.
    """
    def __init__(self):
        """
        Don't call this constructor directly -- use one of the User.from_X or User.new functions
        """
        self.username = None
        self.hashed_pass = None

    def verify_password(self, password):
        """
        Verify whether or not an **UNHASHED** password matches our internal hashed password
        """
        return self.hashed_pass == User.hash_password(password)

    @staticmethod
    def hash_password(password):
        """
        Return a hashed password
        """
        return sha1(password).hexdigest()

    @staticmethod
    def new(username, hashed_pass):
        """
        Create a new user with a username and hashed password

        Returns the user

        raises a ValueError if the username is already taken
        """
        db_data = add_user(username, hashed_pass)
        u = User()
        u.username = db_data[0]
        u.hashed_pass = db_data[1]
        return u
    
    @staticmethod
    def from_username(username):
        """
        Returns a user from the database with the associated username

        raises a KeyError if the user does not exist in the database
        """
        db_data = get_user(username)
        u = User()
        u.username = db_data[0]
        u.hashed_pass = db_data[1]
        return u

    def get_posts(self):
        posts = []
        for post in get_users_posts(self.username):
            p = Post()
            p.username, p.content, p.timestamp = post;
            posts.append(p)
        return posts

    def get_followers(self):
        # caution! This takes n+1 queries, with n being the number of follows our poor user has
        followers = []
        for follower_username in get_users_follower_usernames(self.username):
            followers.append(User.from_username(follower_username))
        return followers

    def get_follows(self):
        # caution! This takes n+1 queries, with n being the number of follows our poor user has
        follows = []
        for followed_username in get_users_followed_usernames(self.username):
            follows.append(User.from_username(followed_username))
        return follows

class Post(object):
    def __init__(self):
        self.username = None
        self.content = None
        self.timestamp = None

    @staticmethod
    def new(user, content):
        """
        Create a new post for a user with given content

        Raises a KeyError if the user already exists
        """
        now = int(time())
        db_data = add_post(user.username, content, now)
        p = Post()
        p.username = db_data[0]
        p.content = db_data[1]
        p.timestamp = db_data[2]
        return p


class Follow(object):
    def __init__(self):
        self.followed_username = None
        self.follower_username = None

    @staticmethod
    def new(followed_username, follower_username):
        db_data = add_follow(followed_username, follower_username)
        p = Follow()
        p.followed_username = db_data[0]
        p.follower_username = db_data[1]
        return p
