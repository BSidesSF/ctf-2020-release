from flask import Flask, render_template, request, redirect, send_from_directory
from flask_csp.csp import csp_header
from werkzeug.middleware import proxy_fix
import requests

app = Flask(__name__)
app.wsgi_app = proxy_fix.ProxyFix(app.wsgi_app)

# csp two (redirect / path relaxation)
@app.route('/')
@app.route('/csp-three')
@csp_header({'connect-src':"*",'default-src':"'self'",'script-src':"'self' http://storage.googleapis.com/good.js"})
def cspThree():
	return render_template('csp-three.html')

@app.route('/csp-three-result', methods = ['POST','GET'])
@csp_header({'connect-src':"*",'default-src':"'self'",'script-src':"'self' http://storage.googleapis.com/good.js"})
def cspThreeResult():
	payload = "None"
	if request.method == 'POST':
		payload = request.form['payload']
		# todo point to webbot 
		r = requests.post('http://127.0.0.1:3000/submit', data={'url': request.base_url, "payload": payload})
	if request.method == 'GET' and 'admin' in request.cookies and request.cookies.get("admin") == u"55e0ee3ca486ca6b8d11fdd0b21fa714":
		payload = request.args.get('payload')
	return render_template('csp-three-result.html', payload = payload)

@app.route('/csp-three-flag', methods = ['GET'])
@csp_header({'connect-src':"*",'default-src':"'self'",'script-src':"'self' http://storage.googleapis.com/good.js"})
def cspThreeFlag():
	if 'admin' in request.cookies and request.cookies.get("admin") == u"55e0ee3ca486ca6b8d11fdd0b21fa714":
		return "CTF{Cyber_Security_Practitioner}"
	else:
		return "Ah ah ah, you didn't say the magic word"

# open redirect (for csp three) - Listed in robots.txt
@app.route('/redirect')
def openRedirect():
	url = request.args.get('url')
	print url
	if url is None:
		return "URL Get parameter not provided. Try /redirect?url=http://example.com"
	else:
		return redirect(url, code=301)

# Robots.txt
@app.route('/robots.txt')
def robotsStatic():
    return send_from_directory(app.static_folder, 'robots.txt')


app.run(host="0.0.0.0",port=8000)
