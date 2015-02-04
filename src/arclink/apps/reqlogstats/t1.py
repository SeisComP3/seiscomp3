#!/usr/bin/env python
#
# Normalise a list of lines ending in GiB, MiB to MiB.
#
import sys
for x in sys.stdin.readlines():
    line = x.strip()
    if line.endswith('GiB'):
        words = line.split();
        val = words[-2];
        print '\t'.join(words[0:-2]), '\t', float(val) * 1024.0, 'MiB'
    else:
        print line
