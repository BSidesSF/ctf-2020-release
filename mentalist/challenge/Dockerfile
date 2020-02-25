FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY mentalist /home/ctf
COPY flag.txt /home/ctf
RUN chmod 555 /home/ctf/mentalist && \
  chmod 444 /home/ctf/flag.txt

USER ctf

EXPOSE 12345

WORKDIR /home/ctf
CMD ["/usr/bin/socat", "TCP4-LISTEN:12345,reuseaddr,fork", "EXEC:/home/ctf/mentalist,stderr"]
