FROM gcc:9.2

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y cmake libgtest-dev libboost-test-dev && rm -rf /var/lib/apt/lists/*

COPY . /code/
WORKDIR /code
# RUN cmake .
# RUN make factory
# RUN make dashboard
# RUN make ml

EXPOSE 8080