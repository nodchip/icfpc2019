#!/bin/bash
MAIN_PATH=$(dirname $0)/../src/solver
for f in $@; do
    echo "Processing $f"
    $MAIN_PATH convert $f > ${f%.desc}.map
done