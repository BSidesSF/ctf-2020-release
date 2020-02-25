from flask import Flask, render_template, request, redirect, send_from_directory
from flask_csp.csp import csp_header
from werkzeug.middleware import proxy_fix
import requests

app = Flask(__name__)
app.wsgi_app = proxy_fix.ProxyFix(app.wsgi_app)

# csp two ajax.googleapis.com
@app.route('/')
@app.route('/csp-two')
@csp_header({'connect-src':"*",'default-src':"'self' 'unsafe-inline'",'script-src':"'self' ajax.googleapis.com 'unsafe-eval'"})
def cspTwo():
	return render_template('csp-two.html')

@app.route('/csp-two-result', methods = ['POST','GET'])
@csp_header({'connect-src':"*",'default-src':"'self' 'unsafe-inline'",'script-src':"'self' ajax.googleapis.com 'unsafe-eval'"})
def cspTwoResult():
	payload = "None"
	if request.method == 'POST':
		payload = request.form['payload']
		# todo point to webbot 
		r = requests.post('http://127.0.0.1:3000/submit', data={'url':request.base_url, "payload": payload})
	if request.method == 'GET' and 'admin' in request.cookies and request.cookies.get("admin") == u"f9c56c41eb6896da26b49fbc8c03b4d0":
		payload = request.args.get('payload')
	return render_template('csp-two-result.html', payload = payload)

@app.route('/csp-two-flag', methods = ['GET'])
@csp_header({'connect-src':"*",'default-src':"'self' ajax.googleapis.com",'script-src':"'self' ajax.googleapis.com"})
def cspTwoFlag():
	if 'admin' in request.cookies and request.cookies.get("admin") == u"f9c56c41eb6896da26b49fbc8c03b4d0":
		return "CTF{Canned_Spam_Perfection}"
	else:
		return "Ah ah ah, you didn't say the magic word"

app.debug = True
app.run(host="0.0.0.0", port=8000)
