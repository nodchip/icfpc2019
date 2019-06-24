
import os
import argparse
import glob
import time
import re
import json
import pandas as pd
import numpy as np

REPO_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

def main():
    df = pd.read_excel(os.path.join(REPO_DIR, 'best_buy', 'buy.xlsx'))
    df = df.fillna(0)
    df.buy_B = df.buy_B.astype(np.int32)
    df.buy_L = df.buy_L.astype(np.int32)
    df.buy_F = df.buy_F.astype(np.int32)
    df.buy_R = df.buy_R.astype(np.int32)
    df.buy_C = df.buy_C.astype(np.int32)
    with pd.option_context('display.max_rows', 300, 'display.width', 1000):
        print(df)
    
    for i, row in df.iterrows():
        buy_str = 'B' * row.buy_B + \
                  'L' * row.buy_L + \
                  'F' * row.buy_F + \
                  'R' * row.buy_R + \
                  'C' * row.buy_C
        file_path = os.path.join(REPO_DIR, 'best_buy', 'prob-{:03d}.buy'.format(row.problem))
        if len(buy_str) > 0:
            print('Write {:03d}: {}'.format(row.problem, buy_str))
            with open(file_path, 'w') as fo:
                fo.write(buy_str)
        elif os.path.isfile(file_path):
            print('Delete {:03d}'.format(row.problem))
            os.unlink(file_path)
        else:
            print('Skip {:03d}'.format(row.problem))
    

if __name__ == '__main__':
    main()
