FROM debian:10-slim

RUN adduser --disabled-password --gecos '' ctf

RUN apt-get update && apt-get install -y xinetd

WORKDIR /home/ctf

ADD rusty flag.txt /home/ctf/

RUN chown -R root:root /home/ctf
RUN chmod 0755 /home/ctf /home/ctf/rusty
RUN chmod 0644 /home/ctf/flag.txt
RUN ls -l /home/ctf

ADD rusty.xinetd /etc/xinetd.d/rusty

USER root
EXPOSE 8833

CMD service xinetd restart && sleep infinity
