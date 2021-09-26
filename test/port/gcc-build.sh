#!/bin/sh

set -eu

echo 'Require edicta'
gem install edicta
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
