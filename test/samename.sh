#!/bin/sh

IMPORT=$1

specificjson --clean --import $IMPORT --warn --build samenamejson
mkdir a
cd a
../samenamejson --export --warn
RC=0
for F in License.txt License_1.txt
do
    if [ ! -f $F ]; then
        echo "Missing $F"
        RC=1
    fi
done
cd ..
rm -rf samenamejson a
exit $RC
