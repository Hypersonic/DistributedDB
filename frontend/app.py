from flask import Flask, render_template, request, redirect, url_for, session, flash
from models import User, Post, Follow
from db import update_dbs

app = Flask(__name__)
app.secret_key = "blerpdy derpdy herpdy shmerpdy"

def requires_login(f, *args, **kwargs):
    if not 'username' in session:
        return redirect(url_for('index'))
    else:
        return f(*args, **kwargs)


@app.route('/')
def index():
    return "Hi"

@app.route('/login')
def login():
    if request.method == 'GET':
        return 'pls to login'
    elif request.method == 'POST':
        print 'k'
        return 'cool loggin u in breh'

if __name__ == '__main__':
    update_dbs() # get all possible databases
    app.run('0.0.0.0', port=9002, debug=True)
