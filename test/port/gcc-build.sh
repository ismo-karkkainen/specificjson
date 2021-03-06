#!/bin/sh

set -eu

clone () {
    git clone --branch master --depth 1 $1 >/dev/null
}

clone https://github.com/onqtam/doctest.git
mv doctest dtroot
mv dtroot/doctest .
rm -rf dtroot
clone https://github.com/ismo-karkkainen/edicta.git
cd edicta
sudo rake install >/dev/null
cd ..
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
