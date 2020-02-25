FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY --chown=root:ctf flag.txt /home/ctf/
COPY --chown=root:ctf kvaas /home/ctf/
RUN chmod 555 /home/ctf/kvaas && chmod 444 /home/ctf/flag.txt

USER ctf

EXPOSE 3344

CMD ["/usr/bin/socat", "TCP4-LISTEN:3344,reuseaddr,fork", "EXEC:/home/ctf/kvaas,stderr"]
