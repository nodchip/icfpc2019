import matplotlib.pyplot as plt
import numpy as np
import glob
import os
import sys
import json
import collections
from pathlib import Path
from bottle import route, run, static_file

SCRIPT_DIR = Path(__file__).parent
REPO_ROOT_DIR = Path(__file__).absolute().parent.parent.parent
assert (REPO_ROOT_DIR / 'dataset').is_dir()

def read_map(f):
    lines = []
    with open(f, 'r') as fi:
        for line in fi.readlines():
            line = line.strip('\n').upper()
            lines.append(line)
    W = len(line)
    H = len(lines)

    counts = collections.Counter()
    arr = np.zeros((H, W, 3), np.uint8)
    for y in range(H):
        for x in range(W):
            counts[lines[y][x]] += 1
            if   lines[y][x] == '.': arr[y, x, :] = (224, 224, 224)
            elif lines[y][x] == ' ': arr[y, x, :] = (224, 224, 128)
            elif lines[y][x] == '#': arr[y, x, :] = (16, 16, 16)
            elif lines[y][x] == 'X': arr[y, x, :] = (16, 128, 16)
            elif lines[y][x] == 'B': arr[y, x, :] = (16, 16, 128)
            elif lines[y][x] == 'L': arr[y, x, :] = (128, 16, 16)
            elif lines[y][x] == 'F': arr[y, x, :] = (128, 128, 16)
            elif lines[y][x] == 'R': arr[y, x, :] = (16, 128, 128)
            elif lines[y][x] == 'C': arr[y, x, :] = (128, 16, 128)
            elif lines[y][x] == '@': arr[y, x, :] = (255, 16, 16)
    return arr, counts

def main(patterns=[REPO_ROOT_DIR / 'dataset/problems/*.map']):
    metainfo = []
    for pattern in patterns:
        for f in glob.glob(str(pattern)):
            f = Path(f)
            print(f)
            m, counts = read_map(f)
            fig, ax = plt.subplots(1, 1, figsize=(10, 10), dpi=72)
            ax.set_yticks(np.arange(0.5, m.shape[0], 1), minor=True)
            ax.set_xticks(np.arange(0.5, m.shape[1], 1), minor=True)
            #ax.grid(which='minor', color='w', linestyle='-')
            ax.imshow(m, origin='lower')
            fig.tight_layout()
            fig.savefig(str(f.with_suffix('.png')))
            plt.close(fig)
            metainfo.append(
                [f.with_suffix('.png').name, f.name]
                + [m.shape[1], m.shape[0], m.shape[0] * m.shape[1]]
                + [counts['.'] + counts['#']] + [counts[k] for k in '#XBLFRC']
            )

    with open(SCRIPT_DIR / 'problems_info.json', 'w') as fo:
        json.dump({0: metainfo}, fo)

@route('/')
def index():
    return static_file('problems.html', root=SCRIPT_DIR)

@route('/problems_info.json')
def data_json():
    with open(SCRIPT_DIR / 'problems_info.json', 'r') as fi:
        return fi.read()

@route('/image/<name>')
def image_file(name):
    return static_file(name, root=str(REPO_ROOT_DIR / 'dataset/problems'))

def server():
    run(host='0.0.0.0', port=18888)

if __name__ == '__main__':
    if sys.argv[1] == 'png':
        if len(sys.argv) > 2:
            main(sys.argv[2:])
        else:
            main()
    else:
        server()