FROM gcr.io/bsides-sf-ctf-2020/base:latest

RUN dpkg --add-architecture i386 && \
  apt-get -y update && \
  apt-get -y install apache2 libc6:i386 libssl1.1:i386 && \
  a2enmod cgid && \
  apt-get clean

COPY --chown=root:ctf flag.txt /home/ctf/flag.txt
COPY --chown=root:ctf html /var/www/html/
COPY --chown=root:ctf hashpop /var/www/html/cgi-bin/hashpop
COPY 000-default.conf /etc/apache2/sites-enabled

RUN chmod 444 /home/ctf/flag.txt && chmod 555 /home/ctf && chmod -R 755 /var/www/html

EXPOSE 80
CMD ["/usr/sbin/apache2ctl", "-DFOREGROUND"]
