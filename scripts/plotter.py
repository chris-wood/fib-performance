import sys
import math
import numpy as np
import matplotlib.pyplot as plt

def debug(s):
    print >> sys.stderr, s

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

        # alg_fibsize_{out | filter}
        if "out" not in splits[2]:
            alg = splits[0] + "-" + splits[1] + "-" + splits[2]
        else:
            alg = splits[0] + "-" + splits[1]

        print "parsing %s" % alg

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

widths = [4, 8, 12, 16, 20, 24, 28, 32]
hash_algorithms = ["naive", "cisco"]
hash_alg_names = ["Naive", "Cisco"]

filters = [2, 3, 4, 5, 6]
caesar_algorithms = []
caesar_alg_names = []
caesar_filter_algorithms = []
caesar_filter_alg_names = []
merged_filter_algorithms = []
merged_filter_alg_names = []
for w in filters:
    caesar_algorithms.append("caesar-%d" % w)
    caesar_alg_names.append("Caesar[%d]" % w)
    caesar_filter_algorithms.append("caesar-filter-%d" % w)
    caesar_filter_alg_names.append("CaesarFilter[%d]" % w)
    merged_filter_algorithms.append("merged-filter-%d" % w)
    merged_filter_alg_names.append("MergedFilter[%d]" % w)

debug("parsing...")
parse_files(sys.argv[1:])
debug("plotting...")

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

def plot_hash_improvement(hash_algorithms, hash_alg_names):
    width = (1.0 / len(hash_algorithms)) * 0.9
    fig, ax = plt.subplots()
    rects = []
    colors = ["r", "b"] # , "g", "y"]
    indices = np.arange(len(widths))
    for i, alg in enumerate(hash_algorithms):
        values, errs = compute_values(alg)
        rects_set = ax.bar(indices + (i * width), values, width, yerr=errs, color=colors[i % len(colors)], align="center") # log=True
        rects.append(rects_set)

    # add some text for labels, title and axes ticks
    ax.set_ylabel('Percentage Improvement [%]')
    ax.set_xlabel('Name Component Hash Size [B]')
    ax.set_xticks(indices + width)
    #ax.set_yscale('log')
    ax.set_xticklabels(tuple(widths))
    ax.legend((rects[0][0], rects[1][0]), (hash_alg_names[0], hash_alg_names[1]), loc=1)

    plt.grid(True)
    #plt.savefig('sizes.eps')
    plt.show()

def plot_filter_improvement(algorithms, alg_names):
    width = (1.0 / len(algorithms)) * 0.9
    fig, ax = plt.subplots()
    rects = []
    colors = ["r", "b", "g", "y"]
    indices = np.arange(len(widths))
    for i, alg in enumerate(algorithms):
        print alg
        values, errs = compute_values(alg)
        rects_set = ax.bar(indices + (i * width), values, width, color=colors[i % len(colors)], align="center") # log=True
        rects.append(rects_set)

    # add some text for labels, title and axes ticks
    ax.set_ylabel('Percentage Improvement [%]')
    ax.set_xlabel('Name Component Hash Size [B]')
    ax.set_xticks(indices + width)
    #ax.set_yscale('log')
    ax.set_xticklabels(tuple(widths))
    ax.legend((rects[0][0], rects[1][0], rects[2][0], rects[3][0], rects[4][0]), \
        (alg_names[0], alg_names[1], alg_names[2], alg_names[3], alg_names[4]), loc=1)

    plt.grid(True)
    #plt.savefig('sizes.eps')
    plt.show()

plot_hash_improvement(hash_algorithms, hash_alg_names)
plot_filter_improvement(caesar_algorithms, caesar_alg_names)
plot_filter_improvement(caesar_filter_algorithms, caesar_filter_alg_names)
plot_filter_improvement(merged_filter_algorithms, merged_filter_alg_names)
