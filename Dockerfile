FROM resin/beaglebone-debian:latest
# Enable systemd
ENV INITSYSTEM on

RUN apt-get -y update
RUN apt-get install -y \
    git \
    build-essential \
    cmake \
    libssl-dev \
    libx11-dev \
    libboost-thread-dev \
    libboost-system-dev \
    libyaml-cpp-dev \
    libi2c-dev

WORKDIR /app

COPY . /app

RUN wget -N https://ci-cloud.s3-eu-west-1.amazonaws.com/webrtc/libwebrtc-Linux-arm-r49.a -P lib/
RUN cmake . -DARCH=arm
RUN make cdynastat

CMD "/app/src/cdynastat"
