#!/usr/bin/env python3
import argparse
import concurrent.futures
import collections
import os
import re
import shutil
import subprocess
import multiprocessing


Result = collections.namedtuple('Result', ('problem_name', 'new_time', 'best_time'))


TIMEOUT = 300.0
INFINITE = 10**9

def calculate_time(solution_file_path):
    if not os.path.isfile(solution_file_path):
        return INFINITE

    with open(solution_file_path, 'r') as f:
        body = f.read()

    # life_times[i] = life time of the i-th wrappy.
    life_times = [0]
    wrappy_index = 0
    for ch in body:
        if not ch.isupper():
            if ch == '#':
                wrappy_index += 1;
            continue

        life_times[wrappy_index] += 1
        
        if ch == 'C':
            life_times.append[life_times[wrappy_index]]

    assert wrappy_index + 1 == len(life_times), 'Expected number of wrappies={num_wrappies} Actual number of actions={num_actions}'.format(
        num_wrapies=len(life_times), num_actions=wrappy_index + 1)

    return max(life_times)


def execute(problem_name, args):
    description_file_path = os.path.join(args.description_directory_path, problem_name + '.desc')
    solution_file_path = os.path.join(args.solution_directory_path, problem_name + '.sol')
    best_solution_file_path = os.path.join(args.best_solution_directory_path, problem_name + '.sol')

    assert os.path.isfile(description_file_path), 'Description file does not exist. description_file_path={description_file_path}'.format(description_file_path=description_file_path)

    command = [args.engine_file_path, 'run', args.solver_name, '--desc', description_file_path,
               '--output', solution_file_path]
    print(command, flush=True)
    try:
        completed_process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        return Result(problem_name, INFINITE, INFINITE)

    assert completed_process.returncode == 0, 'Failed to execute the engine. command={command} description_file_path={description_file_path} solution_file_path={solution_file_path}'.format(
            command=command, description_file_path=description_file_path,
            solution_file_path=solution_file_path)

    assert os.path.isfile(solution_file_path), 'The engine did not output solutionfile. command={command} description_file_path={description_file_path} solution_file_path={solution_file_path}'.format(
            command=command, description_file_path=description_file_path,
            solution_file_path=solution_file_path)

    new_time = calculate_time(solution_file_path)
    best_time = calculate_time(best_solution_file_path)
    if new_time < best_time:
        if os.path.isfile(best_solution_file_path):
            os.remove(best_solution_file_path)
        shutil.copyfile(solution_file_path, best_solution_file_path)

    return Result(problem_name, new_time, best_time)


def main():
    parser = argparse.ArgumentParser(description='Executes engines.')
    parser.add_argument('--description_directory_path',
                        help='Directory path containing input task descriptions.',
                        default='dataset/problems')
    parser.add_argument('--solution_directory_path',
                        help='Parent directory path containing output solutions.',
                        required=True)
    parser.add_argument('--best_solution_directory_path',
                        help='Directory path containing best output solutions.',
                        default='best_solutions')
    parser.add_argument('--engine_file_path', help='File path of the engine.',
                        default='src/solver')
    parser.add_argument('--jobs', type=int, help='Number of jobs,',
                        default=multiprocessing.cpu_count())
    parser.add_argument('--solver_name', help='Solver name.', required=True)
    args = parser.parse_args()

    os.makedirs(args.description_directory_path, exist_ok=True)
    os.makedirs(args.solution_directory_path, exist_ok=True)
    os.makedirs(args.best_solution_directory_path, exist_ok=True)

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
        futures = list()
        for file_name in os.listdir(args.description_directory_path):
            split = os.path.splitext(file_name)
            problem_name = split[0]
            extension = split[1]
            if extension != '.desc':
                continue
            #execute(problem_name, args)
            future = executor.submit(execute, problem_name, args)
            futures.append(future)

    results = [future.result() for future in futures if future.result()]
    results.sort(key=lambda x:x.problem_name)
    for result in results:
        if result.new_time < result.best_time:
            updated = 'Updated!'
        else:
            updated = ''
        print('{problem_name:>10} {new_time:>10} {best_time:>10} {updated}'.format(
            problem_name=result.problem_name, new_time=result.new_time,
            best_time=result.best_time, updated=updated), flush=True)
    


if __name__ == "__main__":
    main()
