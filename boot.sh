#!/bin/bash
fw0="pru0/gen/pru0.out"
fw1="pru1/gen/pru1.out"

echo 'stop'>/sys/class/remoteproc/remoteproc1/state
echo 'stop'>/sys/class/remoteproc/remoteproc2/state

cp ${fw0} /lib/firmware/am335x-pru0-fw
cp ${fw1} /lib/firmware/am335x-pru1-fw

echo 'start'>/sys/class/remoteproc/remoteproc1/state
echo 'start'>/sys/class/remoteproc/remoteproc2/state

echo ""
echo "Firmware is running"
echo ""

