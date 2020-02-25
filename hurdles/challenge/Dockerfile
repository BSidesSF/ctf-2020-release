FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY --chown=root:ctf challenge /home/ctf/hurdles
RUN chmod 555 /home/ctf/hurdles

EXPOSE 8888

CMD ["/home/ctf/hurdles"]
