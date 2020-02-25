FROM i386/debian:10-slim

RUN adduser --disabled-password --gecos '' ctf

RUN apt-get update && apt-get install -y xinetd procps netcat-openbsd tcpdump

WORKDIR /home/ctf

ADD src/gman src/flag.txt src/highscores.txt /home/ctf/

RUN chown -R root:root /home/ctf
RUN chmod 0755 /home/ctf /home/ctf/gman
RUN chmod 0444 /home/ctf/flag.txt
RUN chmod 0666 /home/ctf/highscores.txt
RUN ls -l /home/ctf

ADD gman.xinetd /etc/xinetd.d/gman
RUN chmod 0444 /etc/xinetd.d/gman

# Needed to render ncurses properly
ENV TERM=screen-256color

USER ctf
EXPOSE 1337

CMD /usr/sbin/xinetd -pidfile /run/xinetd.pid -dontfork -inetd_compat
