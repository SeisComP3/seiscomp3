from __future__ import print_function
import sys
import csv
import re
from datetime import datetime

def getFieldNames(fd):
    tmp = fd.readline().split(',')
    fieldNames = []
    for i in tmp:
        fieldNames.append(i.strip())
    return fieldNames

def quote(instr):
    return '"'+instr+'"'

def hummanStr(instr):
    return instr.replace("_"," ")

def parseDate(val):
    if not val or val == "":
        return None
    date=val.replace("/", "-")
    formats={ len("YYYY-JJJ") : "%Y-%j",
            len("YYYY-MM-DD") : "%Y-%m-%d",
            len("YYYY-JJJ:HHMM") : "%Y-%j:%H%M",
            len("YYYY-JJJTHH:MM") : "%Y-%jT%H:%M",
            len("YYYY-MM-DDTHH:MM") : "%Y-%m-%dT%H:%M",
            len("YYYY-JJJTHH:MM:SS") : "%Y-%jT%H:%M:%S",
            len("YYYY-MM-DDTHH:MM:SS") : "%Y-%m-%dT%H:%M:%S"}
    try:
        return datetime.strptime(date, formats[len(date)])
    except Exception as e:
        raise ValueError, "invalid date: " + date + str(e)

def formatDate(date):
    if not date:
        return ""

    if date.hour != 0 or date.minute != 0:
        return datetime.strftime(date,"%Y/%j:%H%M")
    
    return datetime.strftime(date,"%Y/%j")

def isPyVersion(major, minor):
    return sys.version_info[0] == major and \
           sys.version_info[1] == minor

class StationMappings:
    def __init__(self, networkCode, stationList, filename):
        self.networkCode = networkCode
        self.stationList = stationList
        self.stationMapping = {}
        self.stationBreak = {}

        if not filename: return
        _rx_statmap = re.compile(r'\s*([^_]*)_([^=]*)=(\S*)\s*(from=([0-9]+/[0-9]+))?\s*(to=([0-9]+/[0-9]+))?\s*$')
        fd = open(filename)
        stationMapping = {}
        try:
            lineno = 0
            try:
                line = fd.readline()
                lineno = 1
                while line:
                    m = _rx_statmap.match(line)
                    if m is None:
                        raise Exception("parse error")

                    (sta, net, archive_net, from_def, from_year, to_def, to_year) = m.groups()

                    if net != self.networkCode:
                        line = fd.readline()
                        continue

                    if sta not in self.stationList:
                        line = fd.readline()
                        continue
                    
                    try:
                        sta_net = stationMapping[sta]

                    except KeyError:
                        sta_net = []
                        stationMapping[sta] = sta_net

                    if from_def:
                        from_date = parseDate(from_year)

                    else:
                        from_date = None

                    if to_def:
                        to_date = parseDate(to_year)

                    else:
                        to_date = None

                    sta_net.append((from_date, to_date, archive_net))
                    line = fd.readline()
                    lineno += 1

            except (Exception, TypeError, ValueError) as e:
                raise Exception("%s:%d: %s" % (file, lineno, str(e)))

        finally:
            fd.close()
            
        if len(stationMapping):
            print("Found %d station mappings" % len(stationMapping), file=sys.stderr)
            self.stationMapping = stationMapping
        else:
            ## print("No station mappings found", file=sys.stderr)
            pass
    
    def dump(self, fdo, stationCode):
        items = []
        for (code, mapping) in self.stationMapping.iteritems():
            if stationCode and stationCode != code: continue
            items.append(code)
            for (fromDate, toDate, network) in mapping:
                fdo.write("Sa: ArchiveNetworkCode=%s %s" % (network, code))
                if fromDate:
                    fdo.write(" from=%s" % formatDate(fromDate))
                if toDate:
                    fdo.write(" to=%s" % formatDate(toDate))
                fdo.write("\n")

        for code in items:
            self.stationMapping.pop(code)
    
    def getMappings(self, code, start, end):
        mapping = []
        
        if (code, start, end) not in self.stationBreak:
            mapping.append([start, end])
        else:
            for (archiveNet, s, e, fr, to) in self.stationBreak[(code, start, end)]:
                mapping.append([s, e])

        return mapping
    
    def parseStationLine(self, items):
        stationCode = items[0].strip()
        start = parseDate(items[10])
        
        if len(items) > 11:
            end = parseDate(items[11])
        else:
            end = None
    
        if stationCode not in self.stationMapping:
            ## print("Skipping %s not in mapping list" % stationCode, file=sys.stderr)
            return self.getMappings(stationCode, start, end)
    
        for (fDate, tDate, archiveNet) in self.stationMapping[stationCode]:
            if fDate and tDate:
                raise Exception("Not Supported to and from definitions found.")
            elif fDate:
                if fDate >= start:
                    if (end and fDate <= end) or not end:
                        ## print("Processing fDate %s %s %s [%s]" % (stationCode, start, end, fDate), file=sys.stderr)
                        if (stationCode, start, end) in self.stationBreak:
                            raise Exception("Crazy multiple station mapping for the same station line")
                        self.stationBreak[(stationCode, start, end)] = []
                        self.stationBreak[(stationCode, start, end)].append((self.networkCode, start, fDate, fDate, tDate))
                        self.stationBreak[(stationCode, start, end)].append((archiveNet, fDate, end, fDate, tDate))
                        ## prin( " found mapping From -> %s (%s,%s)" % (fDate, stationCode, formatDate(start)), file=sys.stderr)
                        return self.getMappings(stationCode, start, end)
            elif tDate:
                if tDate >= start:
                    if (end and tDate <= end) or not end:
                        ## print("Processing tDate %s %s %s [%s]" % (stationCode, start, end, tDate), file=sys.stderr)
                        if (stationCode, start, end) in self.stationBreak:
                            raise Exception("Crazy multiple station mapping for the same station line")
                        self.stationBreak[(stationCode, start, end)] = []
                        self.stationBreak[(stationCode, start, end)].append((archiveNet, start, tDate, fDate, tDate))
                        self.stationBreak[(stationCode, start, end)].append((self.networkCode, tDate, end, fDate, tDate))
                        ## print(" found mapping To -> %s (%s,%s)" % (tDate, stationCode, formatDate(start)), file=sys.stderr)
                        return self.getMappings(stationCode, start, end)
            else:
                if (stationCode, start, end) in self.stationBreak:
                    raise Exception("Crazy multiple station mapping for the same station line")
                self.stationBreak[(stationCode, start, end)] = []
                self.stationBreak[(stationCode, start, end)].append((archiveNet, start, end, fDate, tDate))
                ## print(" found mapping ALL (%s,%s)" % (stationCode, formatDate(start)), file=sys.stderr)
                return self.getMappings(stationCode, start, end)
        ## print("Ignored %s" % " ".join(items), file=sys.stderr)
        return self.getMappings(stationCode, start, end)
    
class StationAttributes:
    def __init__(self, networkCode, stationList, filename):
        self.networkCode= networkCode
        self.stationList = stationList
        self.stationAttributeList = {}
        
        if not filename: return
        
        fd = open(filename)
        attributes = {}
        try:
            try:
                fieldNames = None
                if isPyVersion(2, 3):
                    fieldNames = getFieldNames(fd)
    
                for row in csv.DictReader(fd, fieldNames):
                    net_code = row['net_code']
                    if net_code != self.networkCode: continue
                    
                    sta_code = row['sta_code']
                    if sta_code not in self.stationList: continue
                    
                    start = parseDate(row['start'].strip())
                                        
                    if sta_code in attributes:
                        raise Exception("multiple %s found in %s" % (str((net_code, sta_code, row['start'])), filename))
                    
                    del row['net_code']
                    del row['sta_code']
                    del row['start']
                    
                    ## Clean up input
                    for key in ['restricted', 'restricted_exc', 'place', 'country', 'affiliation', 'remark']:
                        row[key] = row[key].strip()
                        if len(row[key]) == 0:
                            del row[key]
                    
                    if 'restricted' in row:
                        row['restricted'] = bool(int(row['restricted']))
                        if not row['restricted']: del (row['restricted'])
                    
                    if row:
                        attributes[sta_code] = row

            except KeyError as e:
                raise Exception("column %s missing in %s" % (str(e), filename))
    
            except (TypeError, ValueError) as e:
                raise Exception("error reading %s: %s" % (filename, str(e)))
            
        finally:
            fd.close()
        self.stationAttributeList = self.__build__(attributes)
        print(" loaded attributes for %d stations on network %s (%s)" % (len(self.stationAttributeList), self.networkCode, filename), file=sys.stderr)

    def __build__(self, attributes):
        newat = {}

        if not attributes:
            ## print("no station attributes found for network %s" % self.networkCode, file=sys.stderr)
            return newat
        
        for (code,row) in attributes.iteritems():
            nr = {}
            for (k,v) in row.iteritems():
                if k == 'country': k = 'Country'
                if k == 'place': k = 'Place'
                if k == 'affiliation': k = 'Affiliation'
                if k == 'remark': k = 'Remark'
                if k == 'restricted': k = 'Restricted'
                nr[k] = v
            if nr:
                newat[code] = nr
        return newat

    def get(self, code):
        if self.stationAttributeList and code in self.stationAttributeList:
            return self.stationAttributeList[code]
        else:
            return None
    
    def __parseDescription__(self, description):
        affiliation = None
        place = None
        country = None
        description = hummanStr(description)
        hasStation = True if description.find("Station") >= 0 else False
        
        if hasStation:
            affiliation = description[0:(description.index("Station"))].strip()
            parts = description[description.index("Station")+7:].strip().split(",")
        else:
            parts = description.split(",")
        
        if len(parts) > 1:
            country = parts[len(parts)-1].strip()
            parts = parts[0:(len(parts)-1)]
            place = ",".join(parts)
        else:
            place = ",".join(parts)
        
#        print("Country:", country, file=sys.stderr)
#        print("Place:", place, file=sys.stderr)
#        print("Affiliation:", affiliation, file=sys.stderr)

        oui = {}
        if country:
            oui['Country'] = country
        if place:
            oui['Place'] = place
        if affiliation:
            oui['Affiliation'] = affiliation
        return oui

    def reorder_station_attr(self):
        att = {}
        if not self.stationAttributeList:
            return None
        
        for (code, row) in self.stationAttributeList.iteritems():
            for (k, v) in row.iteritems():
                if k == 'restricted_exc':
                    k = 'Restricted'
                    extra=',*,'+str(v)
                    v = (not row['Restricted']) if 'Restricted' in row else True 
                else:
                    extra= ''
                
                try:
                    dk = att[k]
                except:
                    dk = {}
                    att[k] = dk

                try:
                    dv = dk[str(v)]
                except:
                    dv = []
                    dk[str(v)] = dv
                
                dv.append(code+extra)
        return att

    def parseStationLine(self, items, fStart = None, fEnd = None):
        stationCode = items[0].strip()
        description = items[1]
        start =  parseDate(items[10])
        if stationCode not in self.stationList:
            raise Exception("Station %s not in station list." % stationCode)
        
        ## Here we can force a different start & End values to the line
        if fStart is not None:
            start = fStart
            
        if fEnd is not None:
            end = fEnd
        
        oui = None
        at = self.get(stationCode)
        #print >>sys.stderr,items, at, file=sys.stderr)
        if not at:
            ## print(" Deriving attributes from description %s " % " ".join(items), file=sys.stderr)
            at = self.__parseDescription__(description)
            if at:
                self.stationAttributeList[stationCode] = at
        else:
            for item in ['Affiliation', 'Country', 'Place']:
                if item in at:
                    continue
                if not oui:
                    ## print(" Deriving attribute (%s) from description %s " % (item, " ".join(items)), file=sys.stderr)
                    oui = self.__parseDescription__(description)
                if item in oui:
                    ## print(" Setting attribute (%s) from description for %s = %s" % (item, stationCode, oui[item]), file=sys.stderr)
                    at[item] = oui[item]
                else:
                    ## print(" Empty %s for %s" % (item, stationCode), file=sys.stderr)
                    pass
        
        country = at['Country'] if 'Country' in at else None
        place =  at['Place'] if 'Place' in at else None
        return [place, country]

    def dump(self, fdo, code):
        if not code:
            att = self.reorder_station_attr()
            for (key,v) in att.iteritems():
                if key in ['Country', 'Place']: continue
                for (value, s) in v.iteritems():
                    fdo.write("Sa: %s=%s" % (key, quote(value)))
                    for station in s:
                            fdo.write(" %s" % (station))
                    fdo.write("\n")
        else:
            at = self.get(code)
            if not at: return
            if 'done' in at: return
            at['done'] = 1 # Mark the item as printed
            for (k,v) in at.iteritems():
                extra = ''
                if k in [ 'done', 'Place', 'Country']: continue
                if k in ['Affiliation']: v = quote(v)

                if k == 'Restricted':
                    extra = ' %s,*,*' % code

                if k == 'restricted_exc':
                    k = 'Restricted'
                    extra=',*,'+str(v)
                    v = (not at['Restricted']) if 'Restricted' in at else True


                fdo.write("Sa: %s=%s %s%s\n" % (k,v,code,extra))

class NetworkAttributes:
    def __build__(self, row):
        #net_code,start,end,restricted,shared,net_class,type,institutions,region,remark
        
        attList = {}
        
        if row['start']:
            self.start = row['start'].strftime("%Y/%j")
            self.startDate = row['start']
            self.hasStart = True
        
        if row['end']:
            self.end = row['end'].strftime("%Y/%j")
            self.endDate = row['end']
            self.hasEnd = True
        
        if row['restricted'] != 0:
            attList['Restricted'] = row['restricted']
        
        if row['shared'] != 1:
            attList['Shared'] = row['shared']
        
        if row['net_class']:
            attList['NetClass'] = row['net_class'].strip()
        
        if row['type']:
            attList['Type'] = row['type'].strip()
        
        if row['institutions']:
            attList['Institutions'] = row['institutions'].strip()
        
        if row['region']:
            attList['Region'] = row['region'].strip()
        
        if row['remark']:
            attList['Remark'] = row['remark'].strip()
        
        self.networkAttributes.update(attList)

    def parseNetworkLine(self, items):
        if len(items) < 4 or len(items) > 6:
            raise Exception("Invalid network line")
        
        attList = {}
        if items[1] == "none":
            attList['Description'] = hummanStr(items[0])
        else:
            attList['Description'] = "%s (%s)" % (hummanStr(items[0]), items[1])
        
        self.networkAttributes.update(attList)

    def dump(self, fdo):
        for (k,v) in self.networkAttributes.iteritems():
            if k in ['Description', 'Remark', 'Region', 'Institutions']:
                v = quote(v)
            fdo.write("Na: %s=%s\n" % (k,v))

    def __init__(self, networkCode, filename):
        self.networkCode = networkCode
        self.networkAttributes = {}

        self.start = None
        self.end = None
        
        self.hasStart = False
        self.hasEnd = False
         
        if not filename: return
        fd = open(filename)
        try:
            try:
                fieldNames = None
                if isPyVersion(2, 3):
                    fieldNames = getFieldNames(fd)

                for row in csv.DictReader(fd, fieldNames):
                    net_code = row['net_code']
                    if net_code != self.networkCode: continue 

                    #del row['net_code']
                    #del row['start']
                    row['start'] = parseDate(row['start'])
                    row['end'] = parseDate(row['end'])
                    row['restricted'] = bool(int(row['restricted']))
                    row['shared'] = bool(int(row['shared']))
                    row['region'] = row['region'].strip()
                    row['remark'] = row['remark'].strip()
                    row['institutions'] = row['institutions'].strip()
                                                                                
                    self.__build__(row)
                    break

            except KeyError as e:
                raise Exception("column %s missing in %s" % (str(e), filename))

            except (TypeError, ValueError) as e:
                raise Exception("error reading %s: %s" % (filename, str(e)))

        finally:
            fd.close()
        print(" found %d Attribute for network %s (%s)" % (len(self.networkAttributes), self.networkCode, filename), file=sys.stderr)
