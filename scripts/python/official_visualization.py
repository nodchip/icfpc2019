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

REPO_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
CHROMEDRIVER_PATH = os.path.join(os.path.dirname(__file__), 'chromedriver.exe')

class Visualizer(object):
    OFFICIAL_CHECKER_URL = 'https://icfpcontest2019.github.io/solution_visualiser/'
    def __init__(self):
        self.driver = webdriver.Chrome(CHROMEDRIVER_PATH)
        self.driver.get(Visualizer.OFFICIAL_CHECKER_URL)
    
    def __del__(self):
        self.driver.quit()

    def visualize(self, desc_path, sol_path):
        desc_file = self.driver.find_element_by_id('submit_task')
        desc_file.send_keys(desc_path)

        sol_file = self.driver.find_element_by_id('submit_solution')
        sol_file.send_keys(sol_path)

        button = self.driver.find_element_by_id('execute_solution')
        button.click()

    def wait(self):
        while True:
            logs = self.driver.get_log('driver')
            for entry in logs:
                if 'message' in entry:
                    if entry['message'].startswith('Unable to evaluate script: disconnected'):
                        return
            time.sleep(0.1)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--desc-dir', default=os.path.join(REPO_DIR, 'dataset', 'problems'))
    parser.add_argument('sol_path')
    args = parser.parse_args()

    desc_path = os.path.join(args.desc_dir, os.path.splitext(os.path.basename(args.sol_path))[0] + '.desc')

    v = Visualizer()
    v.visualize(os.path.abspath(desc_path), os.path.abspath(args.sol_path))
    v.wait()
    del v

if __name__ == '__main__':
    main()