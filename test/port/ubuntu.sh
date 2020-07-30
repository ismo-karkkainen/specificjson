#!/bin/sh
set -eu
sudo apt-get install -y -q cmake build-essential ruby rake
git clone --branch master --depth 1 https://github.com/onqtam/doctest.git dtroot
mv dtroot/doctest .
git clone --branch master --depth 1 https://github.com/ismo-karkkainen/edicta.git
cd edicta
sudo rake install
cd ..
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
