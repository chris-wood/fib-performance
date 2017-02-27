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

        # Name format based on experiment.rb
        # last _ is "out.txt"
        # previous one is "count_expnumber"
        alg = "-".join(splits[0:len(splits) - 3])
        debug("Processing %s" % alg)

        if alg not in inserts:
            inserts[alg] = []
            lookups[alg] = []
            insert_hashed[alg] = []
            lookup_hashed[alg] = []

        try:
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
            debug("Done with %s" % fname)
        except:
            debug("Error parsing: %s" % fname)

widths = [4, 8, 12, 16, 20, 24, 28, 32]
hash_algorithms = ["naive", "cisco"]
hash_alg_names = ["Naive", "Cisco"]

filters = [2, 3, 4, 5, 6]
filter_widths = [4, 8, 16, 32]
trie_depths = [2, 3, 4, 5, 6]

caesar_algorithms = []
caesar_alg_names = []
caesar_filter_algorithms = []
caesar_filter_alg_names = []
merged_filter_algorithms = []
merged_filter_alg_names = []
for w in filters:
    for m in filter_widths:
        caesar_algorithms.append("caesar-%d-%d" % (w, m))
        caesar_alg_names.append("Caesar[%d-%d]" % (w, m))
        caesar_filter_algorithms.append("caesar-filter-%d-%d" % (w, m))
        caesar_filter_alg_names.append("CaesarFilter[%d-%d]" % (w, m))
        merged_filter_algorithms.append("merged-filter-%d-%d" % (w, m))
        merged_filter_alg_names.append("MergedFilter[%d-%d]" % (w, m))

trie_algorithms = ["patricia"]
trie_alg_names = ["Patricia"]

hybrid_algorithms = []
hybrid_alg_names = []

for w in filters:
    for m in filter_widths:
        for T in trie_depths:
            hybrid_algorithms.append("tbf-%d-%d-%d" % (w, m, T))
            hybrid_alg_names.append("TBF[%d-%d-%d]" % (w, m, T))

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
        means[i] = ((float(mean) - hashed_mean) / hashed_mean) * 100.0

        hashed_stdev = lookup_hashed[alg][i][1]
        stdev = lookups[alg][0][1]
        stdevs[i] = ((float(stdev) - hashed_stdev) / hashed_stdev) * 100.0

    return means, stdevs

def compute_hashed_comparison_values(base_alg, hash_alg):
    global inserts
    global lookups
    global insert_hashed
    global lookup_hashed
    global widths

    means = [0.0] * (len(widths) + 2)
    stdevs = [0.0] * (len(widths) + 2)

    mean = lookups[base_alg][0][0]
    means[0] = mean
    stdev = lookups[base_alg][0][1]
    stdevs[0] = stdev

    mean = lookups[hash_alg][0][0]
    means[1] = mean
    stdev = lookups[hash_alg][0][1]
    stdevs[1] = stdev

    for i, width in enumerate(widths):
        hashed_mean = lookup_hashed[hash_alg][i][0]
        means[i + 2] = hashed_mean

        hashed_stdev = lookup_hashed[hash_alg][i][1]
        stdevs[i + 2] = hashed_stdev

    return means, stdevs

def compute_comparison_values(base_alg, alg):
    global inserts
    global lookups
    global insert_hashed
    global lookup_hashed
    global widths

    means = [0.0] * (len(widths) + 1)
    stdevs = [0.0] * (len(widths) + 1)

    mean = lookups[base_alg][0][0]
    means[0] = mean

    stdev = lookups[base_alg][0][1]
    stdevs[0] = stdev

    for i, width in enumerate(widths):
        mean = lookups[alg][i][0]
        means[i + 1] = mean

        stdev = lookups[alg][i][1]
        stdevs[i + 1] = stdev

    return means, stdevs

def plot_hash_improvement(fname, hash_algorithms, hash_alg_names):
    width = (1.0 / len(hash_algorithms)) * 0.9
    fig, ax = plt.subplots()
    rects = []
    colors = ["r", "b"] #, "g", "y"]
    indices = np.arange(len(widths))
    for i, alg in enumerate(hash_algorithms):
        values, errs = compute_values(alg)
        # rects_set = ax.bar(indices + (i * width), values, width, yerr=errs, color=colors[i % len(colors)], align="center") # log=True
        rects_set = ax.bar(indices + (i * width), values, width, color=colors[i % len(colors)], align="center", log=True) # log=True
        rects.append(rects_set)

    # add some text for labels, title and axes ticks
    ax.set_ylabel('Percentage Improvement [%]')
    ax.set_xlabel('Name Component Hash Size [B]')
    ax.set_xticks(indices + width)
    #ax.set_yscale('log')
    ax.set_xticklabels(tuple(widths))
    ax.legend((rects[0][0], rects[1][0]), (hash_alg_names[0], hash_alg_names[1]), loc=1)

    plt.grid(True)
    plt.savefig(fname)
    #plt.show()

def plot_filter_improvement(fname, algorithms, alg_names):
    width = (1.0 / len(algorithms)) * 0.9
    fig, ax = plt.subplots()
    rects = []
    colors = ["r", "b", "g", "y"]
    indices = np.arange(len(widths))
    for i, alg in enumerate(algorithms):
        values, errs = compute_values(alg)
        rects_set = ax.bar(indices + (i * width), values, width, color=colors[i % len(colors)], align="center", log=True) # log=True
        rects.append(rects_set)

    # add some text for labels, title and axes ticks
    ax.set_ylabel('Percentage Improvement [%]')
    ax.set_xlabel('Name Component Hash Size [B]')
    ax.set_xticks(indices + width)
    #ax.set_yscale('log')
    ax.set_xticklabels(tuple(widths))

    # This is bound to the number of filters we use...
    ax.legend((rects[0][0], rects[1][0], rects[2][0], rects[3][0], rects[4][0]), \
        (alg_names[0], alg_names[1], alg_names[2], alg_names[3], alg_names[4]), loc=1)

    plt.grid(True)
    plt.savefig(fname)
    #plt.show()

def plot_relative_hash_performance(fname, algA, nameA, algB, nameB):
    width = 0.9
    fig, ax = plt.subplots()
    rects = []
    colors = ["r", "b"]

    indices = np.arange(len(widths) + 2)

    values, errs = compute_hashed_comparison_values(algA, algB)
    rects_set = ax.bar(indices, values, width, align="center", color=colors[0 % len(colors)], log=True) # log=True
    rects.append(rects_set)

    # add some text for labels, title and axes ticks
    ax.set_ylabel('Mean Lookup Time [ns]')
    ax.set_xlabel('Algorithm')
    ax.set_xticks(indices)
    plt.xticks(rotation=45)
    plt.gcf().subplots_adjust(bottom=0.13)
    #ax.set_yscale('log')

    labels = [nameA, nameB] + map(lambda w : "%s[%d]" % (nameB, w), widths)
    debug(labels)
    ax.set_xticklabels(tuple(labels))

    plt.grid(True)
    plt.savefig(fname)
    #plt.show()

# Plot the improvements gained in hash-based FIBs using the hash-based name scheme
plot_hash_improvement("hash.pdf",hash_algorithms, hash_alg_names)


# XXX: need to trim the filter algorithms based on k -- there's too much going on!
plot_filter_improvement("caesar.pdf", caesar_algorithms, caesar_alg_names)
plot_filter_improvement("caesar_filter.pdf", caesar_filter_algorithms, caesar_filter_alg_names)
plot_filter_improvement("merged_filter.pdf", merged_filter_algorithms, merged_filter_alg_names)

# Compare to the trie algorithms
for i in range(len(trie_algorithms)):
    trie_alg = trie_algorithms[i]
    trie_name = trie_alg_names[i]

    def process_algorithm_set(algorithms, trie_alg, trie_name):
        for j in range(len(algorithms)):
            other_alg = algorithms[j]
            other_name = algorithms[j]
            plot_relative_hash_performance("%s_vs_%s.pdf" % (trie_alg, other_alg), trie_alg, trie_name, other_alg, other_name)

    process_algorithm_set(hash_algorithms, trie_alg, trie_name)
    process_algorithm_set(caesar_algorithms, trie_alg, trie_name)
    process_algorithm_set(caesar_filter_algorithms, trie_alg, trie_name)
    process_algorithm_set(merged_filter_algorithms, trie_alg, trie_name)

# Compare to the hybrid TBF algorithm
for i in range(len(hybrid_algorithms)):
    hybrid_alg = hybrid_algorithms[i]
    hybrid_name = hybrid_alg_names[i]

    def process_algorithm_set(algorithms, trie_alg, trie_name):
        for j in range(len(algorithms)):
            other_alg = algorithms[j]
            other_name = algorithms[j]
            plot_relative_hash_performance("%s_vs_%s.pdf" % (trie_alg, other_alg), trie_alg, trie_name, other_alg, other_name)

    process_algorithm_set(hash_algorithms, hybrid_alg, hybrid_name)
    process_algorithm_set(caesar_algorithms, hybrid_alg, hybrid_name)
    process_algorithm_set(caesar_filter_algorithms, hybrid_alg, hybrid_name)
    process_algorithm_set(merged_filter_algorithms, hybrid_alg, hybrid_name)
