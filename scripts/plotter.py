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
alg_names = ["Naive", "Cisco"] #, "Caesar", "Caesar-Filter"]
widths = [4, 8, 12, 16, 20, 24, 28, 32]
width = (1.0 / len(algorithms)) * 0.9

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

        hashed_stdev = lookup_hashed[alg][i][1]
        stdev = lookups[alg][0][1]
        stdevs[i] = (float(stdev) - hashed_stdev) / hashed_stdev

    return means, stdevs

fig, ax = plt.subplots()
rects = []
colors = ["r", "b"] # , "g", "y"]
indices = np.arange(len(widths))
for i, alg in enumerate(algorithms):
    values, errs = compute_values(alg)
    rects_set = ax.bar(indices + (i * width), values, width, yerr=errs, color=colors[i], align="center") # log=True
    rects.append(rects_set)

# add some text for labels, title and axes ticks
ax.set_ylabel('Percentage Improvement [%]')
ax.set_xlabel('Name Component Hash Size [B]')
ax.set_xticks(indices + width)
#ax.set_yscale('log')
ax.set_xticklabels(tuple(widths))
ax.legend((rects[0][0], rects[1][0]), (alg_names[0], alg_names[1]), loc=1)

def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                '%d' % int(height),
                ha='center', va='bottom')

# autolabel(rects[0])
# autolabel(rects[1])

plt.grid(True)
#plt.savefig('sizes.eps')
plt.show()
