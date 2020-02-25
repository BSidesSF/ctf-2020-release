FROM bsidessfctf/base:latest

RUN apt-get update -y && \
    apt-get install -y python-pip python-dev

COPY . /app

WORKDIR /app

RUN pip install -r requirements.txt

EXPOSE 8000

ENTRYPOINT [ "python" ]

CMD [ "server.py" ]
