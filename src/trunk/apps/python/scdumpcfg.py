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

import sys
import seiscomp3.Client
import seiscomp3.Config


def readParams(sc_params):
    if sc_params.baseID():
        sc_params_base = seiscomp3.DataModel.ParameterSet.Find(
            sc_params.baseID())
        if sc_params_base is None:
            sys.stderr.write("Warning: %s: base parameter set for %s not found\n" % (
                sc_params.baseID(), sc_params.publicID()))
            params = {}
        else:
            params = readParams(sc_params_base)
    else:
        params = {}

    for i in range(sc_params.parameterCount()):
        p = sc_params.parameter(i)
        params[p.name()] = p.value()

    return params


class DumpCfg(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        if argc < 2:
            sys.stderr.write("scdumpcfg {modname} [options]\n")
            raise RuntimeError

        self.appName = argv[1]

        # Remove first parameter to replace appname with passed module name
        argc = argc-1
        argv = argv[1:]

        seiscomp3.Client.Application.__init__(self, argc, argv)

        self.setMessagingEnabled(True)
        self.setMessagingUsername("")
        self.setDatabaseEnabled(True, True)
        self.setLoadConfigModuleEnabled(True)
        self.setDaemonEnabled(False)

    def createCommandLineDescription(self):
        self.commandline().addGroup("Dump")
        self.commandline().addStringOption("Dump", "param,P",
                                           "Specify parameter name to filter for")
        self.commandline().addOption("Dump", "bindings,B",
                                     "Dump bindings instead of module configuration")
        self.commandline().addOption("Dump", "allow-global,G",
                                     "Print global bindings if no module binding is avaible")
        self.commandline().addOption("Dump", "cfg", "Print output in .cfg format")

    def validateParameters(self):
        if not seiscomp3.Client.Application.validateParameters(self):
            return False

        self.dumpBindings = self.commandline().hasOption("bindings")

        try:
            self.param = self.commandline().optionString("param")
        except:
            self.param = None

        self.allowGlobal = self.commandline().hasOption("allow-global")
        self.formatCfg = self.commandline().hasOption("cfg")

        if not self.dumpBindings:
            self.setMessagingEnabled(False)
            self.setDatabaseEnabled(False, False)
            self.setLoadConfigModuleEnabled(False)

        return True

    def initConfiguration(self):
        if self.appName == "-h" or self.appName == "--help":
            self.printUsage()
            return False

        return seiscomp3.Client.Application.initConfiguration(self)

    # Do nothing.
    def initSubscriptions(self):
        return True

    def run(self):
        cfg = self.configuration()

        if not self.dumpBindings:
            symtab = cfg.symbolTable()
            names = cfg.names()
            count = 0
            for name in names:
                if self.param and self.param != name:
                    continue
                sym = symtab.get(name)
                if self.formatCfg:
                    if sym.comment:
                        if count > 0:
                            sys.stdout.write("\n")
                        sys.stdout.write("%s\n" % sym.comment)
                    sys.stdout.write("%s = %s\n" % (sym.name, sym.content))
                else:
                    sys.stdout.write("%s\n" % sym.name)
                    sys.stdout.write("  value(s) : %s\n" %
                                     ", ".join(sym.values))
                    sys.stdout.write("  source   : %s\n" % sym.uri)
                count = count + 1

            if self.param and count == 0:
                sys.stderr.write("%s: definition not found\n." % self.param)
        else:
            cfg = self.configModule()
            if cfg is None:
                sys.stderr.write("No config module read\n")
                return False

            tmp = {}
            for i in range(cfg.configStationCount()):
                cfg_sta = cfg.configStation(i)
                tmp[(cfg_sta.networkCode(), cfg_sta.stationCode())] = cfg_sta

            name = self.name()
            # For backward compatibility rename global to default
            if name == "global":
                name = "default"

            for item in sorted(tmp.keys()):
                cfg_sta = tmp[item]
                sta_enabled = cfg_sta.enabled()
                cfg_setup = seiscomp3.DataModel.findSetup(
                    cfg_sta, name, self.allowGlobal)
                if not cfg_setup is None:
                    suffix = ""
                    if sta_enabled and cfg_setup.enabled():
                        out = "+ "
                    else:
                        suffix = " ("
                        if not sta_enabled:
                            suffix += "station disabled"
                        if not cfg_setup.enabled():
                            if suffix:
                                suffix += ", "
                            suffix += "setup disabled"
                        suffix += ")"
                        out = "- "
                    out += "%s.%s%s\n" % (cfg_sta.networkCode(),
                                          cfg_sta.stationCode(), suffix)
                    params = seiscomp3.DataModel.ParameterSet.Find(
                        cfg_setup.parameterSetID())
                    if params is None:
                        sys.stderr.write(
                            "ERROR: %s: ParameterSet not found\n" % cfg_setup.parameterSetID())
                        return False

                    params = readParams(params)
                    count = 0
                    for param_name in sorted(params.keys()):
                        if self.param and self.param != param_name:
                            continue
                        out += "  %s: %s\n" % (param_name, params[param_name])
                        count = count + 1

                    if count > 0:
                        sys.stdout.write(out)

        return True


try:
    app = DumpCfg(len(sys.argv), sys.argv)
except:
    sys.exit(1)

sys.exit(app())
