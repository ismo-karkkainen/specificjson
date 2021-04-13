#!/bin/sh

set -eu
sudo apt-get install -y -q cmake build-essential ruby rake >/dev/null
$1/test/port/gcc-build.sh $1
