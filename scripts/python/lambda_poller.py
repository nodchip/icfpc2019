#!/usr/bin/env python3
import argparse
import concurrent.futures
import collections
import os
import re
import shutil
import subprocess
import multiprocessing
import time
import sys
import json
import execute_engines


PuzzleResult = collections.namedtuple('EngineResult', ('engine_name', 'time'))
EngineResult = collections.namedtuple('EngineResult', ('engine_name', 'time'))


TIMEOUT = 400.0
INFINITE = 10**9
SLEEP_TIME = 60.0
PUBLIC_ID = '164'
NEXT_BLOCK_FILE = os.path.abspath('next_block_file.txt')
PUZZLE_INPUT_FILE_NAME = os.path.abspath('puzzle.input.txt')
PUZZLE_OUTPUT_FILE_NAME = os.path.abspath('puzzle.output.desc')
TASK_INPUT_FILE_NAME = os.path.abspath('task.input.txt')
TASK_OUTPUT_FILE_NAME_FORMAT = os.path.abspath('task.output.{engine_name}.sol')


def solve_puzzle(args):
    if os.path.isfile(PUZZLE_OUTPUT_FILE_NAME):
        os.remove(PUZZLE_OUTPUT_FILE_NAME)

    command = [args.puzzle_solver_file_path, 'puzzle_run', 'outBFS', '--cond', PUZZLE_INPUT_FILE_NAME,
               '--output', PUZZLE_OUTPUT_FILE_NAME]
    print(command, flush=True)
    try:
        completed_process = subprocess.run(command, stdout=subprocess.PIPE,
                                           stderr=subprocess.PIPE, timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        print('Puzzle solver timed out...', flush=True)
        return False

    if completed_process.returncode:
        print('Puzzle solver failed...', flush=True)
        return False

    if not os.path.isfile(PUZZLE_OUTPUT_FILE_NAME):
        print('Puzzle solver output file does not exist...', flush=True)
        return False

    return True


def execute(engine_name, args):
    task_output_file_name = TASK_OUTPUT_FILE_NAME_FORMAT.format(engine_name=engine_name)
    if os.path.isfile(task_output_file_name):
        os.remove(task_output_file_name)

    command = [args.engine_file_path, 'run', engine_name, '--desc', TASK_INPUT_FILE_NAME,
               '--output', task_output_file_name]
    print(command, flush=True)
    t0 = time.time()
    try:
        completed_process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                           timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        t1 = time.time()
        print('Task solver timed out... engine_name={engine_name} elapsed={elapsed_s}s'.format(
            engine_name=engine_name, elapsed_s=t1-t0), flush=True)
        return False, t1 - t0
    t1 = time.time()

    if completed_process.returncode:
        print('Task solver failed... engine_name={engine_name} elapsed={elapsed_s}s'.format(engine_name=engine_name, elapsed_s=t1-t0),
              flush=True)
        return False, t1 - t0

    if not os.path.isfile(task_output_file_name):
        print('Output file does not exist... engine_name={engine_name} elapsed={elapsed_s}s'.format(
            engine_name=engine_name, elapsed_s=t1-t0), flush=True)
        return False, t1 - t0

    return True, t1 - t0


def get_next_block():
    if not os.path.isfile(NEXT_BLOCK_FILE):
        return 0
    with open(NEXT_BLOCK_FILE, 'r') as f:
        return int(f.readline())


def set_next_block(block):
    with open(NEXT_BLOCK_FILE, 'w') as f:
        f.write(str(block))


def main():
    parser = argparse.ArgumentParser(description='Polls lambda.')
    parser.add_argument('--engine_name_file_paths',
                        help='File path containing engine name. One engine name per one line.',
                        default='mining_engine_names.txt')
    parser.add_argument('--puzzle_solver_file_path', help='File path of the puzzle solver.',
                        default='src/solver')
    parser.add_argument('--jobs', type=int, help='Number of jobs,',
                        default=multiprocessing.cpu_count())
    parser.add_argument('--engine_file_path', help='File path of the engine.',
                        default='src/solver')
    args = parser.parse_args()

    # HACK: Process the latest block on startup always.
    set_next_block(-1)

    engine_names = list()
    with open(args.engine_name_file_paths, 'r') as f:
        for engine_name in f:
            if engine_name.startswith('#'):
                print('Skip engine: {}'.format(engine_name))
            else:
                print('Use engine: {}'.format(engine_name))
                engine_names.append(engine_name.strip())

    python_path = 'python3'
    python_path_candidates = [
        r'C:\Users\nodchip\AppData\Local\Programs\Python\Python37\python.exe',
        r'C:\Users\nodchip\AppData\Local\Programs\Python\Python36\python.exe',
        r'C:\Program Files (x86)\Microsoft Visual Studio\Shared\Python37_64\python.exe',
        ]
    for python_path_candidate in python_path_candidates:
        if os.path.isfile(python_path_candidate):
            python_path = python_path_candidate
            break

    while True:
        print('Polling...', flush=True)

        command = [python_path, 'lambda-cli.py', 'getmininginfo']
        print(command, flush=True)
        completed_process = subprocess.run(command, stdout=subprocess.PIPE,
                                           cwd='dataset/lambda-client', universal_newlines=True)
        if completed_process.returncode:
            print(completed_process, flush=True)
            sys.exit('Failed to execute lambda-cli.py getmininginfo')

        stdout = completed_process.stdout
        stdout = stdout.replace("'",'"')
        mininginfo = json.loads(stdout)

        next_block = get_next_block()
        if next_block > mininginfo['block']:
            print('Skipped because the current block {} (< next {}) is old...'.format(mininginfo['block'], next_block), flush=True)
            time.sleep(SLEEP_TIME)
            continue

        if PUBLIC_ID in mininginfo['excluded']:
            print('Skipped because out team is execluded...', flush=True)
            time.sleep(SLEEP_TIME)
            continue
        t_block_processing_0 = time.time()

        with open(PUZZLE_INPUT_FILE_NAME, 'w') as f:
            f.write(mininginfo['puzzle'])

        with open(TASK_INPUT_FILE_NAME, 'w') as f:
            f.write(mininginfo['task'])

        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
            puzzle_solver_future = executor.submit(solve_puzzle, args)
            task_solver_futures = list()
            for engine_name in engine_names:
                task_output_file_name = TASK_OUTPUT_FILE_NAME_FORMAT.format(engine_name=engine_name)
                if os.path.isfile(task_output_file_name):
                    print('remove old task output file: {}'.format(task_output_file_name))
                    os.unlink(task_output_file_name)
                task_solver_future = executor.submit(execute, engine_name, args)
                task_solver_futures.append((engine_name, task_solver_future))

        if not puzzle_solver_future.result():
            sys.exit('Some thing faild...  Read above...')
        engine_elapsed_s = collections.defaultdict(lambda: -1)
        for engine_name, task_solver_future in task_solver_futures:
            succeeded, elapsed_s = task_solver_future.result()
            engine_elapsed_s[engine_name] = elapsed_s
            if not succeeded:
                #sys.exit('Some thing faild...  Read above...')
                print("some engine failed but we'd like to continue with poor results..")

        best_time = INFINITE
        best_engine = None
        for engine_name in engine_names:
            task_output_file_name = TASK_OUTPUT_FILE_NAME_FORMAT.format(engine_name=engine_name)
            if os.path.isfile(task_output_file_name):
                engine_time = execute_engines.calculate_time(task_output_file_name)
                print('engine_name={engine_name:>10} time={time:>10} elapsed_time={elapsed_s}s'.format(
                    engine_name=engine_name, time=engine_time, elapsed_s=engine_elapsed_s[engine_name]), flush=True)
                if best_time <= engine_time:
                    continue
                best_time = engine_time
                best_engine = engine_name
        
        if best_engine is None:
            sys.exit('no engines returned a valid result.')
        print('choose best engine [{}] time={}'.format(best_engine, best_time))

        task_output_file_name = TASK_OUTPUT_FILE_NAME_FORMAT.format(engine_name=best_engine)

        command = [python_path, 'lambda-cli.py', 'submit', str(mininginfo['block']),
                   task_output_file_name, PUZZLE_OUTPUT_FILE_NAME]
        print(command, flush=True)
        completed_process = subprocess.run(command, cwd='dataset/lambda-client',
                                           universal_newlines=True)
        t_block_processing_1 = time.time()

        if completed_process.returncode:
            sys.exit('Failed to execute lambda-cli.py submit. took {}s'.format(t_block_processing_1 - t_block_processing_1))

        print('Submission succeeded... took {}s'.format(t_block_processing_1 - t_block_processing_1), flush=True)

        set_next_block(mininginfo['block'] + 1)

        time.sleep(SLEEP_TIME)


if __name__ == "__main__":
    main()
