import pandas as pd
import json

with open('problems_info.json') as fi:
    data = json.load(fi)['0']
print(data)
df = pd.DataFrame(data, columns=['img', 'name', 'W', 'H', 'WxH', 'Area', 'Wall', 'X', 'B', 'L', 'F', 'R', 'C'])
df.to_csv('problems_info.csv')
