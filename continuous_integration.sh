#!/bin/sh
: ${NUMBER_OF_PROCESSORS:=72}
git submodule init
git submodule update
cd ${WORKSPACE}/src
make -j ${NUMBER_OF_PROCESSORS} all || exit 1
# ./test test.exe --gtest_output=xml:test_result.xml || exit 1
