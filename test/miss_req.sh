#!/bin/sh

specificjson --input $1
if [ $? -eq 0 ]; then
    exit 1
fi
