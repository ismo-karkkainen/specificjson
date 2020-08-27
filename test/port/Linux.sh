#!/bin/sh

F="/etc/os-release"

if [ ! -f $F ]; then
    echo "No $F"
    exit 1
fi

D=$(grep '^ID=' $F | sed 's/"/ /g' | sed 's/=/ /g' | sed 's/  / /g' | cut -d ' ' -f 2).sh

if [ ! -x $1/test/port/$D ]; then
    echo "Not supported, ID-line does not match any script:"
    cat /etc/os-release
    exit 1
fi
echo $D
$1/test/port/$D "$1"
