FROM resin/beaglebone-debian:latest
# Enable systemd
ENV INITSYSTEM on

# Install packages
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

# Setup and copy app
WORKDIR /app
COPY . /app

# Fetch depenant libraries
RUN wget -N https://ci-cloud.s3-eu-west-1.amazonaws.com/webrtc/libwebrtc-Linux-arm-r49.a -P lib/

# Configure and build system
RUN cmake . -DARCH=arm
RUN make cdynastat

# Requires cape config to enable i2c-1
CMD "echo cape-universaln > /sys/devices/platform/bone_capemgr/slots && /app/src/cdynastat"
