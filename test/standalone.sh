#!/bin/sh

SPEC=$1
FILENAME=$2
specificjson --input $SPEC
RC=0
for T in $(specificjson --list | cut -d ' ' -f 1)
do
    for E in hpp cpp
    do
        grep $T $FILENAME.$E >/dev/null
        if [ $? -ne 0 ]; then
            echo "Missing $T from $FILENAME.$E"
            RC=1
        fi
    done
done
rm -f $FILENANE.?pp
