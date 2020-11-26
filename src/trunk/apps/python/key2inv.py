#!/usr/bin/env seiscomp-python

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

import os
import re
import sys
import time
import glob
import fnmatch
import decimal
import optparse
from seiscomp import logs
from seiscomp.keyfile import Keyfile
from seiscomp.db.seiscomp3.inventory import Inventory
from nettab.nettab import Instruments, NettabError
import seiscomp3.Client
from functools import reduce

if sys.version_info < (2, 6, 0):
    from sets import Set as set


def sortDictionary(dict):
    newDict = {}
    sortedKeys = list(dict.keys())
    sortedKeys.sort()

    for key in sortedKeys:
        newDict[key] = dict[key]
    return newDict
# end


gainTable = {}


def loadGains(fileName):
    try:
        fd = open(fileName)
        try:
            line = fd.readline()
            lineno = 0
            while line:
                line = line.strip()
                lineno += 1

                if not line or line[0] == '#':
                    line = fd.readline()
                    continue

                try:
                    (deviceName, deviceIdPattern,
                     streamPattern, gain) = line.split()
                    if deviceName in gainTable:
                        gainTable[deviceName].append((streamPattern, deviceIdPattern,
                                                      float(gain)))
                    else:
                        gainTable[deviceName] = [(streamPattern, deviceIdPattern,
                                                  float(gain))]

                except (TypeError, ValueError):
                    logs.error("%s:%d: parse error" % (fileName, lineno))
                    sys.exit(1)

                line = fd.readline()

        finally:
            fd.close()

    except IOError as e:
        logs.error("cannot open %s: %s" % (fileName, str(e)))
        sys.exit(1)


def getGain(datalogger, dataloggerId, seismometer, seismometerId, streamCode):
    try:
        if datalogger == "DUMMY":
            dataloggerGain = 1.0

        else:
            for (streamPattern, dataloggerIdPattern, dataloggerGain) in gainTable[datalogger]:
                if fnmatch.fnmatch(streamCode, streamPattern) and \
                        fnmatch.fnmatch(dataloggerId, dataloggerIdPattern):
                    break

            else:
                logs.error("cannot find gain for %s, %s, %s" % (datalogger,
                                                                dataloggerId, streamCode))

                return 0

        if seismometer == "DUMMY":
            seismometerGain = 1.0

        else:
            for (streamPattern, seismometerIdPattern, seismometerGain) in gainTable[seismometer]:
                if fnmatch.fnmatch(streamCode, streamPattern) and \
                        fnmatch.fnmatch(seismometerId, seismometerIdPattern):
                    break

            else:
                logs.error("cannot find gain for %s, %s, %s" % (seismometer,
                                                                seismometerId, streamCode))

                return 0

    except KeyError as e:
        logs.error("cannot find gain for " + str(e))
        return 0

    return dataloggerGain * seismometerGain


_rx_samp = re.compile(r'(?P<bandCode>[A-Z])?(?P<sampleRate>.*)$')


def _normalize(num, denom):
    if num > denom:
        (a, b) = (num, denom)
    else:
        (a, b) = (denom, num)

    while b > 1:
        (a, b) = (b, a % b)

    if b == 0:
        return (num / a, denom / a)

    return (num, denom)


def _rational(x):
    sign, mantissa, exponent = x.as_tuple()
    sign = (1, -1)[sign]
    mantissa = sign * reduce(lambda a, b: 10 * a + b, mantissa)
    if exponent < 0:
        return _normalize(mantissa, 10 ** (-exponent))
    else:
        return (mantissa * 10 ** exponent, 1)


def parseSampling(sampling):
    compressionLevel = "2"
    instrumentCode = "H"
    locationCode = ""
    endPreamble = sampling.find('_')
    if endPreamble > 0:
        for x in sampling[:endPreamble].split('/'):
            if x[0] == 'F':
                compressionLevel = x[1:]
            elif x[0] == 'L':
                locationCode = x[1:]
            elif x[0] == 'T':
                instrumentCode = x[1:]
            else:
                logs.warning("unknown code %s in %s" % (x[0], sampling))

    if not sampling[endPreamble+1:]:
        return

    for x in sampling[endPreamble+1:].split('/'):
        m = _rx_samp.match(x)
        if not m:
            logs.error("error parsing sampling %s at %s" % (sampling, x))
            continue

        try:
            sampleRate = decimal.Decimal(m.group('sampleRate'))
        except decimal.InvalidOperation:
            logs.error("error parsing sampling %s at %s" % (sampling, x))
            continue

        bandCode = m.group('bandCode')
        if not bandCode:
            if sampleRate >= 80:
                bandCode = 'H'
            elif sampleRate >= 40:
                bandCode = 'S'
            elif sampleRate > 1:
                bandCode = 'B'
            elif sampleRate == 1:
                bandCode = 'L'
            elif sampleRate == decimal.Decimal("0.1"):
                bandCode = 'V'
            elif sampleRate == decimal.Decimal("0.01"):
                bandCode = 'U'
            else:
                logs.error(
                    "could not determine band code for %s in %s" (x, sampling))
                continue

        yield ((bandCode + instrumentCode, locationCode) +
               _rational(sampleRate) + ("Steim" + compressionLevel,))


def parseOrientation(orientation):
    for x in orientation.split(';'):
        try:
            (code, azimuth, dip) = x.split()
            yield (code, float(azimuth), float(dip))
        except (TypeError, ValueError):
            logs.error("error parsing orientation %s at %s" % (orientation, x))
            continue


_doy = (0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365)


def _is_leap(y):
    return (y % 400 == 0 or (y % 4 == 0 and y % 100 != 0))


def _ldoy(y, m):
    return _doy[m] + (_is_leap(y) and m >= 2)


def _dy2mdy(doy, year):
    month = 1
    while doy > _ldoy(year, month):
        month += 1

    mday = doy - _ldoy(year, month - 1)
    return (month, mday)


_rx_datetime = re.compile(
    r'(?P<year>[0-9]+)/(?P<doy>[0-9]+)(:(?P<hour>[0-9]{2})(?P<minute>[0-9]{2})(?P<second>[0-9]{2})?)?$')


def parseDate(datestr):
    m = _rx_datetime.match(datestr)
    if not m:
        logs.error("invalid date: " + datestr)
        return (seiscomp3.Core.Time(1980, 1, 1, 0, 0, 0), "1980-01-01T00:00:00.0000Z")

    try:
        year = int(m.group("year"))
        (month, mday) = _dy2mdy(int(m.group("doy")), year)

        if m.group("hour"):
            hour = int(m.group("hour"))
            minute = int(m.group("minute"))
        else:
            hour = 0
            minute = 0

        if m.group("second"):
            second = int(m.group("second"))
        else:
            second = 0

        coretime = seiscomp3.Core.Time(year, month, mday, hour, minute, second)

    except (TypeError, ValueError, IndexError):
        logs.error("invalid date: " + datestr)
        return (seiscomp3.Core.Time(1980, 1, 1, 0, 0, 0), "1980-01-01T00:00:00.0000Z")

    return (coretime, coretime.toString("%Y-%m-%dT%H:%M:%S.%fZ"))


_rx_pkg = re.compile(r'(?P<pkg>[^:\s]+)(:(?P<profile>[^\s]+))?$')


def parsePkgstr(pkgstr):
    result = {}
    for x in pkgstr.split():
        m = _rx_pkg.match(x)
        if not m:
            logs.error("error parsing %s at %s" % (pkgstr, x))
            continue

        result[m.group('pkg')] = m.group('profile')

    if 'trunk' not in result:
        result['trunk'] = None

    return result


class StreamWrapper(object):
    def __init__(self, stream):
        self.obj = stream

    def update(self, rate, rateDiv, orientation, datalogger, dataloggerId,
               seismometer, seismometerId, channel, depth, azimuth, dip,
               gainFreq, gainMult, gainUnit, format, restricted, inv):

        errmsg = None

        try:
            (smsn, smgain) = (seismometerId.split('/') + [None])[:2]
            (datalogger, dlgain, seismometer, smgain, gainFreq, gainUnit) = instdb.update_inventory(inv,
                                                                                                    datalogger, dataloggerId, gainMult, seismometer, smsn, smgain, rate, rateDiv)

        except NettabError as e:
            errmsg = str(e)
            dlgain = []
            smgain = []

        self.obj.setSampleRateNumerator(rate)
        self.obj.setSampleRateDenominator(rateDiv)
        self.obj.setDatalogger(datalogger)
        self.obj.setDataloggerSerialNumber(dataloggerId)
        self.obj.setDataloggerChannel(channel)
        self.obj.setSensor(seismometer)
        self.obj.setSensorSerialNumber(seismometerId)
        self.obj.setSensorChannel(channel)
        self.obj.setDepth(depth)
        self.obj.setAzimuth(azimuth)
        self.obj.setDip(dip)
        self.obj.setGainFrequency(gainFreq)
        self.obj.setGainUnit(gainUnit)
        self.obj.setFormat(format)
        self.obj.setFlags("GC")
        self.obj.setRestricted(restricted)
        self.obj.setShared(True)

        complete = True

        try:
            gain = smgain[channel] * dlgain[channel]

        except IndexError:
            complete = False
            gain = gainMult * getGain(datalogger, dataloggerId, seismometer,
                                      seismometerId, self.obj.code())

            if gain == 0 and errmsg:
                logs.error(errmsg)

        self.obj.setGain(gain)

        return complete


class StationWrapper(object):
    def __init__(self, netCode, station):
        self.obj = station
        self.netCode = netCode
        self.streams = {}
        self.__loc = {}
        self.__usedStreams = set()

        for i in range(station.sensorLocationCount()):
            loc = station.sensorLocation(i)
            self.__loc[loc.code()] = loc

            for j in range(loc.streamCount()):
                stream = loc.stream(j)
                try:
                    s = self.streams[(loc.code(), stream.code())]
                    if s.obj.start() > stream.start():
                        continue

                except KeyError:
                    pass

                self.streams[(loc.code(), stream.code())
                             ] = StreamWrapper(stream)

    def __updateStreams(self, sampling, orientation, datalogger, dataloggerId,
                        seismometer, seismometerId, depth, gainFreq, gainMult, gainUnit,
                        startDate, restricted, inv):

        complete = True
        channel = 0

        for (compCode, azimuth, dip) in parseOrientation(orientation):
            for (biCode, locCode, rate, rateDiv, format) in parseSampling(sampling):
                streamCode = biCode + compCode
                #(start, startString) = parseDate(startDate)
                start = self.obj.start()
                stream = self.streams.get((locCode, streamCode))
                if stream and stream.obj.start() < start:
                    stream.obj.setEnd(start)
                    stream.obj.update()
                    stream = None

                if stream is None:
                    loc = self.__loc.get(locCode)
                    if loc is None:
                        loc = seiscomp3.DataModel.SensorLocation.Create()
                        loc.setCode(locCode)
                        loc.setStart(start)
                        self.obj.add(loc)
                        self.__loc[locCode] = loc

                    else:
                        loc.update()

                    loc.setLatitude(self.obj.latitude())
                    loc.setLongitude(self.obj.longitude())
                    loc.setElevation(self.obj.elevation())

                    stream = StreamWrapper(seiscomp3.DataModel.Stream.Create())
                    stream.obj.setCode(streamCode)
                    stream.obj.setStart(start)
                    loc.add(stream.obj)
                    self.streams[(locCode, streamCode)] = stream

                else:
                    stream.obj.update()

                complete = stream.update(rate, rateDiv, orientation,
                                         datalogger, dataloggerId, seismometer, seismometerId,
                                         channel, depth, azimuth, dip, gainFreq, gainMult, gainUnit,
                                         format, restricted, inv) and complete

                self.__usedStreams.add((locCode, streamCode))

            channel += 1

        return complete

    def update(self, DCID, restricted, kf, inv):
        self.obj.setDescription(kf.statDesc)
        self.obj.setLatitude(float(kf.latitude))
        self.obj.setLongitude(float(kf.longitude))
        self.obj.setElevation(float(kf.elevation))
        self.obj.setType(kf.type)
        self.obj.setRestricted(restricted)
        self.obj.setShared(True)
        self.obj.setArchive(DCID)

        self.__usedStreams = set()

        complete = self.__updateStreams(kf.sampling1, kf.orientation1, kf.datalogger,
                                        kf.dataloggerSn, kf.seismometer1, kf.seismometerSn1, float(
                                            kf.depth1),
                                        0.02, float(kf.gainMult1), kf.unit1, kf.startDate, restricted, inv)

        if kf.sampling2:
            if not kf.depth2:
                logs.warning("missing depth of secondary sensor for %s %s" % (
                    self.netCode, self.obj.code()))
                return

            if not hasattr(kf, "datalogger2") or not kf.datalogger2:
                kf.datalogger2 = kf.datalogger

            if not hasattr(kf, "dataloggerSn2") or not kf.dataloggerSn2:
                kf.dataloggerSn2 = kf.dataloggerSn

            complete = self.__updateStreams(kf.sampling2, kf.orientation2, kf.datalogger2,
                                            kf.dataloggerSn2, kf.seismometer2, kf.seismometerSn2, float(
                                                kf.depth2),
                                            1.0, float(kf.gainMult2), kf.unit2, kf.startDate, restricted, inv) and complete

        for (locCode, streamCode) in set(self.streams.keys()) - self.__usedStreams:
            self.__loc[locCode].remove(self.streams[(locCode, streamCode)].obj)
            del self.streams[(locCode, streamCode)]

        inv.flush()
        return complete


class NetworkWrapper(object):
    def __init__(self, network):
        self.obj = network

    def update(self, DCID, kf):
        self.obj.setDescription(kf.netDesc)
        self.obj.setArchive(DCID)


class InventoryWrapper(object):
    def __init__(self, inventory, DCID):
        self.obj = inventory
        self.inv = Inventory(inventory)
        self.DCID = DCID
        self.networks = {}
        self.stations = {}

        for i in range(inventory.networkCount()):
            network = inventory.network(i)
            self.networks[network.code()] = NetworkWrapper(network)

            for j in range(network.stationCount()):
                station = network.station(j)
                self.stations[(network.code(), station.code())
                              ] = StationWrapper(network.code(), station)

    def updateNetwork(self, netCode, kf):
        network = self.networks.get(netCode)
        if network is None:
            network = NetworkWrapper(
                seiscomp3.DataModel.Network("Network/%s" % (netCode,)))
            network.obj.setCode(netCode)
            network.obj.setStart(seiscomp3.Core.Time(1980, 1, 1, 0, 0, 0))
            network.obj.setArchive(self.DCID)
            network.obj.setType("BB")
            network.obj.setNetClass("p")
            network.obj.setRestricted(False)
            network.obj.setShared(True)
            self.obj.add(network.obj)
            self.networks[netCode] = network

        else:
            network.obj.update()

        network.update(self.DCID, kf)

    def updateStation(self, netCode, staCode, restricted, kf):
        station = self.stations.get((netCode, staCode))
        (start, startString) = parseDate(kf.startDate)
        if station and station.obj.start() < start:
            for i in range(station.obj.sensorLocationCount()):
                loc = station.obj.sensorLocation(i)
                for j in range(loc.streamCount()):
                    try:
                        loc.stream(j).end()
                        continue
                    except ValueError:
                        loc.stream(j).setEnd(start)
                        loc.stream(j).update()

                try:
                    station.obj.sensorLocation(i).end()
                    continue
                except ValueError:
                    station.obj.sensorLocation(i).setEnd(start)
                    station.obj.sensorLocation(i).update()

            station.obj.setEnd(start)
            station.obj.update()
            station = None

        if station is None:
            station = StationWrapper(netCode, seiscomp3.DataModel.Station(
                "Station/%s/%s/%s" % (netCode, staCode, startString)))
            station.obj.setCode(staCode)
            station.obj.setStart(start)
            self.networks[netCode].obj.add(station.obj)
            self.stations[(netCode, staCode)] = station

        else:
            station.obj.update()

        return station.update(self.DCID, restricted, kf, self.inv)

    def setNetworkRestricted(self, netCode, state):
        network = self.networks.get(netCode)
        if network.obj.restricted() != state:
            network.obj.setRestricted(state)
            network.obj.update()

    def setStationRestricted(self, netCode, staCode, state):
        station = self.stations.get((netCode, staCode))
        try:
            if station.obj.restricted() != state:
                station.obj.setRestricted(state)
                station.obj.update()
        except:
            station.obj.setRestricted(state)
            station.obj.update()

        for stream in station.streams.values():
            try:
                if stream.obj.restricted() != state:
                    stream.obj.setRestricted(state)
                    stream.obj.update()
            except:
                stream.obj.setRestricted(state)
                stream.obj.update()


class Key2DB(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)

        self.setLoggingToStdErr(True)

        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(False, True)

    def createCommandLineDescription(self):
        self.commandline().addGroup("Convert")
        self.commandline().addOption("Convert", "formatted,f", "Enable formatted output")

    def initConfiguration(self):
        if not seiscomp3.Client.Application.initConfiguration(self):
            return false

        opts = self.commandline().unrecognizedOptions()
        if len(opts) < 1:
            sys.stderr.write("Missing root directory that holds key files\n")
            sys.stderr.write("Usage: key2inv key-dir [output=stdout]\n")
            return False

        if len(opts) > 1:
            self.output = opts[1]
        else:
            self.output = ""

        # force logging to stderr even if logging.file = 1
        self.setLoggingToStdErr(True)

        return True

    def __load_file(self, func, file):
        if file and os.path.exists(file):
            logs.info("loading " + file)
            func(file)

    def run(self):
        try:
            seiscompRoot = self.commandline().unrecognizedOptions()[0]
            sys.stderr.write("root directory: %s\n" % seiscompRoot)

            try:
                DCID = self.configGetString("datacenterID")

            except:
                logs.error("datacenterID not found in global.cfg")
                return False

            networkRestricted = {}
            incompleteResponse = {}

            global instdb
            instdb = Instruments(DCID)

            self.__load_file(loadGains, os.path.join(
                seiscompRoot, "config", "gain.dlsv"))

            # for backwards compatibility
            self.__load_file(loadGains, os.path.join(
                seiscompRoot, "config", "gain.tab.out"))
            self.__load_file(loadGains, os.path.join(
                seiscompRoot, "config", "gain.tab"))

            try:
                self.__load_file(instdb.load_db, os.path.join(
                    seiscompRoot, "resp", "inst.db"))
                self.__load_file(instdb.load_sensor_attr, os.path.join(
                    seiscompRoot, "resp", "sensor_attr.csv"))
                self.__load_file(instdb.load_datalogger_attr, os.path.join(
                    seiscompRoot, "resp", "datalogger_attr.csv"))

            except (IOError, NettabError) as e:
                logs.error("fatal error: " + str(e))
                return False

            sc3Inv = seiscomp3.DataModel.Inventory()
            inventory = InventoryWrapper(sc3Inv, DCID)

            existingNetworks = set()
            existingStations = set()

            for f in glob.glob(os.path.join(seiscompRoot, "key", "network_*")):
                try:
                    logs.debug("processing " + f)
                    netCode = f.split("/network_")[-1]
                    try:
                        kf = Keyfile(f)
                    except IOError as e:
                        logs.error(str(e))
                        continue

                    existingNetworks.add(netCode)
                    networkRestricted[netCode] = False

                    inventory.updateNetwork(netCode, kf)

                except ValueError as e:
                    logs.error("%s: %s" % (f, str(e)))

            for f in glob.glob(os.path.join(seiscompRoot, "key", "station_*")):
                try:
                    logs.debug("processing " + f)
                    (netCode, staCode) = f.split("/station_")[-1].split('_', 1)
                    try:
                        kf = Keyfile(f)
                    except IOError as e:
                        logs.error(str(e))
                        continue

                    existingStations.add((netCode, staCode))

                    if netCode not in existingNetworks:
                        logs.warning(
                            "network %s does not exist, ignoring station %s" % (netCode, staCode))
                        continue

                    if not hasattr(kf, "latitude") or not kf.latitude:
                        logs.warning("missing latitude for %s %s" %
                                     (netCode, staCode))
                        continue

                    if not hasattr(kf, "longitude") or not kf.longitude:
                        logs.warning("missing longitude for %s %s" %
                                     (netCode, staCode))
                        continue

                    if not hasattr(kf, "elevation") or not kf.elevation:
                        logs.warning("missing elevation for %s %s" %
                                     (netCode, staCode))
                        continue

                    if not hasattr(kf, "depth1") or not kf.depth1:
                        logs.warning(
                            "missing depth of primary sensor for %s %s" % (netCode, staCode))
                        continue

                    if decimal.Decimal(kf.latitude) == decimal.Decimal("0.0") and \
                            decimal.Decimal(kf.longitude) == decimal.Decimal("0.0"):
                        logs.warning("missing coordinates for %s %s" %
                                     (netCode, staCode))
                        continue

                    if not hasattr(kf, "orientation1") or not kf.orientation1:
                        logs.warning("missing orientation of primary sensor for %s %s, using default" % (
                            netCode, staCode))
                        kf.orientation1 = "Z 0.0 -90.0; N 0.0 0.0; E 90.0 0.0"

                    if not hasattr(kf, "orientation2"):
                        kf.orientation2 = ""

                    if not hasattr(kf, "unit1") or not kf.unit1:
                        logs.warning(
                            "missing unit of primary sensor for %s %s, using M/S" % (netCode, staCode))
                        kf.unit1 = "M/S"

                    if not hasattr(kf, "unit2"):
                        logs.warning(
                            "missing unit of secondary sensor for %s %s, using M/S**2" % (netCode, staCode))
                        kf.unit2 = "M/S**2"

                    if not hasattr(kf, "type"):
                        kf.type = ""

                    restricted = False

                    # TODO: Make restricted part of the key file

                    if not inventory.updateStation(netCode, staCode, restricted, kf):
                        try:
                            incNet = incompleteResponse[netCode]

                        except KeyError:
                            incNet = set()
                            incompleteResponse[netCode] = incNet

                        incNet.add(staCode)

                except ValueError as e:
                    logs.error("%s: %s" % (f, str(e)))

            for (netCode, restricted) in networkRestricted.items():
                inventory.setNetworkRestricted(netCode, restricted)

            for (netCode, network) in inventory.networks.items():
                if netCode not in existingNetworks:
                    logs.notice("deleting network %s from inventory" %
                                (netCode,))
                    inventory.obj.remove(network.obj)

            for ((netCode, staCode), station) in inventory.stations.items():
                if netCode in existingNetworks and (netCode, staCode) not in existingStations:
                    logs.notice("deleting station %s_%s from inventory" %
                                (netCode, staCode))
                    inventory.networks[netCode].obj.remove(station.obj)

            if incompleteResponse:
                logs.info(
                    "The following stations are missing full response data")
                logs.info("Use dlsv2inv if needed")

                # for netCode in sorted(incompleteResponse.keys()):
                #    logs.info("%s: %s" % (netCode, " ".join(sorted(list(incompleteResponse[netCode])))))
                tmpDict = sortDictionary(incompleteResponse)
                for netCode in list(tmpDict.keys()):
                    tmpSortedList = list(tmpDict[netCode])
                    tmpSortedList.sort()
                    logs.info("%s: %s" % (netCode, " ".join(tmpSortedList)))

            ar = seiscomp3.IO.XMLArchive()
            if not self.output:
                sys.stderr.write("Writing output to stdout\n")
                if not ar.create("-"):
                    sys.stderr.write("Cannot open open stdout\n")
                    return False
            else:
                sys.stderr.write("Writing output to %s\n" % self.output)
                if not ar.create(self.output):
                    sys.stderr.write("Cannot open open %s\n" % self.output)
                    return False

            ar.setFormattedOutput(self.commandline().hasOption("formatted"))
            ar.writeObject(sc3Inv)

        except Exception:
            logs.print_exc()

        return True


if __name__ == "__main__":
    logs.debug = seiscomp3.Logging.debug
    logs.info = seiscomp3.Logging.info
    logs.notice = seiscomp3.Logging.notice
    logs.warning = seiscomp3.Logging.warning
    logs.error = seiscomp3.Logging.error
    app = Key2DB(len(sys.argv), sys.argv)
    sys.exit(app())
