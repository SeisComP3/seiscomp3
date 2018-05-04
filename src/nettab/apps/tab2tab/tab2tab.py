#!/usr/bin/env python

import os
import sys
from datetime import datetime
from nettab.convertUtils import StationAttributes, NetworkAttributes, StationMappings, parseDate, formatDate, quote, hummanStr
from nettab.tab import Tab
from optparse import OptionParser
from nettab.nodesi import Instruments

class TabConverter:
    def __init__(self, networkCode):
        self.__fmt__ = None
        self.takeSugestions = None
        
        self.filename = None
        
        self.networkCode = networkCode
        self.stationList = None

        self.nat  = None
        self.sat  = None
        self.sma  = None
        self.inst = None
        self.defaultEpoch = parseDate("1980/001")
        
        self.start=0
        self.code=0
        self.description=0 
        self.datalogger=0
        self.sensor=0
        self.channel=0
        self.gaind = 0
        self.longitude=0
        self.latitude=0
        self.elevation=0
        self.end=0
        self.depth=0
        self.orientation=0
        
        ## default dates
        self.startDate = parseDate("1980/001")
        self.endDate = parseDate(None)

    def loadStationMapping(self, filename):
        if self.networkCode is None: raise Exception("Cannot load Station mapping without network code")
        if self.stationList is None: raise Exception("Cannot load Station mapping without station list")
        
        try:
            sm = StationMappings(self.networkCode, self.stationList, filename)
            self.sma = sm
        except Exception as e:
            raise e

    def loadStationAttribute(self, filename):
        if self.networkCode is None: raise Exception("Cannot load Station att without network code")
        if self.stationList is None: raise Exception("Cannot load Station att without station list")

        try:
            sa = StationAttributes(self.networkCode, self.stationList, filename)
            self.sat = sa
        except Exception as e:
            raise e

    def loadNetworkAttribute(self, filename):
        if self.networkCode is None: raise Exception("Cannot load Network att without network code")
        if self.stationList is None: raise Exception("Cannot load Network att without station list")
        try:
            na = NetworkAttributes(self.networkCode, filename)
            self.nat = na
        except Exception as e:
            raise e

    def loadInstrumentsFile(self, filename, filterFolder):
        tab = Tab(filterFolder=filterFolder)
        tab.digest(filename)
        if tab.i:
            self.inst = tab.i

    def __fmtline__(self):
        if not self.__fmt__:
            fmt = "Sl: "
            fmt += "%%-%ds" % self.code
            fmt += " %%-%ds" % self.description 
            fmt += " %%-%ds" % self.datalogger
            fmt += " %%-%ds" % self.sensor
            fmt += " %%-%ds" % self.channel
            fmt += " %%-%ds" % self.orientation    
            fmt += " %%-%ds" % self.latitude
            fmt += " %%-%ds" % self.longitude
            fmt += " %%-%ds" % self.elevation
            fmt += " %%-%ds" % self.depth
            fmt += " %%-%ds" % self.start
            fmt += " %%-%ds" % self.end
            self.__fmt__ = fmt
        
        return self.__fmt__

    def __analyseLine__(self, items):
        inputLine = " ".join(items)
        if len(items) < 4:
            raise Exception("Invalid items count on line %s" % inputLine)
        
        if len(items) <= 5:
            netCode = items[2]
            if netCode != self.networkCode:
                raise Exception("Tab file (%s) doesn't match class (%s) -- %s" % (netCode,self.networkCode,inputLine))
            return [None, None, None]
        else:
            if len(items) < 6:
                raise Exception("Invalid Station line %s" % inputLine)

            stationCode = items.pop(0)
            code = len(stationCode)
            self.code=max(self.code,code)
    
            description = len(quote(hummanStr(items.pop(0))))
            self.description=max(self.description, description) 
    
            datalogger = len(items.pop(0))
            self.datalogger=max(self.datalogger, datalogger)
    
            sensor = len(items.pop(0))
            self.sensor=max(self.sensor, sensor)
    
            # Gain
            gaind =  items.pop(0)
            if float(gaind) != 1.0:
                self.datalogger = max (self.datalogger, datalogger + len(gaind))
            
            channel = len(items.pop(0))
            self.channel=max(self.channel, channel)
    
            latitude = len(items.pop(0))
            self.latitude=max(self.latitude, latitude)
    
            longitude = len(items.pop(0))
            self.longitude=max(self.longitude, longitude)
    
            elevation = len(items.pop(0))
            self.elevation=max(self.elevation, elevation)
    
            #Orientation
            depth = items.pop(0)
            try:
                float(depth)
                orientation="ZNE"
            except:
                orientation = "Z"
                (depth,a1,a2) = depth.split("/")
                
                a1n = float(a1)
                a2n = float(a2)
    
                orientation+="1"
                if a1n != 0.0: orientation += "(0.0,%s)"%a1
        
                orientation+="2"
                if a2n != 90.0: orientation+="(0.0,%s)"%a1
    
            orientation = len(orientation)
            self.orientation=max(self.orientation, orientation)
    
            depth = len(depth)            
            self.depth=max(self.depth, depth)
    
            # Start
            try:
                start = parseDate(items.pop(0))
                self.start = max (self.start, len(formatDate(start)))
            except:
                raise Exception ("Invalid Station line start date %s" % inputLine)
            
            # End
            try:
                end = parseDate(items.pop(0))
            except:
                end=parseDate("")
                pass
            self.end = max (self.end, len(formatDate(end)))

            return [stationCode, start, end]

    def preload(self, filename, takeSugestions):
        self.takeSugestions = takeSugestions
        sugestedStart = datetime.now()
        sugestedEnd = self.defaultEpoch
        stationList = []

        error = []

        # Some initialization
        if self.filename is not None:
            raise Exception("Cannot pre-load two different files (current one is %s)" % self.filename)
        
        print("Analysing ... ", file=sys.stderr)
        fd = open(filename)
        for line in fd:
            line = line.strip()
            if not line or line[0] == "#": continue

            try:
                (stationCode, start, end) = self.__analyseLine__(line.split())
            except Exception as e:
                error.append(str(e))
                continue
            
            if not stationCode: continue
            if stationCode not in stationList:
                stationList.append(stationCode)
                
            sugestedStart = min(sugestedStart, start)
            if end and sugestedEnd:
                sugestedEnd = max(sugestedEnd, end)
            else:
                sugestedEnd = None
        fd.close()

        if len(error):
            raise Exception("\n".join(error))
        
        print(" Loaded %d different stations" % len(stationList), file=sys.stderr)
        if takeSugestions:
            print(" Taking suggestion start date of %s " % formatDate(self.startDate), file=sys.stderr)
            self.startDate = sugestedStart
            print(" Taking suggestion end date of %s " % formatDate(self.endDate), file=sys.stderr)
            self.endDate = sugestedEnd
        
        self.filename = filename
        self.stationList = stationList
        print("Done.", file=sys.stderr)

    def __convertHeader__(self, line, fdo):
        
        # Split line
        items = line.split()
        
        if not self.takeSugestions:
            if self.nat.hasStart: 
                print(" Using start from attribute.", file=sys.stderr)
                self.startDate = self.nat.startDate
            if self.nat.hasEnd: 
                print(" Using end from attribute.", file=sys.stderr)
                self.endDate = self.nat.endDate

        nCode = items[2].strip()
        if nCode != self.networkCode:
            raise Exception("Wrong network code found: %s != %s" % (self.networkCode, nCode))
        
        fdo.write("Nw: %s %s %s" % (nCode, formatDate(self.startDate), formatDate(self.endDate)) + "\n")
        
        self.nat.dump(fdo)

    def __convertLine__(self, line, fdo, atFront):
        lnfmt = self.__fmtline__()

        # Split line
        items = line.split()

        try:
            code = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Code on %s" % line)
        
        if code not in self.stationList:
            raise Exception("Unknow station code $s" % code)
        
        try:
            hummanStr(items.pop(0))
        except Exception as e:
            raise Exception ("Missing Gain on %s" % line)

        try:
            datalogger = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Datalogger on %s" % line)

        try:
            sensor = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Sensor on %s" % line)

        try:
            gaind =  items.pop(0)
            if float(gaind) != 1.0:
                if not self.inst:
                    raise Exception("Instrument database needed to convert gain")
                try:
                    dte = self.inst.dls[str(datalogger).split("%")[0]]
                except Exception as e:
                    print(e, file=sys.stderr)
                    raise Exception("Datalogger %s not found" % str(datalogger).split("%")[0])
                datalogger += "%%%s" % (float(dte.gain) * float(gaind))
                print(" Converting gain multiplier to real gain using instrument DB on %s" % code, file=sys.stderr)
        except Exception as e:
            raise Exception ("Missing Gain on %s (%s)" % (line,str(e)))

        try:
            channel = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Channel on %s" % line)
        
        try:
            latitude = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Latitude on %s" % line)
        
        try:
            longitude = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Longitude on %s" % line)
        try:
            elevation = items.pop(0)
        except Exception as e:
            raise Exception ("Missing Elevation on %s" % line)

        try:
                depth =  items.pop(0)
        except Exception as e:
            raise Exception ("Missing Depth on %s" % line)
        
        #Orientation 
        try:
            float(depth)
            orientation = "ZNE"
        except:
            orientation = "Z"
            (depth,a1,a2) = depth.split("/")
            
            a1n = float(a1)
            if a1n == 0.0:
                orientation+="1"
            else:
                orientation+="1(0.0,%s)"%a1
    
            a2n = float(a2)
            if a2n == 90.0:
                orientation+="2"
            else:
                orientation+="2(0.0,%s)"%a2

        # Start
        try:
            start =  items.pop(0)
        except Exception:
            raise Exception ("Missing Start on %s" % line)
        
        try:
            start = parseDate(start)
        except Exception as e:
            raise Exception("Invalide Start date: %s (%s) on %s" % (start, e, line))
        
        #End 
        try:
            end =  items.pop(0)
        except:
            end = ""

        try:
            end = parseDate(end)
        except Exception as e:
            raise Exception("Invalide End date: %s (%s) on %s" % (end, e, line))
        
        [place, country] = self.sat.parseStationLine(line.split())
        description = "%s/%s" % (place, country)

        ## Prepare necessary output
        if not atFront:
            self.sma.dump(fdo, code)
            self.sat.dump(fdo, code)
        
        for (start, end) in self.sma.getMappings(code, start, end):
            fdo.write(lnfmt % (code, quote(description), datalogger, sensor, channel, orientation, latitude, longitude, elevation, depth, formatDate(start), formatDate(end))  + "\n")
        
        return code

    def convert(self, fdo, keepcomments = False, atFront = True):
        if self.filename is None:
            raise Exception("You should pre-load a tab file before before converting.")
        
        ## Obtain additional attribute classes if needed
        if not self.nat:
            self.nat = NetworkAttributes(self.networkCode, None)
        if not self.sat:
            self.sat = StationAttributes(self.networkCode, self.stationList, None)
        if not self.sma:
            self.sma = StationMappings(self.networkCode, self.stationList, None)

        # Parse in again the station lines and network header by the additional classes
        print("Pre-Parsing Station/Network lines ... ", file=sys.stderr)
        fd = open(self.filename)
        for line in fd:
            line = line.strip()
            if not line or line[0] == "#": 
                continue
            items = line.split()
            if len(items) <= 5:
                self.nat.parseNetworkLine(items)
            elif len(items) <= 12:
                self.sma.parseStationLine(items)
                self.sat.parseStationLine(items)
        fd.close()
        
        fd = open(self.filename)
        oldcode="" # Station code of the last printed line
        last="" # Type of the last printed line
        print("Converting ... ", file=sys.stderr)
        for line in fd:
            line = line.strip()
            if not line or line[0] == "#": 
                if last == "l" or last == "a" or last == "h": fdo.write("\n")
                if keepcomments: fdo.write(line + "\n")
                last = "c"
                continue
            items = line.split()
            if len(items) <= 5:
                self.__convertHeader__(line, fdo)
                last = "h"
                if (atFront):
                    fdo.write("\n")
                    self.sma.dump(fdo, None)
                    self.sat.dump(fdo, None)
                    last = "a"
                    fdo.write("\n")
            elif len(items) <= 12:
                if (last == "l" and items[0].strip() != oldcode) or last == "h": fdo.write("\n")
                oldcode = self.__convertLine__(line, fdo, atFront)
                last = "l"
                pass
            else:
                print("input at %s" % line, file=sys.stderr)
        fd.close()

def main():
    # Creating the parser
    parser = OptionParser(usage="Old tab to New tab converter", version="1.0", add_help_option=True)

    parser.add_option("", "--instdb", type="string",
                      help="Indicates the instrument databases file to use", dest="inst", default=None)
    parser.add_option("", "--smap", type="string",
                      help="Indicates the station attribute file to use", dest="smap", default=None)
    parser.add_option("", "--sat", type="string",
                      help="Indicates the station attribute file to use", dest="sat", default=None)
    parser.add_option("", "--nat", type="string",
                      help="Indicates the station attribute file to use", dest="nat", default=None)
    parser.add_option("-t", "--tab", type="string",
                      help="Indicates the tab file to convert", dest="tabFile", default=None)
    parser.add_option("-f", "--filterf", type="string",
                      help="Indicates a folder containing the filters coefficients files", dest="ffolder", default=None)
    parser.add_option("-n", "--net", type="string",
                      help="Indicates a two leter station code", dest="netCode", default=None)
    parser.add_option("-g", "--globalsa", action="store_true",
                      help="Indicate that we should put a condensed version of the station attributes just below the network definition", dest="globalSa", default=False)
    parser.add_option("-a", "--autotime", action="store_true",
                      help="Guess the start and end times for a network from the channel times", dest="autoTime", default=False)
    parser.add_option("-c", "--clean", action="store_true",
                      help="Remove the comments and blank lines", dest="cleanFile", default=False)

    # Parsing & Error check
    (options, args) = parser.parse_args()
    error = False
    
    if len(args) != 1:
        print("need an Output Filename or '-' for stdout", file=sys.stderr)
        error = True

    if not options.tabFile:
        print("tab file name not supplied", file=sys.stderr)
        error = True

    if options.inst and not options.ffolder:
        print("Filter folder not supplied.", file=sys.stderr)
        error = True

    if options.tabFile and not os.path.isfile(options.tabFile):
        print("supplied tab file (%s) is not a file" % options.tabFile, file=sys.stderr)
        error = True

    if not options.netCode:
        print("network code not supplied", file=sys.stderr)
        error = True

    #if options.autoTime and (options.netStart or options.netEnd):
    #    print >> sys.stderr, "options Auto Time and Network Start/End times are exclusive"
    #    return

    if error:
        print("use -h for getting a help on usage", file=sys.stderr)
        return

    if  args[0] != "-":
        fdo = open(args[0], "w")
    else:
        fdo = sys.stdout

    # Execution
    try:
        cnv = TabConverter(options.netCode.upper())
        cnv.preload(options.tabFile, options.autoTime)

        if options.inst or options.smap or options.nat or options.sat:
            print("Loading optional files: ", file=sys.stderr)

            if options.inst and os.path.isfile(options.inst):
                cnv.loadInstrumentsFile(options.inst, options.ffolder)
    
            if options.smap and os.path.isfile(options.smap):
                cnv.loadStationMapping(options.smap)
    
            if options.nat and os.path.isfile(options.nat): 
                cnv.loadNetworkAttribute(options.nat)
                
            if options.sat and os.path.isfile(options.sat): 
                cnv.loadStationAttribute(options.sat)
            print("Done.", file=sys.stderr)

        cnv.convert(fdo, not options.cleanFile, options.globalSa)
    except Exception as e:
        print("", file=sys.stderr)
        print("Error on processing: %s" % e, file=sys.stderr)

    fdo.close()

if __name__ == "__main__":
    main()
