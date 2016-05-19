#!/usr/bin/env python
#
# Rearrange a list to a table for histogram plotting, and sum by size.
#
import sys

dcid_list = ['BGR', 'ETHZ', 'GFZ', 'INGV', 'IPGP', 'KOERI', 'LMU', 'NIEP', 'NOA', 'ODC', 'RESIF']
curday = None
#row = dict.fromkeys(dcid_list, 0)

def flush_day(day, row):
    s = sum(float(row[x]) for x in row.keys())
    print "%s %10.1f" % (day, s),
    for dcid in dcid_list:
        ##if row.has_key(dcid):
        # Ugly: ideally would be None if there were no data
        if row[dcid] != 0:
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
        # If there are multiple reports from a dcid, sum them:
        #row[dcid] = val
        row[dcid] += float(val)
    else:
        # new day, flush and reload
        if (curday != None):
            flush_day(curday, row)
        curday = day
        row = dict.fromkeys(dcid_list, 0)
        row[dcid] = float(val)

if (curday != None):
    flush_day(curday, row)

