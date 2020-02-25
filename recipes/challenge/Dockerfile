FROM gcr.io/bsides-sf-ctf-2020/base:latest

COPY src/recipes /home/ctf/recipes
COPY src/templates /home/ctf/templates
COPY src/data /home/ctf/data
COPY src/static /home/ctf/static

RUN chown -R root:ctf /home/ctf && chmod -R a+rX /home/ctf

EXPOSE 8080

USER ctf

WORKDIR /home/ctf
CMD ["/home/ctf/recipes"]
