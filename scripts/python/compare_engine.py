#!/usr/bin/env python3
import argparse
import os

import execute_engines


INVALID_ENERGY = 1e100
RANKING_COLORS = [
    'aqua',
    'lime',
    'yellow',
    ]


def main():
    parser = argparse.ArgumentParser(description='Compare engine results and generate a html file.')
    parser.add_argument('--solution_base_directory_path', required=True,
                        help='Base directory path of solutions.')
    parser.add_argument('--engine_name_file_paths', required=True,
                        help='File path containing engine name. One engine name per one line.')
    parser.add_argument('--output_html_file_path', required=True,
                        help='File path of the output html file.')
    args = parser.parse_args()

    engines = list()
    with open(args.engine_name_file_paths, 'r') as f:
        for engine in f:
            engines.append(engine.strip())

    with open(args.output_html_file_path, 'wt') as f:
        print('''<html>
<head>
<title>Engine comparison result</title>
</head>
<body>
<table border="1" cellspacing="0" cellpadding="0">
<tr>
<th>problem name</th>
''', file=f)
        for engine in engines:
            print('<th>{engine}</th>'.format(engine=engine), file=f)
        print('</tr>', file=f)

        problem_names = set()
        for engine in engines:
            solution_directory_path = os.path.join(args.solution_base_directory_path, engine)
            if not os.path.isdir(solution_directory_path):
                continue
            problem_names |= {os.path.splitext(file_name)[0] for file_name
                              in os.listdir(solution_directory_path)
                              if os.path.splitext(file_name)[1] == '.sol'}
        problem_names = sorted(problem_names)

        for problem_name in problem_names:
            times = list()
            for engine in engines:
                solution_file_path = os.path.join(
                    args.solution_base_directory_path, engine, problem_name + '.sol')
                time = execute_engines.calculate_time(solution_file_path)
                times.append(time)

            time_to_rank = {v: k for k, v in reversed(list(enumerate(sorted(times))))}
            
            print('<tr align="right">', file=f)
            print('<td>{problem_name}</td>'.format(problem_name=problem_name), file=f)
            for engine_index in range(len(engines)):
                time = times[engine_index]
                rank = time_to_rank[times[engine_index]]
                if rank < len(RANKING_COLORS):
                    color = RANKING_COLORS[rank]
                else:
                    color = 'transparent'

                print('<td bgcolor="{color}">{time}</td>'.format(color=color, time=time),
                      file=f)
            print('</tr>', file=f)
        print('''</table>
</body>
</html>''', file=f)


if __name__ == '__main__':
	main()
