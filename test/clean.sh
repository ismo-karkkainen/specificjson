#!/bin/sh

specificjson --clean --rebuild clean
clean --clean --rebuild clean2
STATUS=$(diff -q clean clean2)
if [ -z "$KEEP" ]; then
    rm -f clean clean2
fi
exit $STATUS
