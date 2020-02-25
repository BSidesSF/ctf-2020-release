# Tested exclusively on ruby-2.6.3, so far
FROM ruby:2.6.3

RUN apt-get update
RUN apt-get install -y net-tools ruby-bundler

# Configuration
ENV HOST 0.0.0.0
ENV PORT 4567

# Install gems
ENV APP_HOME /app
ENV HOME /root
COPY server/ $APP_HOME
WORKDIR $APP_HOME

RUN bundle install

# Start server
CMD ["ruby", "./server.rb"]
