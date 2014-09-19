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

import sys, os, subprocess, glob
import seiscomp3.Client

class Importer(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)

        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(False, False)

        self._args = argv[1:]


    def run(self):
        if len(self._args) == 0:
            print >> sys.stderr, "Usage: import_inv [{format}|help] input output"
            return False

        if self._args[0] == "help":
            if len(self._args) < 2:
                print >> sys.stderr, "'help' can only be used with 'formats'"
                print >> sys.stderr, "import_inv help formats"
                return False

            if self._args[1] == "formats":
                return self.printFormats()

            print >> sys.stderr, "unknown topic '%s'" % self._args[1]
            return False

        format = self._args[0]
        try: prog = os.path.join(os.environ['SEISCOMP_ROOT'], "bin", "%s2inv" % format)
        except:
            print >> sys.stderr, "Could not get SeisComP3 root path, SEISCOMP_ROOT not set?"
            return False

        if not os.path.exists(prog):
            print >> sys.stderr, "Format '%s' is not supported" % format
            return False

        if len(self._args) < 2:
            print >> sys.stderr, "Input missing"
            return False

        input = self._args[1]

        if len(self._args) < 3:
            filename = os.path.basename(os.path.abspath(input))
            if not filename:
                filename = format;

            # Append .xml if the ending is not already .xml
            if filename[-4:] != ".xml": filename = filename + ".xml"
            storage_dir = os.path.join(os.environ['SEISCOMP_ROOT'], "etc", "inventory")
            output = os.path.join(storage_dir, filename)
            try: os.makedirs(storage_dir)
            except: pass
            print >> sys.stderr, "Generating output to %s" % output
        else:
            output = self._args[2]

        proc = subprocess.Popen([prog, input, output], stdout=None, stderr=None, shell=False)
        chans = proc.communicate()
        if proc.returncode != 0:
            print >> sys.stderr, "Conversion failed, return code: %d" % proc.returncode
            return False

        return True


    def printFormats(self):
        try: path = os.path.join(os.environ['SEISCOMP_ROOT'], "bin", "*2inv")
        except:
            print >> sys.stderr, "Could not get SeisComP3 root path, SEISCOMP_ROOT not set?"
            return False

        files = glob.glob(path)
        for f in files:
            prog = os.path.basename(f)
            prog = prog[:prog.find("2inv")]
            print prog

        return True


if __name__ == "__main__":
    app = Importer(len(sys.argv), sys.argv)
    sys.exit(app())
