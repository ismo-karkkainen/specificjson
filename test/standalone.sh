#!/bin/sh

SPEC=$1
FILENAME=$2
specificjson --input $SPEC
RC=0
for T in $(specificjson --list | cut -d ' ' -f 1 | grep -v '^write')
do
    grep $T $FILENAME.hpp >/dev/null
    if [ $? -ne 0 ]; then
        echo "Missing $T from $FILENAME.hpp"
        RC=1
    fi
done
rm -f $FILENAME.?pp
exit $RC
