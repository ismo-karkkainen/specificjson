#!/bin/sh

specificjson --clean --build clean
clean --clean --build clean2
STATUS=$(diff -q clean clean2)
if [ "$(clean --list)" != "" ]; then
  echo "Not empty list."
  STATUS=1
fi
if [ -z "$KEEP" ]; then
    rm -f clean clean2
fi
exit $STATUS
