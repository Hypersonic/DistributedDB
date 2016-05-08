from flask import Flask, render_template, request, redirect, url_for, session, flash
from models import User, Post, Follow
from db import update_dbs
from functools import wraps

app = Flask(__name__)
app.secret_key = "blerpdy derpdy herpdy shmerpdy"

def requires_login(f, *args, **kwargs):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if not 'username' in session:
            flash("You must be logged in to view this page!")
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

def requires_not_login(f, *args, **kwargs):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'username' in session:
            return redirect(url_for('profile'))
        return f(*args, **kwargs)
    return decorated_function

@app.route('/')
def index():
    posts = None
    if 'username' in session:
        u = User.from_username(session['username'])
        followed = u.get_follows()
        posts = sum([x.get_posts() for x in followed], []) + u.get_posts()
        posts.sort(key=lambda x: -x.timestamp) # sort in reverse chronological order
    import time
    return render_template('index.html', posts=posts, time=time)

@app.route('/login', methods=['GET', 'POST'])
@requires_not_login
def login():
    if request.method == 'GET':
        return render_template('login.html')
    elif request.method == 'POST':
        try: 
            u = User.from_username(request.form['username'])
            if u.verify_password(request.form['password']):
                session['username'] = request.form['username']
                return redirect(url_for('profile'))
            # wrong password
            flash("Incorrect username or password!")
            return redirect(url_for('login'))
        except KeyError: # user already exists 
            flash("Incorrect username or password!")
            return redirect(url_for('login'))


@app.route('/register', methods=['GET', 'POST'])
@requires_not_login
def register():
    if request.method == 'GET':
        return render_template('register.html')
    elif request.method == 'POST':
        try:
            u = User.new(request.form['username'], User.hash_password(request.form['password']))
            session['username'] = u.username # log user in
            return redirect(url_for('profile'))
        except KeyError: # user already exists
            flash("This user already exists... perhaps another username would be more appropriate?")
            return redirect(url_for('register'))

@app.route('/profile')
@requires_login
def profile():
    try:
        user = User.from_username(session['username'])
        return render_template('profile.html', user=user)
    except KeyError: # user doesn't actually exist
        session.pop('username')
        return redirect(url_for('login'))

@app.route('/profile/<username>')
def user_profile(username):
    try:
        user = User.from_username(username)
        return render_template('profile.html', user=user)
    except KeyError: # user doesn't exist
        return render_template('404.html')


@app.route('/logout')
@requires_login
def logout():
    session.pop('username')
    return redirect(url_for('index'))

@app.route('/post', methods=['POST'])
@requires_login
def post():
    user = User.from_username(session['username'])
    user.make_post(request.form['content'])
    return redirect(url_for('profile'))

@app.route('/follow/<username>')
@requires_login
def follow(username):
    try:
        Follow.new(username, session['username'])
    except ValueError: # already following
        flash("You're already following that user!!!\n")
    return redirect(url_for('user_profile', username=username))

if __name__ == '__main__':
    update_dbs() # get all possible databases
    app.run('0.0.0.0', port=9002, debug=True)
