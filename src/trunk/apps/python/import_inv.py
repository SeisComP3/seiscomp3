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
import os
import subprocess
import glob
import seiscomp3.Client


class Importer(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)

        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(False, False)

        self._args = argv[1:]

    def run(self):
        if len(self._args) == 0:
            sys.stderr.write(
                "Usage: import_inv [{format}|help] input output\n")
            return False

        if self._args[0] == "help":
            if len(self._args) < 2:
                sys.stderr.write("'help' can only be used with 'formats'\n")
                sys.stderr.write("import_inv help formats\n")
                return False

            if self._args[1] == "formats":
                return self.printFormats()

            sys.stderr.write("unknown topic '%s'\n" % self._args[1])
            return False

        fmt = self._args[0]
        try:
            prog = os.path.join(
                os.environ['SEISCOMP_ROOT'], "bin", "%s2inv" % fmt)
        except:
            sys.stderr.write(
                "Could not get SeisComP3 root path, SEISCOMP_ROOT not set?\n")
            return False

        if not os.path.exists(prog):
            sys.stderr.write("Format '%s' is not supported\n" % fmt)
            return False

        if len(self._args) < 2:
            sys.stderr.write("Input missing\n")
            return False

        input = self._args[1]

        if len(self._args) < 3:
            filename = os.path.basename(os.path.abspath(input))
            if not filename:
                filename = fmt

            # Append .xml if the ending is not already .xml
            if filename[-4:] != ".xml":
                filename = filename + ".xml"
            storage_dir = os.path.join(
                os.environ['SEISCOMP_ROOT'], "etc", "inventory")
            output = os.path.join(storage_dir, filename)
            try:
                os.makedirs(storage_dir)
            except:
                pass
            sys.stderr.write("Generating output to %s\n" % output)
        else:
            output = self._args[2]

        proc = subprocess.Popen([prog, input, output],
                                stdout=None, stderr=None, shell=False)
        chans = proc.communicate()
        if proc.returncode != 0:
            sys.stderr.write(
                "Conversion failed, return code: %d\n" % proc.returncode)
            return False

        return True

    def printFormats(self):
        try:
            path = os.path.join(os.environ['SEISCOMP_ROOT'], "bin", "*2inv")
        except:
            sys.stderr.write(
                "Could not get SeisComP3 root path, SEISCOMP_ROOT not set?\n")
            return False

        files = glob.glob(path)
        for f in files:
            prog = os.path.basename(f)
            prog = prog[:prog.find("2inv")]
            sys.stdout.write("%s\n" % prog)

        return True


if __name__ == "__main__":
    app = Importer(len(sys.argv), sys.argv)
    sys.exit(app())
