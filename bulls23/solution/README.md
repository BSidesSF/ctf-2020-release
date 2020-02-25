# Bulls23 Solution

The solution for this challenge is the following:

* Open the `distfiles/challenge.pcapng` with Wireshark
* Set a filter of `(websocket)` to only view the WebSocket traffic. There are two WebSocket connections in the capture, one login that failed and one that succeeded with the appropriate password.
* Find the second (successful) Websocket connection, specifically at `No. 1335` which has `WebSocket Binary [FIN] [MASKED]` in the `Info` description.
* Once you've selected the line, near the bottom of Wireshark you'll find two tabs, one labeled `Frame (XX bytes)` and one labeled `Unmasked Data (X bytes)`.
* Click on the `Unmasked Data (X bytes)` tab to see the decoded ASCII character of the telnet password. Sadly, you'll have to do this for each character of the password and you'll have to re-click the `Unmasked Data (X bytes)` tab each time.

You'll eventually get the password of `ib3atm0nstar5` for the telnet user `michaeljordan`. You use this to log in to the Websocket port which is serving telnet on `8888`. The easiest way to do this is to use the [Websockify.js JavaScript client implementation](https://github.com/novnc/websockify-js/blob/master/wstelnet.html) (broken at the moment, but can be fixed if you know JavaScript at all). Once you log in successfully you'll get the CTF flag returned to you as the telnet banner.