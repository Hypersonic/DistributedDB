from flask import Flask, render_template, request, redirect, url_for, session
from models import User, Post, Follow
from db import get_users_post_ids, get_users_followed_ids, get_users_follower_ids

app = Flask(__name__)
app.secret_key = "blerpdy derpdy herpdy shmerpdy"

@app.route('/')
def index():
    user = None
    followed = None
    followed_user_map = None
    posts = None
    if 'username' in session:
        try:
            user = User.from_username(session['username'])
            followed = []
            followed_user_map = {}
            posts = []
            followed = map(User.from_uuid, get_users_followed_ids(user.uuid))
            followed_user_map = {u.uuid: u for u in followed}
            posts = map(Post.from_post_id, sum([get_users_post_ids(follow.uuid) for follow in followed], []))
            posts.sort(key=lambda p: -p.timestamp)
        except KeyError: # for our in-memory DB, when the application restarts we lose users. for convenience, logout automatically
            session.pop('username')
    return render_template('index.html', user=user, posts=posts, followed=followed, followed_user_map=followed_user_map)

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'GET':
        return render_template('login.html')
    try:
        if not 'username' in request.form or not 'password' in request.form:
            raise KeyError("No username or password!! oh noes!")
        user = User.from_username(request.form['username'])
        if not user.verify_password(request.form['password']):
            raise KeyError("Wrong password!")
        session['username'] = user.username
        return redirect(url_for('index'))
    except KeyError as e:
        print e
        return render_template('login.html', login_failed=True)

@app.route('/logout')
def logout():
    if 'username' in session:
        session.pop('username')
    return redirect(url_for('index'))

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'GET':
        return render_template('register.html')
    try:
        if not 'username' in request.form or not 'password' in request.form:
            raise KeyError("No username or password!! oh noes!")
        user = User.new(request.form['username'], User.hash_password(request.form['password']))
        session['username'] = user.username
        return redirect(url_for('index'))
    except KeyError:
        return render_template('register.html', register_failed=True)

@app.route('/post', methods=['POST'])
def post():
    if not 'username' in session:
        return redirect(url_for('index.html'))
    user = User.from_username(session['username'])
    post = Post.new(user, request.form['content'])
    return redirect(url_for('index'))

@app.route('/u/<username>')
def user_page(username):
    try:
        user = User.from_username(username)
        posts = map(Post.from_post_id, get_users_post_ids(user.uuid))
        posts.sort(key=lambda p: -p.timestamp)
        return render_template('user_page.html', user=user, posts=posts)
    except KeyError:
        return render_template('no_such_user.html', username=username)

@app.route('/follow/<int:uuid>')
def follow(uuid):
    if not 'username' in session:
        return redirect(url_for('index'))
    try:
        to_follow = User.from_uuid(uuid)
        follower = User.from_username(session['username'])
        follow = Follow.new(to_follow.uuid, follower.uuid)
        return redirect(url_for('index'))
    except KeyError as e:
        print e
        return render_template('no_such_user.html', uuid=uuid)


if __name__ == '__main__':
    app.run('0.0.0.0', port=9001, debug=True)
