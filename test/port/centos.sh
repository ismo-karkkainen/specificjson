#!/bin/sh
set -eu
sudo yum install -y -q cmake make clang ruby rake
git clone --branch master --depth 1 https://github.com/onqtam/doctest.git dtroot >/dev/null
mv dtroot/doctest .
git clone --branch master --depth 1 https://github.com/ismo-karkkainen/edicta.git >/dev/null
cd edicta
sudo rake install >/dev/null
cd ..
CXX=clang++ cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
