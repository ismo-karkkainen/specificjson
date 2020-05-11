#!/bin/sh

mkdir a
cd a
specificjson --export
specificjson --clean --import exported.yaml --build ../b
rm -f *
../b --export
../b --clean --import exported.yaml --build ../c
cd ..
diff -q b c
RC=$?
rm -rf a b c
exit $RC
