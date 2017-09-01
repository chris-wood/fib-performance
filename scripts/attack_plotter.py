import sys
import math
import numpy as np
import matplotlib.pyplot as plt

indices = [int(data.split(",")[0]) for data in open(sys.argv[1]).readlines()]
app_times = [float(data.split(",")[1]) for data in open(sys.argv[1]).readlines()]
net_times = [float(data.split(",")[1]) for data in open(sys.argv[2]).readlines()]

# width = 0.5
fig, ax = plt.subplots()
# rects = []
# colors = ["r", "b"]
# indices = np.arange(len(widths))
# for i, alg in enumerate(hash_algorithms):
#     values, errs = compute_values(alg)
#     # rects_set = ax.bar(indices + (i * width), values, width, yerr=errs, color=colors[i % len(colors)], align="center") # log=True
#     rects_set = ax.bar(indices + (i * width), values, width, color=colors[i % len(colors)], align="center", log=True) # log=True
#     rects.append(rects_set)

plt.plot(indices, app_times, color="red", label="Application Names")
plt.plot(indices, net_times, color="blue", label="Network Names")

print app_times[0:10]
print net_times[0:10]

# add some text for labels, title and axes ticks
ax.set_ylabel('Service Time [ns]')
ax.set_xlabel('Name Index')

# ax.set_xticks(indices + width)
#ax.set_yscale('log')
# ax.set_xticklabels(tuple(widths))
ax.legend(loc=2)
# ax.legend((rects[0][0], rects[1][0]), (hash_alg_names[0], hash_alg_names[1]), loc=1)

plt.grid(True)
# plt.savefig(fname)
plt.savefig(sys.argv[3])
