FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY --chown=root:ctf flag.txt /home/ctf/flag.txt
COPY --chown=root:ctf src/adder /home/ctf/adder
RUN chmod 555 /home/ctf/adder && chmod 440 /home/ctf/flag.txt

USER ctf

EXPOSE 12345

CMD ["/usr/bin/socat", "TCP4-LISTEN:12345,reuseaddr,fork", "EXEC:/home/ctf/adder,stderr"]
