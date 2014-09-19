#!/usr/bin/env python

import sys, os
import csv 
from optparse import OptionParser

def quote(instr):
    return '"'+instr+'"'

class base(object):
    def __init__(self, filename, fields):
        self.att = {}
        fd = open(filename)
        try:
            try:
                fieldNames = None
                for row in csv.DictReader(fd, fieldNames):
                    id = row['id']
                    if id in self.att:
                        print "multiple %s found in %s" % (id, filename)
                        continue

                    for key in fields:
                        if not row[key]:
                            del(row[key])

                    del row['id']

                    try:
                        row['low_freq'] = float(row['low_freq'])
                    except KeyError:
                        pass

                    try:
                        row['high_freq'] = float(row['high_freq'])
                    except KeyError:
                        pass

                    self.att[id] = row

            except KeyError, e:
                raise Exception("column %s missing in %s" % (str(e), filename))

            except (TypeError, ValueError), e:
                raise Exception("error reading %s: %s" % (filename, str(e)))

        finally:
            fd.close()

    def keys(self):
        return self.att.keys()

    def screname(self, what):
        nc = ""
        nu = True
        for c in what:
            if c == '_':
                nu = True
                continue
            if nu:
                nc += c.upper()
                nu = False 
            else:
                nc += c
                
        if nc == 'LowFreq': nc = 'LowFrequency'
        if nc == 'HighFreq': nc = 'HighFrequency'

        return nc

    def reorder(self):
        att = {}
        if not self.att:
            return None
        
        for (code, row) in self.att.iteritems():
            for (k, v) in row.iteritems():
                k = self.screname(k)
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
                
                dv.append(code)
        return att

    def dump(self, fdo):
        att = self.reorder()
        lastK=None

        for (k, v) in att.iteritems():
            if not lastK: lastK = k
            if lastK != k:
                fdo.write("\n")
            for (kv, ids) in v.iteritems():
                fdo.write("Ia: %s=%s" % (k,quote(kv)))
                for id in ids:
                    fdo.write(" %s" % id)
                fdo.write("\n")
        fdo.write("\n")

class sensorAttributes(base):
    def __init__(self, filename):
        base.__init__(self, filename, ['id', 'type','unit', 'low_freq', 'high_freq', 'model', 'manufacturer', 'remark'])

class dataloggerAttributes(base):
    def __init__(self, filename):
        base.__init__(self, filename, ['id', 'digitizer_model', 'digitizer_manufacturer', 'recorder_model', 'recorder_manufacturer', 'clock_model', 'clock_manufacturer', 'clock_type', 'remark'])

class INST(object):
    def cleanID(self, id):
        nc = ""
        for c in id:
            nc += c
            if c == '_':
                nc = ""
        
        return nc
        
    def __init__(self, filename, attS, attD):
        self.filename = filename
        self.sensorA = sensorAttributes(attS)
        self.dataloggerA = dataloggerAttributes(attD)
        lines = []
        f = open(filename)
        for line in f:
            line = line.strip()
            if not line or line[0] == '#':
                # Add comments line types
                lines.append({ 'content': line, 'type': 'C', 'id': None})
            else:
                (id, line) = line.split(">", 1)
                id = id.strip()
                line = line.strip()
                # Add undefined line types
                lines.append({ 'content': line, 'type': 'U', 'id': id})
        f.close()
        self.lines = lines
        self._filltypes()

    def _filltypes(self):
        for line in self.lines:
            if line['type'] != 'U':  continue
            id = line['id']
            if id.find('_FIR_') != -1:
                line['type']  = 'F'
            elif id.find('Sngl-gain_') != -1:
                line['type'] = 'L'
                line['id'] = self.cleanID(id)
            elif id.find('_digipaz_') != -1:
                line['type'] = 'P'
            elif id.find('_iirpaz_') != -1:
                line['type'] = 'I'
        
        for line in self.lines:
            if line['type'] != 'U':  continue
            id = self.cleanID(line['id'])

            if id in self.sensorA.keys():
                line['type'] = 'S'
                line['id'] = id
            elif id in self.dataloggerA.keys():
                line['type'] = 'D'
                line['id'] = id
            # Those we are forcing !
            elif id in ['OSIRIS-SC',  'Gaia',  'LE24',  'MALI',  'PSS',  'FDL',  'CMG-SAM',  'CMG-DCM',  'EDAS-24',  'SANIAC']:
                line['id'] = id
                line['type'] = 'D'
            elif id in ['Trillium-Compact',  'Reftek-151/120',  'BBVS-60',  'CMG-3ESP/60F',  'LE-1D/1',  'L4-3D/BW',  'S13',  'GS13',  'SH-1',  'MP',  'MARKL22',  'CM-3',  'CMG-6T',  'SM-6/BW']: 
                line['id'] = id
                line['type'] = 'S'

        for line in self.lines:
            if line['type'] == 'U':
                print "'"+self.cleanID(line['id'])+"', ",

    def dump(self, fdo):
        sa = False
        da = False
        
        dataloggerFieldSize = 0
        sensorFieldSize = 0
        for line in self.lines:
            if line['type'] == 'C': continue
            if line['type'] == 'S':
                if len(line['id']) > sensorFieldSize:
                    sensorFieldSize = len(line['id']) 
            if line['type'] == 'D':
                if len(line['id']) > dataloggerFieldSize:
                    dataloggerFieldSize = len(line['id']) 

        seLine = "Se: %%%ss %%s\n" % (-1*(sensorFieldSize+1))
        dtLine = "Dl: %%%ss %%s\n" % (-1*(dataloggerFieldSize+1))
        for line in self.lines:
            if line['type'] == 'C':
                fdo.write(line['content'] + "\n")
                continue

            if line['type'] == 'S':
                if not sa:
                    self.sensorA.dump(fdo)
                    sa = True
                fdo.write(seLine % (line['id'], line['content']))
                continue

            if line['type'] == 'D':
                if not da:
                    self.dataloggerA.dump(fdo)
                    da = True
                fdo.write(dtLine % (line['id'], line['content']))
                continue

            if line['type'] == 'L':
                fdo.write("Cl: %s %s\n" % (line['id'], line['content']))
                continue

            if line['type'] == 'F':
                fdo.write("Ff: %s %s\n" % (line['id'], line['content']))
                continue

            if line['type'] == 'P':
                fdo.write("Pz: %s %s\n" % (line['id'], line['content']))
                continue


            if line['type'] == 'I':
                fdo.write("If: %s %s\n" % (line['id'], line['content']))
                continue

def main():

    parser = OptionParser(usage="Old tab to New tab converter", version="1.0", add_help_option=True)

    parser.add_option("", "--sat", type="string",
                      help="Indicates the sensor attribute file to use", dest="sat", default="sensor_attr.csv")
    parser.add_option("", "--dat", type="string",
                      help="Indicates the station attribute file to use", dest="dat", default="datalogger_attr.csv")
    parser.add_option("-c", "--clean", action="store_true",
                      help="Remove the comments and blank lines", dest="cleanFile", default=False)

    # Parsing & Error check
    (options, args) = parser.parse_args()
    errors = []

    if len(args) != 1:
        errors.append("need an Input filename")

    if not os.path.isfile(options.sat):
        errors.append("sensor attribute file '%s' not found." % options.sat)

    if not os.path.isfile(options.dat):
        errors.append("datalogger attribute file '%s' not found." % options.dat)

    if len(args) == 2 and os.path.isfile(args[1]):
        errors.append("output file already exists, will not overwrite.")

    if errors:
        print >> sys.stderr, "Found error while processing the command line:"
        for error in errors:
            print >> sys.stderr, "  %s" % error
        return 1

    inputName = args[0]
    i= INST(inputName, options.sat, options.dat)
    fdo = sys.stdout if len(args) < 2 else open(args[1],"w")
    i.dump(fdo)
    fdo.close()

if __name__ == "__main__":
    main()
