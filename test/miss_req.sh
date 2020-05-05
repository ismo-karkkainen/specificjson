#!/bin/sh

specificjson --clean --build clean
./clean --import $1
STATUS=$?
./clean --import $1 --warn
WARN=$?
rm -f clean
if [ $STATUS -eq 0 -o $WARN -ne 0 ]; then
    exit 1
fi
