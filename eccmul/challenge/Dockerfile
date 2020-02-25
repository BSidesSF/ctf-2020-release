FROM gcr.io/bsides-sf-ctf-2020/base:latest

RUN apt install -yy pari-gp perl

COPY --chown=root:ctf flag.txt /home/ctf/
COPY --chown=root:ctf eccmul.pl /home/ctf/
COPY --chown=root:ctf banner.txt /home/ctf/
COPY --chown=root:ctf gen_curve.gp /home/ctf
RUN chmod 555 /home/ctf/eccmul.pl && \
  chmod 444 /home/ctf/flag.txt && \
  chmod 444 /home/ctf/banner.txt && \
  chmod 444 /home/ctf/gen_curve.gp

WORKDIR /home/ctf

USER ctf

EXPOSE 25519

CMD ["/usr/bin/socat", "TCP4-LISTEN:25519,reuseaddr,fork", "EXEC:/home/ctf/eccmul.pl,stderr"]
