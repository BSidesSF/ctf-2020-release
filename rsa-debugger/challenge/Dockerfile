FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY rsa /home/ctf
COPY flag.txt /home/ctf
RUN chmod 555 /home/ctf/rsa && \
  chmod 444 /home/ctf/flag.txt

USER ctf

EXPOSE 1717

WORKDIR /home/ctf
CMD ["/usr/bin/socat", "TCP4-LISTEN:1717,reuseaddr,fork", "EXEC:/home/ctf/rsa,stderr"]
