FROM ubuntu:latest

RUN apt-get update -y && \
    apt-get install -y python-pip python-dev

COPY ./requirements.txt /app/requirements.txt

WORKDIR /app

RUN pip install -r requirements.txt

COPY . /app

EXPOSE 8000

ENTRYPOINT [ "python" ]

CMD [ "answer_phone.py" ]
