#!/bin/sh

SPEC=$1
specificjson --import $SPEC
if [ $? -eq 0 ]; then
  exit 1
fi
