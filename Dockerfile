FROM resin/beaglebone-debian:latest
# Enable systemd
ENV INITSYSTEM on

RUN apt-get update
RUN apt-get install -y cmake libx11-dev libboost-thread-dev libboost-system-dev libyaml-cpp-dev

RUN cd /app
RUN wget -N https://ci-cloud.s3-eu-west-1.amazonaws.com/webrtc/libwebrtc-Linux-arm-r49.a -P lib/
RUN cmake . -DARCH=arm
RUN make src/cdynastat

CMD "/app/src/cdynastat"
