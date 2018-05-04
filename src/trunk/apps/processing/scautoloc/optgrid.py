#!/usr/bin/env python

import sys
import seiscomp3.Client
import seiscomp3.DataModel
import seis.geo

nearestStations = 300


class _Station:
    pass


class _GridPoint:
    pass


def readGrid(gridfile):
    gridfile = file(gridfile)
    grid = []
    for line in gridfile:
        line = line.strip()
        if line.startswith("#"):
            continue
        line = line.split()

        p = _GridPoint()
        p.lat, p.lon, p.dep, p.rad, p.dist = tuple(map(float, line[:5]))
        p.nmin = int(line[5])

#       print p.lat,p.lon,p.dep,p.rad,p.dist,p.nmin
        grid.append(p)
    return grid


def writeGrid(grid, gridfile):
    gridfile = file(gridfile, "w")
    for p in grid:
        dist = int(p.dist+1)
        if dist > 180:
            dist = 180
        gridfile.write("%6.2f %6.2f %5.1f %5.2f %5.1f %d\n" %
                       (p.lat, p.lon, p.dep, p.rad, dist, p.nmin))
    gridfile.close()


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

        grid = readGrid("config/grid.conf")

        now = seiscomp3.Core.Time.GMT()
        try:
            _stations = []
            dbr = seiscomp3.DataModel.DatabaseReader(self.database())
            inv = seiscomp3.DataModel.Inventory()
            dbr.loadNetworks(inv)
            nnet = inv.networkCount()
            for inet in range(nnet):
                net = inv.network(inet)
                dbr.load(net)
                nsta = net.stationCount()
                for ista in range(nsta):
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

                    _sta = _Station()
                    _sta.code = sta.code()
                    _sta.net = net.code()
                    _sta.lat, _sta.lon = sta.latitude(), sta.longitude()

                    _stations.append(_sta)

            for p in grid:
                distances = []
                for s in _stations:
                    d, a, b = seis.geo.delazi(p.lat, p.lon, s.lat, s.lon)
                    distances.append(d)
                distances.sort()
                if len(distances) >= nearestStations:
                    p.dist = distances[nearestStations]
                else:
                    p.dist = 180

            writeGrid(grid, "config/grid.conf.new")

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
