#!/bin/sh
: ${NUMBER_OF_PROCESSORS:=72}
git submodule init
git submodule update
cd ${WORKSPACE}/src
make -B -j ${NUMBER_OF_PROCESSORS} all || exit 1
