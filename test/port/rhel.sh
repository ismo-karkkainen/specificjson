#!/bin/sh
set -eu
sudo yum install -y -q cmake gcc-c++ ruby rake
git clone --branch master --depth 1 https://github.com/onqtam/doctest.git
git clone --branch master --depth 1 https://github.com/ismo-karkkainen/edicta.git
cd edicta
sudo rake install
cd ..
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
