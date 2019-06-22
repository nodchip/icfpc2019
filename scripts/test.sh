#!/bin/sh
: ${NUMBER_OF_PROCESSORS:=72}
cd ${WORKSPACE}/src
./test --gtest_output=xml:test_result.xml || exit 1
