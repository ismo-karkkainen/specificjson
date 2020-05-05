#!/bin/sh

LIST=$1
let A=$(cat $LIST | wc -l | sed 's/ //g')-1
B=$(specificjson --list | wc -l | sed 's/ //g')
test $A -eq $B
