#!/bin/sh

echo "Amazon Linux 2 cmake is too old, not building."
exit

sudo yum install cmake clang -y
sudo amazon-linux-extras install ruby2.6 -y
git clone --branch master --depth 1 https://github.com/onqtam/doctest.git
git clone --branch master --depth 1 https://github.com/ismo-karkkainen/edicta.git
cd edicta
sudo rake install
cd ..
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG $1
make -j 3
make test
