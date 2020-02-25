FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY --chown=root:ctf flag.txt /home/ctf/
COPY --chown=root:ctf ripc4 /home/ctf/
RUN chmod 555 /home/ctf/ripc4 && chmod 444 /home/ctf/flag.txt

USER ctf

EXPOSE 8267

CMD ["/usr/bin/socat", "TCP4-LISTEN:8267,reuseaddr,fork", "EXEC:/home/ctf/ripc4,stderr"]
