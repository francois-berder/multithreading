#!/bin/sh

if [ $# -lt 2 ]; then
    echo "Usage: $0 <jlinkdevice> <elffile>"
    exit 1
fi

JLINK_DEVICE=$1
ELFFILE=$2
GDBSCRIPT=$(mktemp /tmp/gdb.XXXXX)

echo "set confirm off\ntarget remote localhost:3333" > ${GDBSCRIPT}

trap "" INT

setsid sh -c "JLinkGDBServer -silent -device ${JLINK_DEVICE} -speed 4000 -if SWD -port 3333" &
arm-none-eabi-gdb -q -x ${GDBSCRIPT} ${ELFFILE}
killall JLinkGDBServer
rm -f ${GDBSCRIPT}
