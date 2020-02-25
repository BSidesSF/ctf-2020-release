from flask import Flask, request
from twilio.twiml.voice_response import VoiceResponse, Gather, Say, Pause

app = Flask(__name__)

@app.route("/")
def home():
    return ""

@app.route("/voice", methods=['GET', 'POST'])
def voice():
    """Respond to incoming phone calls with a menu of options"""
    # Start our TwiML response
    resp = VoiceResponse()
    # Gather the digits
    gather = Gather(num_digits=6, action='/gather')
    gather.say('Welcome to Corgi bank. Please enter your identification number')
    resp.append(gather)

    # If no digits entered, loop
    resp.redirect('/voice')
    return str(resp)

@app.route('/gather', methods=['GET', 'POST'])
def gather():
    resp = VoiceResponse()
    # If digits were included
    if 'Digits' in request.values:
        choice = request.values['Digits']
        # If correct code
        if choice == '965312':
            resp.say('Identification code for Manager Mark. The flag is.')
            resp.pause(length=1)
            resp.say('C.T.F. Curly brace. D.T.M.F. underscore. I. s. underscore. A.w.e.s.o.m.e. Curly brace.')
            resp.pause(length=2)
            resp.say('That is. C.T.F Curly brace. D.T.M.F. underscore. I.s. underscore. A.w.e.s.o.m.e. Curly brace.')
            resp.pause(length=1)
            resp.say('Goodbye.')
            return str(resp)
        else:
            resp.say('Incorrect identification code. Goodbye!')
            return str(resp)
    # No digits
    resp.redirect('/voice')
    return str(resp)

if __name__ == "__main__":
    app.run(host='0.0.0.0',port=8000)
