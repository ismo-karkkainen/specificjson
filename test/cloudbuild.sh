#!/bin/sh

set -u
export D=$1
R=$2

export C="gem install edicta"
$C

# Scripts from installed gems not found (in OpenSUSE Tumbleweed)?
for X in edicta
do
    $X --help >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        continue
    fi
    for P in /usr/lib64/ruby/gems
    do
        S=$(find $P -type f -name $X -perm -0555)
        if [ $? -eq 0 ]; then
            export PATH="$PATH:$(dirname $S)"
            echo "Found $X in $S"
            break
        fi
        echo "$X not found under $P"
    done
done

cd $R
for X in clang++ g++
do
    export X
    mkdir testbuild
    (
        echo "Built on $D using $X at $(date '+%Y-%m-%d %H:%M')"
        cat _logs/info.txt
        echo "$C"
        (
            set -eu
            cd testbuild
            CXX=$X cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
            make -j 2
            make test
        )
        echo "Build and test exit code: $?"
    ) 2>&1 | tee -a "$R/_logs/$D-$X.log"
    rm -rf testbuild
done
