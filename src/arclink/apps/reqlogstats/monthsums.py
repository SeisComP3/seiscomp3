#!/usr/bin/env python
#
# Summarise numeric values by month.
#
# Sample input: see test/monthsums1.in
# First column must be 'yyyy-mm-dd' dates.

# Expected output: monthly summaries as in test/monthsums1.out
# ...with no row if there's no data for that month.

import sys

row = 0
days = 0
today = ''
current = ''

sums = list()
for x in sys.stdin.readlines():
    line = x.strip()
    words = line.split()
    if len(words) == 0: continue   # Tolerate blank lines

    if line.startswith('#'):
        print line.replace('DAY ', 'DATE') + " DAYS"    # Pass on comments, is this right?
        continue

    today = words.pop(0)

    if (row == 0):
        cols = len(words)
        sums.extend(cols*[0])
        current = today
    row += 1

    if today[0:7] == current[0:7]:
        for k in range(len(words)):
            sums[k] += float(words[k])
        days +=1
    else:
        print current[0:7], ' '.join('%8s' % z for z in sums), days
        current = today
        sums = cols*[0]
        for k in range(len(words)):
            sums[k] += float(words[k])
        days = 1

# dump remaining sums:
print current[0:7], ' '.join('%8s' % z for z in sums), days
