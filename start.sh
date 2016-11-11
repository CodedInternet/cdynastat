#!/usr/bin/env bash

# Enable i2c-1
if ! [ -a /dev/i2c-1 ]; then
    echo cape-universaln > /sys/devices/platform/bone_capemgr/slots && /app/src/cdynastat
fi

# Setup local clock in case we are offline
echo "Started at " $(date) | tee -a /data/start.log

# Start application
/app/src/cdynastat