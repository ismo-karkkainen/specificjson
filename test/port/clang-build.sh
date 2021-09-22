#!/bin/sh

set -eu

gem install edicta
CXX=clang++ cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
