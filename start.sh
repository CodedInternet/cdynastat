#!/usr/bin/env bash

# Enable i2c-1
if ! [ -a /dev/i2c-1 ]; then
    echo cape-universaln > /sys/devices/platform/bone_capemgr/slots && /app/src/cdynastat
fi

# Setup local clock in case we are offline
echo ds3231 0x68 >/sys/bus/i2c/devices/i2c-1/new_device
sleep 1
hwclock -f /dev/rtc1 -s

# Start application
./src/cdynastat