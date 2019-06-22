#!/bin/sh
: ${NUMBER_OF_PROCESSORS:=72}
git submodule init
git submodule update
cd ${WORKSPACE}/src
make clean || exit 1
make -j ${NUMBER_OF_PROCESSORS} all || exit 1
