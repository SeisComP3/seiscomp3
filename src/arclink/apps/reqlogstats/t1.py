#!/usr/bin/env python
#
# Normalise a list of lines ending in GiB, MiB or bytes, to MiB.
#
import sys

def normalise(line):
    words = line.split()
    if line.endswith('GiB'):
        val = words[-2];
        print '\t'.join(words[0:-2]), '\t', float(val) * 1024.0, 'MiB'
    elif line.endswith('MiB'):
        print line
        # Not sure; do I need to adjust white space to tabs??
    elif line.endswith('KiB'):
        val = words[-2];
        print '\t'.join(words[0:-2]), '\t', float(val) / 1024.0, 'MiB'
    elif line.endswith('B'):
        val = words[-2]
        print '\t'.join(words[0:-2]), '\t', float(val) / 1024.0 / 1024.0, 'MiB'
    else:
        val = words[-1]
        words[-1] = str(float(val) / 1024.0 / 1024.0) + " MiB"
        print "\t".join(words)

def test1():
    normalise("a man had 5.0 GiB")
    normalise("he used only 400 MiB")
    normalise("the difference was 65536")
    normalise("the difference was 64 KiB")
    normalise("the difference was 65536 B")

#test1()

for x in sys.stdin.readlines():
    line = x.strip()
    normalise(line)
