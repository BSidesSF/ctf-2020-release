FROM bsidessfctf/base:latest

COPY --chown=root:ctf ruler /home/ctf/ruler
COPY --chown=root:ctf flag.txt /home/ctf/flag.txt
RUN chmod 440 /home/ctf/flag.txt && \
  chmod 550 /home/ctf/ruler

USER ctf

EXPOSE 4141

CMD ["/usr/bin/socat", "TCP4-LISTEN:4141,reuseaddr,fork", "EXEC:/home/ctf/ruler,stderr"]
