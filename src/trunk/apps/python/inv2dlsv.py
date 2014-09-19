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
import StringIO
from seiscomp.fseed import *
from seiscomp.db.seiscomp3 import sc3wrap
from seiscomp.db.seiscomp3.inventory import Inventory
from seiscomp3 import DataModel, IO

ORGANIZATION = "EIDA"

def main():
    if len(sys.argv) < 1 or len(sys.argv) > 3:
        print "Usage inv2dlsv [in_xml [out_dataless]]"
        return 1

    if len(sys.argv) > 1:
        inFile = sys.argv[1]
    else:
        inFile = "-"

    if len(sys.argv) > 2:
        out = sys.argv[2]
    else:
        out = ""

    sc3wrap.dbQuery = None

    ar = IO.XMLArchive()
    if ar.open(inFile) == False:
        raise IOError, inFile + ": unable to open"

    obj = ar.readObject()
    if obj is None:
        raise TypeError, inFile + ": invalid format"

    sc3inv = DataModel.Inventory.Cast(obj)
    if sc3inv is None:
        raise TypeError, inFile + ": invalid format"

    inv = Inventory(sc3inv)
    inv.load_stations("*", "*", "*", "*")
    inv.load_instruments()

    vol = SEEDVolume(inv, ORGANIZATION, "", resp_dict=False)

    for net in sum([i.values() for i in inv.network.itervalues()], []):
        for sta in sum([i.values() for i in net.station.itervalues()], []):
            for loc in sum([i.values() for i in sta.sensorLocation.itervalues()], []):
                for strm in sum([i.values() for i in loc.stream.itervalues()], []):
                    try:
                        vol.add_chan(net.code, sta.code, loc.code, strm.code, strm.start, strm.end)

                    except SEEDError, e:
                        print >> sys.stderr, "Error (%s,%s,%s,%s):" % (net.code, sta.code, loc.code, strm.code), str(e)

    if not out or out == "-":
        output = StringIO.StringIO()
        vol.output(output)
        sys.stdout.write(output.getvalue())
        sys.stdout.flush()
        output.close()
    else:
        vol.output(sys.argv[2])

    return 0


if __name__ == "__main__":
    try: sys.exit(main())
    except Exception, e:
        print >> sys.stderr, "ERROR: %s" % str(e)
        sys.exit(1)
