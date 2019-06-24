for i in range(1, 301):
    num_clones = 0
    if 101 <= i and i <= 150:
        num_clones = 1
    elif 181 <= i and i <= 210:
        num_clones = 1
    elif 211 <= i and i <= 220:
        num_clones = 2
    with open('prob-{i:03}.buy'.format(i=i), 'w') as f:
        f.write('C' * num_clones)
