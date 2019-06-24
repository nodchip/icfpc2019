# 
# get ChromeDriver from
# https://sites.google.com/a/chromium.org/chromedriver/home
# and place it into the same directory of the script.

import os
import argparse
import glob
import time
import re
import json
import tqdm

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
#from selenium.webdriver.common.by import By
#from selenium.webdriver.support.ui import WebDriverWait
#from selenium.webdriver.support import expected_conditions as EC

CHROMEDRIVER_PATH = os.path.join(os.path.dirname(__file__), 'chromedriver.exe')

class Validator(object):
    OFFICIAL_CHECKER_URL = 'https://icfpcontest2019.github.io/solution_checker/'
    def __init__(self):
        options = Options()
        options.headless = True

        self.driver = webdriver.Chrome(CHROMEDRIVER_PATH, chrome_options=options)
        self.driver.get(Validator.OFFICIAL_CHECKER_URL)
    
    def __del__(self):
        self.driver.quit()

    def validate(self, desc_path, sol_path, buy_path=None):

        desc_file = self.driver.find_element_by_id('submit_task')
        desc_file.send_keys(os.path.abspath(desc_path))

        sol_file = self.driver.find_element_by_id('submit_solution')
        sol_file.send_keys(os.path.abspath(sol_path))

        if buy_path is not None and len(open(buy_path).read()) > 0:
            print('use buy file')
            buy_file = self.driver.find_element_by_id('submit_boosters')
            buy_file.send_keys(os.path.abspath(buy_path))

        button = self.driver.find_element_by_id('execute_solution')
        button.click()

        output = self.driver.find_element_by_id('output')
        while True:
            # stupid wait..
            t = output.text.strip()
            if t.startswith('Pre-processing') or t.startswith('Done'):
                time.sleep(0.1)
            else:
                break
        #print(t)
        
        mo = re.match(r'Success! Your solution took (\d+) time units.', t)
        if mo is not None:
            return {
                'succeeded': t.startswith('Success!'),
                'time': int(mo.groups()[0]),
                'message': t,
            }
        else:
            return {
                'succeeded': False,
                'message': t,
            }

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--desc-dir')
    parser.add_argument('--sol-dir')
    parser.add_argument('--buy-dir')
    args = parser.parse_args()

    pairs = []
    for desc_path in glob.glob(os.path.join(args.desc_dir, '*.desc')):
        sol_path = os.path.join(args.sol_dir, os.path.splitext(os.path.basename(desc_path))[0] + '.sol')
        buy_path = os.path.join(args.buy_dir, os.path.splitext(os.path.basename(desc_path))[0] + '.buy')
        pairs.append((desc_path, sol_path, buy_path))

    v = Validator()
    results = []
    for desc_path, sol_path, buy_path in tqdm.tqdm(pairs):
        if not os.path.isfile(sol_path):
            res = {
                'succeeded': False,
                'message': 'file not exist',
            }
        else:
            res = v.validate(desc_path, sol_path, buy_path)
        
        problem_index = -1
        mo = re.match(r'prob-(\d+)\.desc', os.path.basename(desc_path))
        if mo:
            problem_index = int(mo.groups()[0])
        results.append({
            'problem_index': problem_index,
            'desc_path': desc_path,
            'sol_path': sol_path,
            'result': res,
        })
        print(results[-1])
    
    del v
    print(json.dumps(results, indent=4))

if __name__ == '__main__':
    main()