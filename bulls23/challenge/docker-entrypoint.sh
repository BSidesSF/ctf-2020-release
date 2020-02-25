# I know it's dirty to have two services running
# in one Docker container, but I'm just too faded
# to separate this out.
echo "Starting telnet service..."
/etc/init.d/xinetd restart
echo "Starting websockify proxy..."
websockify 0.0.0.0:8888 127.0.0.1:23