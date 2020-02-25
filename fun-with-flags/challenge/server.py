import requests
import os
from flask import Flask, render_template, request, redirect, flash
from flask_csp.csp import csp_header
from flask_wtf import FlaskForm, CSRFProtect
from wtforms import StringField, PasswordField, SubmitField, TextAreaField
from wtforms.ext.sqlalchemy.fields import QuerySelectField
from wtforms.validators import DataRequired, EqualTo, ValidationError, Regexp, Length
from flask_login import UserMixin, logout_user, login_user, LoginManager, login_required, current_user
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy.orm import relationship
from werkzeug.security import generate_password_hash, check_password_hash
from flask_wtf.csrf import CSRFError
from sqlalchemy.exc import InterfaceError 
from bleach import clean
from markupsafe import Markup
from werkzeug.middleware import proxy_fix


# Flask App initialization 
app = Flask(__name__)
app.wsgi_app = proxy_fix.ProxyFix(app.wsgi_app)

# Flask_login initialization
login_manager = LoginManager()
login_manager.init_app(app)

# Secret key, also used for CSRF token
app.secret_key = b'_corgi_'
csrf = CSRFProtect(app)

# Database setup 
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///database.sqlite"
db = SQLAlchemy(app)

# User model
class User(db.Model, UserMixin):
    __tablename__ = 'users'
    id = db.Column(db.Integer, primary_key=True, index=True)
    # email = db.Column(db.String(255), nullable=False, unique=True)
    username = db.Column(db.String(50), nullable=False, unique=True)
    password_hash = db.Column(db.String(255), nullable=False)
    def set_password(self, password):
        self.password_hash = generate_password_hash(password)
    def check_password(self, password):
        return check_password_hash(self.password_hash, password)

    def __repr__(self):
        return self.username
    


class Message(db.Model):
    __tablename__ = 'messages'
    id = db.Column(db.Integer, primary_key=True, index=True)
    title = db.Column(db.String(50), unique=False)
    content = db.Column(db.Text)
    to_user = db.Column(db.ForeignKey('users.id'), index=True)
    from_user = db.Column(db.Integer)
    from_username = db.Column(db.Text)
    users = db.relationship(User)
    


db.create_all()

# Forms used by the application

class LoginForm(FlaskForm):
    class Meta:
        csrf=False
    username = StringField('Username', validators=[DataRequired(), Regexp('^\w+$', message="Username must be AlphaNumeric")])
    password = PasswordField('Password', validators=[DataRequired()])
    submit = SubmitField('Login')

class RegistrationForm(FlaskForm):
    class Meta:
        csrf=False
    username = StringField('Username', validators=[DataRequired(), Regexp('^\w+$', message="Username must be AlphaNumeric")])
    # email = StringField('Email Address', validators=[DataRequired(), Email()])
    password = PasswordField('New Password', 
        validators=[DataRequired()])
    confirm = PasswordField('Repeat Password', validators=[DataRequired(), EqualTo('password', message='Passwords must match')])
    submit = SubmitField('Register')

    def validate_username(self, username):
        user = User.query.filter_by(username=username.data).first()
        if user is not None:
            raise ValidationError('Please use a different username.')

class MessageForm(FlaskForm):
    to_user =  QuerySelectField(
        'Send to',
        query_factory=lambda: User.query,
        get_label='username',
        allow_blank=False
    )
    title = StringField('Title', validators=[DataRequired(), Length(min=1,max=25,message='Title should be between 1-25 characters.')])
    content = TextAreaField('Message' ,validators=[DataRequired()])
    submit = SubmitField('Send')

# Route handlers 
@app.route('/')
@app.route('/login', methods=['GET', 'POST'])
@csrf.exempt
@csp_header({'default-src':"'self'"})
def login():
    form = LoginForm()
    if form.validate_on_submit():
        user = User.query.filter_by(username=form.username.data).first()
        if user is None or not user.check_password(form.password.data):
            flash('Invalid username or password')
            return redirect('/login')
        login_user(user)
        flash('Logged in successfully.')
        return redirect('/home')
    return render_template('login.html', form=form)

@app.route('/register', methods=['GET', 'POST'])
@csp_header({'default-src':"'self'"})
@csrf.exempt
def register():
    form = RegistrationForm()
    if form.validate_on_submit():
        user = User(username=form.username.data)
        user.set_password(form.password.data)
        db.session.add(user)
        db.session.commit()
        flash('Thanks for registering')
        return redirect('/login')
    return render_template('register.html', form=form)

@app.route('/home', methods=['GET','POST'])
@csp_header({'img-src':"*",'default-src':"'self'",'script-src':"'self'", 'style-src':"'self' 'unsafe-inline'"})
@login_required
def home():
    flag = "Try reading this value"
    form = MessageForm()
    if form.validate_on_submit():
        msg = Message(title=form.title.data,content=form.content.data,to_user=form.to_user.data.id,from_user=current_user.id, from_username=current_user.username)
        db.session.add(msg)
        db.session.commit()
        if form.to_user.data.id == 1:
            db.session.refresh(msg)
            webbot_ip= os.environ.get("WEBBOT_IP", "http://127.0.0.1:3000")
            r = requests.post(webbot_ip + "/submit", data = {'url':request.url_root  + "display-message?id=" + str(msg.id)})
        return redirect('/home')
    messages = Message.query.filter_by(to_user=current_user.id).all()
    return render_template('home.html', current_user=current_user, form=form, messages=messages, flag=flag)

@app.route('/display-message', methods=['GET'])
@csp_header({'img-src':"*",'default-src':"'self'",'script-src':"'self'", 'style-src':"'self' 'unsafe-inline'"})
@login_required
def displayMessage():
    form = MessageForm()
    if current_user.id != 1:
        print 'Not Admin'
        return redirect('/home')
    flag = "CTF{Clandestine_Secret_Stealing}"
    id = request.args.get('id')
    message = Message.query.filter_by(id=id).first()
    return render_template('message.html', current_user=current_user, message=message, flag=flag, form=form)

@app.route('/logout', methods=['GET'])
def logout():
    logout_user()
    return redirect('/login')

@login_manager.user_loader
def load_user(id):
    return User.query.get(int(id))

@app.errorhandler(CSRFError)
def handle_csrf_error(e):
    return render_template('csrf_error.html', reason=e.description), 400

# Restricting allowed tags 
def do_clean(text, **kw):
    """Perform clean and return a Markup object to mark the string as safe.
    This prevents Jinja from re-escaping the result."""
    return Markup(clean(text, **kw))

app.jinja_env.filters['clean'] = do_clean

app.run(host='0.0.0.0', port=8000)

