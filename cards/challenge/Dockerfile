FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY blackjack /home/ctf
COPY flag.txt /home/ctf
COPY src/static /home/ctf/static
RUN dd if=/dev/urandom of=/home/ctf/secret.key bs=32 count=1
RUN chmod 555 /home/ctf/blackjack && \
  chmod 444 /home/ctf/flag.txt && \
  chmod -R a-w,a+rX /home/ctf/static && \
  chmod 444 /home/ctf/secret.key

USER ctf

EXPOSE 8080

WORKDIR /home/ctf
CMD /home/ctf/blackjack
