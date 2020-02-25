from flask import Flask, render_template, request, redirect, send_from_directory
from flask_csp.csp import csp_header
from werkzeug.middleware import proxy_fix
import requests

app = Flask(__name__)
app.wsgi_app = proxy_fix.ProxyFix(app.wsgi_app)

# csp one (data uri) use cookie e397d059d7148ad6ecacdf4af7a1deda
@app.route('/')
@app.route('/csp-one')
@csp_header({'connect-src':"*",'script-src':"'self' data:"})
def cspOne():
	return render_template('csp-one.html')

@app.route('/csp-one-result', methods = ['POST','GET'])
@csp_header({'connect-src':"*",'script-src':"'self' data:"})
def cspOneResult():
	payload = "None"
	if request.method == 'POST':
		payload = request.form['payload']
		r = requests.post('http://127.0.0.1:3000/submit', data={'url': request.base_url, "payload": payload})
	if request.method == 'GET' and 'admin' in request.cookies and request.cookies.get("admin") == u"e397d059d7148ad6ecacdf4af7a1deda":
		payload = request.args.get('payload')
	elif request.method == 'GET':
	    app.logger.warning('GET request without valid admin cookie.')
	return render_template('csp-one-result.html', payload = payload)

@app.route('/csp-one-flag', methods = ['GET'])
def cspOneFlag():
	if 'admin' in request.cookies and request.cookies.get("admin") == u"e397d059d7148ad6ecacdf4af7a1deda":
		return "CTF{Cant_Stop_Pwning}"
	else:
		return "Ah ah ah, you didn't say the magic word"

app.run(host='0.0.0.0', port=8000)
