import pandas as pd
import json
import glob

rows = []
columns = []
for f in glob.glob('*.json'):
    j = json.load(open(f))
    print(j)

    tmp_columns = []
    row = []
    row.append(f)
    row.append(j['time_unit'])
    tmp_columns.append('name')
    tmp_columns.append('time')
    for w in j['wrapper_infos']:
        row.append(w['id'])
        row.append(w['num_not_unwrap_move'])
        row.append(w['time_spawn'])
        row.append(w['last_wrap'])
        tmp_columns.append('id{}'.format(w['id']))
        tmp_columns.append('num_not_unwrap_move{}'.format(w['id']))
        tmp_columns.append('time_spawn{}'.format(w['id']))
        tmp_columns.append('last_wrap{}'.format(w['id']))
    rows.append(row)
    if len(tmp_columns) > len(columns):
        columns = tmp_columns

df = pd.DataFrame(rows, columns=columns)
df.to_csv('metadata.csv')