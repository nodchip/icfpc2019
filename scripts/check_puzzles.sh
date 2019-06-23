#!/bin/bash
# ./scripts/check_puzzles.sh lambda-client/blocks/*/puzzle.cond
for f in $@; do
  echo "input: $f"
  if [ -f ]; then
    ./src/solver puzzle_run ${ENGINE} --cond $f --validate --output dummy_file.desc 1>/dev/null 2>&1 && echo "OK" || echo "fail"
  else
    echo "no file"
  fi
done

