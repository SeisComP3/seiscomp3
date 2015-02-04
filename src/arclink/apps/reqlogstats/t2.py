#!/usr/bin/env python
#
# Rearrange a list to a table for histogram plotting, and sum by size.
#
import sys

dcid_list = ['BGR', 'ETHZ', 'GFZ', 'INGV', 'IPGP', 'LMU', 'NIEP', 'ODC', 'RESIF']
curday = None
row = {}

def flush_day(day, row):
    s = sum(float(row[x]) for x in row.keys())
    print "%s %10.1f" % (day, s),
    for dcid in dcid_list:
        if row.has_key(dcid):
            print "%8.1f" % (float(row[dcid])),
        else:
            print "%8d" % 0,
    print

print "# DAY    ", "      TOTAL", " ".join("%8s" % x for x in dcid_list)

for x in sys.stdin.readlines():
    line = x.strip()
    words = line.split()
    day = words[0]
    dcid = words[1]
    val = words[2]

    if day == curday:
        row[dcid] = val
    else:
        # new day, flush and reload
        if (curday != None):
            flush_day(curday, row)
        curday = day
        row = {}
        row[dcid] = val
if (curday != None):
    flush_day(curday, row)

