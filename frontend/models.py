from db import add_user, get_user, update_user, add_post, get_post
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
        self.uuid = None
        self.username = None
        self.hashed_pass = None
        self.post_ids = None
        self.follower_ids = None
        self.following_ids = None

    def verify_password(self, password):
        """
        Verify whether or not an **UNHASHED** password matches our internal hashed password
        """
        return self.hashed_pass == User.hash_password(password)

    def commit(self):
        update_user(self)

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
        u.uuid = db_data[0]
        u.username = db_data[1]
        u.hashed_pass = db_data[2]
        u.post_ids = db_data[3]
        u.follower_ids = db_data[4]
        u.following_ids = db_data[5]
        return u
    
    @staticmethod
    def from_uuid(uuid):
        """
        Returns a user from the database with the associated uuid
        
        raises a KeyError if the user does not exist in the database
        """
        db_data = get_user(uuid=uuid)
        u = User()
        u.uuid = db_data[0]
        u.username = db_data[1]
        u.hashed_pass = db_data[2]
        u.post_ids = db_data[3]
        u.follower_ids = db_data[4]
        u.following_ids = db_data[5]
        return u

    @staticmethod
    def from_username(username):
        """
        Returns a user from the database with the associated username

        raises a KeyError if the user does not exist in the database
        """
        db_data = get_user(username=username)
        u = User()
        u.uuid = db_data[0]
        u.username = db_data[1]
        u.hashed_pass = db_data[2]
        u.post_ids = db_data[3]
        u.follower_ids = db_data[4]
        u.following_ids = db_data[5]
        return u

class Post(object):
    def __init__(self):
        self.post_id = None
        self.user_id = None
        self.content = None
        self.timestamp = None

    @staticmethod
    def new(user, content):
        """
        Create a new post for a user with given content

        Raises a KeyError if the user already exists
        """
        now = int(time())
        db_data = add_post(user.uuid, content, now)
        p = Post()
        p.post_id = db_data[0]
        p.user_id = db_data[1]
        p.content = db_data[2]
        p.timestamp = db_data[3]
        return p

    @staticmethod
    def from_post_id(post_id):
        """
        Returns a post from the database with the associated post_id

        raises a KeyError if the post does not exist in the database
        """
        db_data = get_post(post_id=post_id)
        p = Post()
        p.post_id = db_data[0]
        p.user_id = db_data[1]
        p.content = db_data[2]
        p.timestamp = db_data[3]
        p.timestamp = db_data[3]
        return p
