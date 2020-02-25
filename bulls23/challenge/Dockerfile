FROM ubuntu:18.04

RUN mkdir /work/
WORKDIR /work/

RUN apt-get update

# For debug
RUN apt-get install net-tools -y

RUN apt-get install telnetd xinetd telnet openssl curl build-essential -y

# Set SSH banner
COPY ./telnet-banner /etc/motd

# Remove old banners
RUN chmod -x /etc/update-motd.d/*
RUN mv /etc/legal /etc/legal.old

# Set test user's shell to false
RUN useradd -m michaeljordan -p "$(openssl passwd -1 ib3atm0nstar5)"
RUN usermod -s /usr/sbin/nologin michaeljordan

RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:deadsnakes/ppa -y
RUN apt-get install -y python3.7 python3-setuptools

COPY ./websockify/ /work/websockify/
WORKDIR /work/websockify/
RUN python3 setup.py install

EXPOSE 8888

COPY telnet /etc/xinetd.d/telnet

COPY docker-entrypoint.sh /work/docker-entrypoint.sh
RUN chmod +x /work/docker-entrypoint.sh

ENTRYPOINT /work/docker-entrypoint.sh