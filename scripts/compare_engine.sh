#!/bin/sh
./scripts/python/compare_engine.py \
--solution_base_directory_path ${EXECUTE_ENGINES_WORKSPACE_PATH}/solutions \
--engine_name_file_paths engine_names.txt \
--output_html_file_path compare_engine.html || exit 1
mv compare_engine.html ${EXECUTE_ENGINES_WORKSPACE_PATH}/compare_engine.html || exit 1
