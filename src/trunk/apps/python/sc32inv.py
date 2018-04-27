#!/usr/bin/env python

############################################################################
#    Copyright (C) by GFZ Potsdam                                          #
#                                                                          #
#    You can redistribute and/or modify this program under the             #
#    terms of the SeisComP Public License.                                 #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    SeisComP Public License for more details.                             #
############################################################################

import sys
import getopt
import seiscomp3.IO
import seiscomp3.DataModel


usage = """sc32inv [options] input output=stdout

Options:
  -h [ --help ]  Produce help message
  -f             Enable formatted XML output
"""

def main(argv):
    formatted = False

    # parse command line options
    try:
        opts, args = getopt.getopt(argv[1:], "hf", ["help"])
    except getopt.error, msg:
        sys.stderr.write("%s\n" % msg)
        sys.stderr.write("for help use --help\n")
        return 1

    for o, a in opts:
        if o in ["-h", "--help"]:
            sys.stderr.write("%s\n" % usage)
            return 1
        elif o in ["-f"]:
            formatted = True

    argv = args
    if len(argv) < 1:
        sys.stderr.write("Missing input file\n")
        return 1

    ar = seiscomp3.IO.XMLArchive()
    if not ar.open(argv[0]):
        sys.stderr.write("Unable to parse input file: %s\n" % argv[0])
        return 2

    obj = ar.readObject()
    ar.close()

    if obj is None:
        sys.stderr.write("Empty document in %s\n" % argv[0])
        return 3

    inv = seiscomp3.DataModel.Inventory.Cast(obj)
    if inv is None:
        sys.stderr.write("No inventory found in %s\n" % argv[0])
        return 4

    if len(argv) < 2:
        output_file = "-" 
    else:
        output_file = argv[1]

    ar.create(output_file)
    ar.setFormattedOutput(formatted)
    ar.writeObject(inv)
    ar.close()

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

