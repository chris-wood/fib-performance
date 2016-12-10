import sys
import math
import numpy as np
import matplotlib.pyplot as plt

def format_e(n):
    a = '%E' % n
    return a.split('E')[0].rstrip('0').rstrip('.') + 'E' + a.split('E')[1]

hash_sizes = []
inserts = {}
lookups = {}
insert_hashed = {}
lookup_hashed = {}

def parse_files(files):
    global inserts
    global lookups
    global insert_hashed
    global lookup_hashed
    global hash_sizes

    for fname in files:
        splits = fname.split("_")
        alg = splits[0]

        if alg not in inserts:
            inserts[alg] = []
            lookups[alg] = []
            insert_hashed[alg] = []
            lookup_hashed[alg] = []

        with open(fname, "r") as fh:
            for line in fh:
                line = line.strip()
                splits = line.split(",")

                action = splits[0]
                d = int(splits[1])
                mean = float(splits[2])
                stdev = float(splits[3])
                fraction = float(splits[4])

                if d not in hash_sizes:
                    hash_sizes.append(d)

                if action == "lookup" and d == 0:
                    lookups[alg].append((mean, stdev, fraction))
                elif action == "lookup" and d != 0:
                    lookup_hashed[alg].append((mean, stdev, fraction))
                elif action == "insert" and d == 0:
                    inserts[alg].append((mean, stdev, fraction))
                elif action == "insert" and d != 0:
                    insert_hashed[alg].append((mean, stdev, fraction))

algorithms = ["naive", "cisco"] #, "caesar", "caesar-filter"]
widths = [4, 8, 12, 16, 20, 24, 28, 32]
width = 1.0 / len(algorithms)

parse_files(sys.argv[1:])

#print hash_sizes
#print inserts
#print lookups
#print insert_hashed
#print lookup_hashed

def compute_values(alg):
    global inserts
    global lookups
    global insert_hashed
    global lookup_hashed
    global widths

    means = [0.0] * len(widths)
    stdevs = [0.0] * len(widths)

    for i, width in enumerate(widths):
        hashed_mean = lookup_hashed[alg][i][0]
        mean = lookups[alg][0][0]
            means[i] = (float(mean) - hashed_mean) / hashed_mean

    return means

fig, ax = plt.subplots()
rects = []
colors = ["r", "b"] # , "g", "y"]
indices = np.arange(len(widths))
for i, alg in enumerate(algorithms):
    values = compute_values(alg)
    rects_set = ax.bar(indices + (i * width), values, width, color=colors[i]) # log=True 

# add some text for labels, title and axes ticks
ax.set_ylabel('Percentage Improvement')
ax.set_xlabel('Hash Size [B]')
#ax.set_xticks(indices + width)
#ax.set_yscale('log')
#ax.set_xticklabels(tuple(map(lambda s : "{:.1e}".format(s), sizes)))
#ax.legend((rects1[0], rects2[0]), ('SCR', 'IPBC'), loc=2)

def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                '%d' % int(height),
                ha='center', va='bottom')
#autolabel(rects1)
#autolabel(rects2)
plt.grid(True)
#plt.savefig('sizes.eps')
plt.show()