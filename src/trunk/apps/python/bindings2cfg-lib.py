import os
import time
import sys
import seiscomp3.System
import seiscomp3.Client


def collectParams(container):
    params = {}
    for i in range(container.groupCount()):
        params.update(collectParams(container.group(i)))
    for i in range(container.structureCount()):
        params.update(collectParams(container.structure(i)))
    for i in range(container.parameterCount()):
        p = container.parameter(i)
        if p.symbol.stage == seiscomp3.System.Environment.CS_UNDEFINED:
            continue
        params[p.variableName] = ",".join(p.symbol.values)

    return params


def collect(idset, paramSetID):
    paramSet = seiscomp3.DataModel.ParameterSet.Find(paramSetID)
    if not paramSet:
        return
    idset[paramSet.publicID()] = 1
    if not paramSet.baseID():
        return
    collect(idset, paramSet.baseID())


def sync(paramSet, params):
    obsoleteParams = []
    seenParams = {}
    i = 0
    while i < paramSet.parameterCount():
        p = paramSet.parameter(i)
        if p.name() in params:
            if p.name() in seenParams:
                # Multiple parameter definitions with same name
                sys.stderr.write(
                    "- %s:%s / duplicate parameter name\n" % (p.publicID(), p.name()))
                p.detach()
                continue
            seenParams[p.name()] = 1
            val = params[p.name()]
            if val != p.value():
                p.setValue(val)
                p.update()
        else:
            obsoleteParams.append(p)
        i = i + 1

    for p in obsoleteParams:
        p.detach()

    for key, val in params.items():
        if key in seenParams:
            continue
        p = seiscomp3.DataModel.Parameter.Create()
        p.setName(key)
        p.setValue(val)
        paramSet.add(p)


class ConfigDBUpdater(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        self.setLoggingToStdErr(True)
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
        self.setMessagingUsername("_sccfgupd_")
        self.setLoadConfigModuleEnabled(True)
        # Load all configuration modules
        self.setConfigModuleName("")
        self.setPrimaryMessagingGroup(
            seiscomp3.Communication.Protocol.LISTENER_GROUP)

        self._outputFile = ""
        self._keyDir = None

    def createCommandLineDescription(self):
        self.commandline().addGroup("Input")
        self.commandline().addStringOption("Input", "key-dir",
                                           "Overrides the location of the default key directory ($SEISCOMP_ROOT/etc/key)")
        self.commandline().addGroup("Output")
        self.commandline().addStringOption("Output", "output,o",
                                           "If given, an output XML file is generated")

    def validateParameters(self):
        try:
            self._outputFile = self.commandline().optionString("output")
            # Switch to offline mode
            self.setMessagingEnabled(False)
            self.setDatabaseEnabled(False, False)
        except:
            pass

        try:
            self._keyDir = self.commandline().optionString("key-dir")
        except:
            pass

        return True

    def init(self):
        if not seiscomp3.Client.Application.init(self):
            return False

        # Initialize the basic directories
        filebase = seiscomp3.System.Environment.Instance().installDir()
        descdir = os.path.join(filebase, "etc", "descriptions")

        # Load definitions of the configuration schema
        defs = seiscomp3.System.SchemaDefinitions()
        if not defs.load(descdir):
            sys.stderr.write("Error: could not read descriptions\n")
            return False

        if defs.moduleCount() == 0:
            sys.stderr.write("Warning: no modules defined, nothing to do\n")
            return False

        # Create a model from the schema and read its configuration including
        # all bindings.
        model = seiscomp3.System.Model()
        if self._keyDir:
            model.keyDirOverride = self._keyDir
        model.create(defs)
        model.readConfig()

        # Find all binding mods for trunk. Bindings of modules where standalone
        # is set to true are ignored. They are supposed to handle their bindings
        # on their own.
        self.bindingMods = []
        for i in range(defs.moduleCount()):
            mod = defs.module(i)
            # Ignore stand alone modules (eg seedlink, slarchive, ...) as they
            # are not using the trunk libraries and don't need database
            # configurations
            if mod.isStandalone():
                continue
            self.bindingMods.append(mod.name)

        if len(self.bindingMods) == 0:
            sys.stderr.write(
                "Warning: no usable modules found, nothing to do\n")
            return False

        self.stationSetups = {}

        # Read bindings
        for m in self.bindingMods:
            mod = model.module(m)
            if not mod:
                sys.stderr.write("Warning: module %s not assigned\n" % m)
                continue
            if len(mod.bindings) == 0:
                continue

            # Rename global to default for being compatible with older
            # releases
            if m == "global":
                m = "default"

            sys.stderr.write("+ %s\n" % m)
            for staid in mod.bindings.keys():
                binding = mod.getBinding(staid)
                if not binding:
                    continue
                # sys.stderr.write("  + %s.%s\n" % (staid.networkCode, staid.stationCode))
                params = {}
                for i in range(binding.sectionCount()):
                    params.update(collectParams(binding.section(i)))
                key = (staid.networkCode, staid.stationCode)
                if not key in self.stationSetups:
                    self.stationSetups[key] = {}
                self.stationSetups[key][m] = params
            sys.stderr.write("  + read %d stations\n" %
                             len(mod.bindings.keys()))

        return True

    def send(self, *args):
        '''
        A simple wrapper that sends a message and tries to resend it in case of
        an error.
        '''
        while not self.connection().send(*args):
            sys.stderr.write("Warning: sending failed, retrying\n")
            time.sleep(1)

    def run(self):
        '''
        Reimplements the main loop of the application. This methods collects
        all bindings and updates the database. It searches for already existing
        objects and updates them or creates new objects. Objects that is didn't
        touched are removed. This tool is the only one that should writes the
        configuration into the database and thus manages the content.
        '''
        config = seiscomp3.Client.ConfigDB.Instance().config()
        if config is None:
            config = seiscomp3.DataModel.Config()

        configMod = None
        obsoleteConfigMods = []

        if not self._outputFile:
            moduleName = self.name()
            seiscomp3.DataModel.Notifier.Enable()
        else:
            moduleName = "trunk"

        configID = "Config/%s" % moduleName

        for i in range(config.configModuleCount()):
            if config.configModule(i).publicID() != configID:
                obsoleteConfigMods.append(config.configModule(i))
            else:
                configMod = config.configModule(i)

        # Remove obsolete config modules
        for cm in obsoleteConfigMods:
            sys.stderr.write(
                "- %s / obsolete module configuration\n" % cm.name())
            ps = seiscomp3.DataModel.ParameterSet.Find(cm.parameterSetID())
            if not ps is None:
                ps.detach()
            cm.detach()
        del obsoleteConfigMods

        if not configMod:
            configMod = seiscomp3.DataModel.ConfigModule.Find(configID)
            if configMod is None:
                configMod = seiscomp3.DataModel.ConfigModule.Create(configID)
                config.add(configMod)
            else:
                if configMod.name() != moduleName:
                    configMod.update()
                if not configMod.enabled():
                    configMod.update()

            configMod.setName(moduleName)
            configMod.setEnabled(True)
        else:
            if configMod.name() != moduleName:
                configMod.setName(moduleName)
                configMod.update()
            paramSet = seiscomp3.DataModel.ParameterSet.Find(
                configMod.parameterSetID())
            if configMod.parameterSetID():
                configMod.setParameterSetID("")
                configMod.update()

            if not paramSet is None:
                paramSet.detach()

        stationConfigs = {}
        obsoleteStationConfigs = []

        for i in range(configMod.configStationCount()):
            cs = configMod.configStation(i)
            if (cs.networkCode(), cs.stationCode()) in self.stationSetups:
                stationConfigs[(cs.networkCode(), cs.stationCode())] = cs
            else:
                obsoleteStationConfigs.append(cs)

        for cs in obsoleteStationConfigs:
            sys.stderr.write("- %s/%s/%s / obsolete station configuration\n" %
                             (configMod.name(), cs.networkCode(), cs.stationCode()))
            cs.detach()
        del obsoleteStationConfigs

        for staid, setups in self.stationSetups.items():
            try:
                cs = stationConfigs[staid]
            except:
                cs = seiscomp3.DataModel.ConfigStation.Find(
                    "Config/%s/%s/%s" % (configMod.name(), staid[0], staid[1]))
                if not cs:
                    cs = seiscomp3.DataModel.ConfigStation.Create(
                        "Config/%s/%s/%s" % (configMod.name(), staid[0], staid[1]))
                    configMod.add(cs)
                cs.setNetworkCode(staid[0])
                cs.setStationCode(staid[1])
                cs.setEnabled(True)

                ci = seiscomp3.DataModel.CreationInfo()
                ci.setCreationTime(seiscomp3.Core.Time.GMT())
                ci.setAgencyID(self.agencyID())
                ci.setAuthor(self.name())
                cs.setCreationInfo(ci)

            stationSetups = {}
            obsoleteSetups = []
            for i in range(cs.setupCount()):
                setup = cs.setup(i)
                if setup.name() in setups:
                    stationSetups[setup.name()] = setup
                else:
                    obsoleteSetups.append(setup)

            for s in obsoleteSetups:
                sys.stderr.write("- %s/%s/%s/%s / obsolete station setup\n" %
                                 (configMod.name(), cs.networkCode(), cs.stationCode(), setup.name()))
                ps = seiscomp3.DataModel.ParameterSet.Find(s.parameterSetID())
                if ps:
                    ps.detach()
                s.detach()
            del obsoleteSetups

            newParamSets = {}
            globalSet = ""
            for mod, params in setups.items():
                try:
                    setup = stationSetups[mod]
                except:
                    setup = seiscomp3.DataModel.Setup()
                    setup.setName(mod)
                    setup.setEnabled(True)
                    cs.add(setup)

                paramSet = seiscomp3.DataModel.ParameterSet.Find(
                    setup.parameterSetID())
                if not paramSet:
                    paramSet = seiscomp3.DataModel.ParameterSet.Find("ParameterSet/%s/Station/%s/%s/%s" % (
                        configMod.name(), cs.networkCode(), cs.stationCode(), setup.name()))
                    if not paramSet:
                        paramSet = seiscomp3.DataModel.ParameterSet.Create(
                            "ParameterSet/%s/Station/%s/%s/%s" % (configMod.name(), cs.networkCode(), cs.stationCode(), setup.name()))
                        config.add(paramSet)
                    paramSet.setModuleID(configMod.publicID())
                    paramSet.setCreated(seiscomp3.Core.Time.GMT())
                    newParamSets[paramSet.publicID()] = 1
                    setup.setParameterSetID(paramSet.publicID())
                    if mod in stationSetups:
                        setup.update()
                elif paramSet.moduleID() != configMod.publicID():
                    paramSet.setModuleID(configMod.publicID())
                    paramSet.update()

                # Synchronize existing parameterset with the new parameters
                sync(paramSet, params)

                if setup.name() == "default":
                    globalSet = paramSet.publicID()

            for i in range(cs.setupCount()):
                setup = cs.setup(i)
                paramSet = seiscomp3.DataModel.ParameterSet.Find(
                    setup.parameterSetID())
                if not paramSet:
                    continue

                if paramSet.publicID() != globalSet and paramSet.baseID() != globalSet:
                    paramSet.setBaseID(globalSet)
                    if not paramSet.publicID() in newParamSets:
                        paramSet.update()

        # Collect unused ParameterSets
        usedSets = {}
        for i in range(config.configModuleCount()):
            configMod = config.configModule(i)
            for j in range(configMod.configStationCount()):
                cs = configMod.configStation(j)
                for k in range(cs.setupCount()):
                    setup = cs.setup(k)
                    collect(usedSets, setup.parameterSetID())

        # Delete unused ParameterSets
        i = 0
        while i < config.parameterSetCount():
            paramSet = config.parameterSet(i)
            if not paramSet.publicID() in usedSets:
                sys.stderr.write("- %s / obsolete parameter set\n" %
                                 paramSet.publicID())
                paramSet.detach()
            else:
                i = i + 1

        # Generate output file and exit if configured
        if self._outputFile:
            ar = seiscomp3.IO.XMLArchive()
            if not ar.create(self._outputFile):
                sys.stderr.write(
                    "Failed to created output file: %s" % self._outputFile)
                return False

            ar.setFormattedOutput(True)
            ar.writeObject(config)
            ar.close()
            return True

        ncount = seiscomp3.DataModel.Notifier.Size()
        if ncount > 0:
            sys.stderr.write("+ synchronize %d change" % ncount)
            if ncount > 1:
                sys.stderr.write("s")
            sys.stderr.write("\n")
        else:
            sys.stderr.write("- database is already up-to-date\n")
            return True

        cfgmsg = seiscomp3.DataModel.ConfigSyncMessage(False)
        cfgmsg.setCreationInfo(seiscomp3.DataModel.CreationInfo())
        cfgmsg.creationInfo().setCreationTime(seiscomp3.Core.Time.GMT())
        cfgmsg.creationInfo().setAuthor(self.author())
        cfgmsg.creationInfo().setAgencyID(self.agencyID())
        self.send(seiscomp3.Communication.Protocol.STATUS_GROUP, cfgmsg)

        # Send messages in a batch of 100 notifiers to not exceed the
        # maximum allowed message size of ~300kb.
        msg = seiscomp3.DataModel.NotifierMessage()
        nmsg = seiscomp3.DataModel.Notifier.GetMessage(False)
        count = 0
        sys.stderr.write("\r  + sending notifiers: %d%%" % (count * 100 / ncount))
        sys.stderr.flush()
        while nmsg:
            for o in nmsg:
                n = seiscomp3.DataModel.Notifier.Cast(o)
                if n:
                    msg.attach(n)

            if msg.size() >= 100:
                count += msg.size()
                self.send("CONFIG", msg)
                msg.clear()
                self.sync()
                sys.stderr.write("\r  + sending notifiers: %d%%" %
                                 (count * 100 / ncount))
                sys.stderr.flush()

            nmsg = seiscomp3.DataModel.Notifier.GetMessage(False)

        if msg.size() > 0:
            count += msg.size()
            self.send("CONFIG", msg)
            msg.clear()
            self.sync()
            sys.stderr.write("\r  + sending notifiers: %d%%" %
                             (count * 100 / ncount))
            sys.stderr.flush()

        sys.stderr.write("\n")

        # Notify about end of synchronization
        cfgmsg.creationInfo().setCreationTime(seiscomp3.Core.Time.GMT())
        cfgmsg.isFinished = True
        self.send(seiscomp3.Communication.Protocol.STATUS_GROUP, cfgmsg)

        return True


def main():
    app = ConfigDBUpdater(len(sys.argv), sys.argv)
    return app()
