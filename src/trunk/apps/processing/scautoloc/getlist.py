#!/usr/bin/env python

import sys
import seiscomp3.Client
import seiscomp3.DataModel


class InvApp(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(True, True)
        self.setLoggingToStdErr(True)

    def validateParameters(self):
        try:
            if seiscomp3.Client.Application.validateParameters(self) == False:
                return False

            return True

        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info:
                sys.stderr.write(i)
            sys.exit(-1)

    def run(self):
        now = seiscomp3.Core.Time.GMT()
        try:
            lines = []
            dbr = seiscomp3.DataModel.DatabaseReader(self.database())
            inv = seiscomp3.DataModel.Inventory()
            dbr.loadNetworks(inv)
            nnet = inv.networkCount()
            for inet in xrange(nnet):
                net = inv.network(inet)
                dbr.load(net)
                nsta = net.stationCount()
                for ista in xrange(nsta):
                    sta = net.station(ista)
                    line = "%-2s %-5s %9.4f %9.4f %6.1f" % (
                        net.code(), sta.code(), sta.latitude(), sta.longitude(), sta.elevation())
#                   print dir(sta)
                    try:
                        start = sta.start()
                    except:
                        continue

                    try:
                        end = sta.end()
                        if not start <= now <= end:
                            continue
                    except:
                        pass

                    lines.append(line)

            lines.sort()
            for line in lines:
                print line

            return True
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info:
                sys.stderr.write(i)
            sys.exit(-1)


def main():
    app = InvApp(len(sys.argv), sys.argv)
    app()


if __name__ == "__main__":
    main()
