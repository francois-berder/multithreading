#!/bin/sh

if [ $# -lt 2 ]; then
    echo "Usage: $0 <jlinkdevice> <hexfile>"
    exit 1
fi

JLINK_DEVICE=$1
HEXFILE=$2

CMDFILE=$(mktemp /tmp/jlink.XXXXX)

echo "r\nloadfile ${HEXFILE}\nr\nq\n" > ${CMDFILE}
JLinkExe -device ${JLINK_DEVICE} -speed 12000 -if SWD -jtagconf -1,-1 -Autoconnect 1 -ExitOnError -commandfile ${CMDFILE}
rm -f ${CMDFILE}
