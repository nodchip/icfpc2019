#!/bin/sh
rm -Rf solutions/${ENGINE_NAME}
./scripts/python/execute_engines.py \
--description_directory_path dataset/problems \
--solution_directory_path solutions/${ENGINE_NAME} \
--buy_directory_path buy \
--best_solution_directory_path best_solutions \
--engine_file_path src/solver \
--jobs `grep processor /proc/cpuinfo | wc -l` \
--solver_name ${ENGINE_NAME} || exit 1

./scripts/python/compare_engine.py \
--solution_base_directory_path solutions \
--engine_name_file_paths engine_names.txt \
--output_html_file_path compare_engine.html || exit 1
