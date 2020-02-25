# Pull base image.
FROM debian:10-slim

# Install packages
RUN apt-get update && apt-get install -y curl procps git

# Install Meteor
RUN curl -k https://install.meteor.com/ | sh

# Create app directory + upload app data
RUN mkdir -p /app
COPY . /app

# Allow root stuff
ENV METEOR_ALLOW_SUPERUSER 1

# Start meteor from /app
WORKDIR /app

# Install node modules
RUN meteor npm install

# Runs on port 3000
EXPOSE 3000

# Run the app
CMD [ "meteor" ]
