import os, glob, time, sys
import seiscomp3.Kernel, seiscomp3.Client


def collectParams(container):
  params = {}
  for i in range(container.groupCount()):
    params.update(collectParams(container.group(i)))
  for i in range(container.structureCount()):
    params.update(collectParams(container.structure(i)))
  for i in range(container.parameterCount()):
    p = container.parameter(i)
    if p.symbol.stage == seiscomp3.System.Environment.CS_UNDEFINED: continue
    params[p.variableName] = ",".join(p.symbol.values)

  return params


def sync(paramSet, params):
  obsoleteParams = []
  seenParams = {}
  for i in range(paramSet.parameterCount()):
    p = paramSet.parameter(i)
    if p.name() in params:
      seenParams[p.name()] = 1
      val = params[p.name()]
      if val != p.value():
        p.setValue(val)
        p.update()
    else:
      obsoleteParams.append(p)

  for p in obsoleteParams:
    p.detach()

  for key,val in params.items():
    if key in seenParams: continue
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
    self.setPrimaryMessagingGroup(seiscomp3.Communication.Protocol.LISTENER_GROUP)

  def init(self):
    # Initialize the basic directories
    filebase = seiscomp3.System.Environment.Instance().installDir()
    descdir = os.path.join(filebase, "etc", "descriptions")
    keydir = os.path.join(filebase, "etc", "key", self.name())

    # Load definitions of the configuration schema
    defs = seiscomp3.System.SchemaDefinitions()
    if defs.load(descdir) == False:
      sys.stdout.write("Error: could not read descriptions\n")
      return False

    if defs.moduleCount() == 0:
      sys.stdout.write("Warning: no modules defined, nothing to do\n")
      return False

    # Create a model from the schema and read its configuration including
    # all bindings.
    model = seiscomp3.System.Model()
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
      if mod.isStandalone(): continue
      self.bindingMods.append(mod.name)

    if len(self.bindingMods) == 0:
      sys.stdout.write("Warning: no usable modules found, nothing to do\n")
      return False

    self.stationSetups = {}

    # Read bindings
    for m in self.bindingMods:
      mod = model.module(m)
      if not mod:
        sys.stdout.write("Warning: module %s not assigned\n" % m)
        continue
      if len(mod.bindings) == 0: continue

      # Rename global to default for being compatible with older
      # releases
      if m == "global": m = "default"

      sys.stdout.write("+ %s\n" % m)
      sys.stdout.write("  - reading stations")
      sys.stdout.flush()
      for staid in mod.bindings.keys():
        binding = mod.getBinding(staid)
        if not binding: continue
        sys.stdout.write("\r  + %s.%s" % (staid.networkCode, staid.stationCode))
        sys.stdout.flush()
        params = {}
        for i in range(binding.sectionCount()):
            params.update(collectParams(binding.section(i)))
        key = (staid.networkCode, staid.stationCode)
        if not key in self.stationSetups:
          self.stationSetups[key] = {}
        self.stationSetups[key][m] = params
      sys.stdout.write("\r  + read %d stations\n" % len(mod.bindings.keys()))

    return seiscomp3.Client.Application.init(self)


  def send(self, *args):
    '''
    A simple wrapper that sends a message and tries to resend it in case of
    an error.
    '''
    while not self.connection().send(*args):
      sys.stdout.write("Warning: sending failed, retrying\n")
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

    seiscomp3.DataModel.Notifier.Enable()

    configID = "Config/%s" % self.name()

    for i in range(config.configModuleCount()):
      if config.configModule(i).publicID() != configID:
        obsoleteConfigMods.append(config.configModule(i))
      else:
        configMod = config.configModule(i)

    # Remove obsolete config modules
    for cm in obsoleteConfigMods:
      sys.stdout.write("- %s / obsolete module configuration\n" % cm.name())
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
        if configMod.name() != self.name(): configMod.update()
        if not configMod.enabled(): configMod.update()

      configMod.setName(self.name())
      configMod.setEnabled(True)
    else:
      if configMod.name() != self.name():
        configMod.setName(self.name())
        configMod.update()
      paramSet = seiscomp3.DataModel.ParameterSet.Find(configMod.parameterSetID())
      if configMod.parameterSetID():
        configMod.setParameterSetID("")
        configMod.update()

      if not paramSet is None: paramSet.detach()


    stationConfigs = {}
    obsoleteStationConfigs = []

    for i in range(configMod.configStationCount()):
      cs = configMod.configStation(i)
      if (cs.networkCode(), cs.stationCode()) in self.stationSetups:
        stationConfigs[(cs.networkCode(), cs.stationCode())] = cs
      else:
        obsoleteStationConfigs.append(cs)

    for cs in obsoleteStationConfigs:
      sys.stdout.write("- %s/%s/%s / obsolete station configuration\n" % (configMod.name(), cs.networkCode(), cs.stationCode()))
      cs.detach()
    del obsoleteStationConfigs

    for staid, setups in self.stationSetups.items():
      try: cs = stationConfigs[staid]
      except:
        cs = seiscomp3.DataModel.ConfigStation.Find("Config/%s/%s/%s" % (configMod.name(), staid[0], staid[1]))
        if not cs:
          cs = seiscomp3.DataModel.ConfigStation.Create("Config/%s/%s/%s" % (configMod.name(), staid[0], staid[1]))
          configMod.add(cs)
        cs.setNetworkCode(staid[0])
        cs.setStationCode(staid[1])
        cs.setEnabled(True)

      stationSetups = {}
      obsoleteSetups = []
      for i in range(cs.setupCount()):
        setup = cs.setup(i)
        if setup.name() in setups:
          stationSetups[setup.name()] = setup
        else:
          obsoleteSetups.append(setup)

      for s in obsoleteSetups:
        sys.stdout.write("- %s/%s/%s/%s / obsolete station setup\n" % (configMod.name(), cs.networkCode(), cs.stationCode(), setup.name()))
        ps = seiscomp3.DataModel.ParameterSet.Find(s.parameterSetID())
        if ps: ps.detach()
        s.detach()
      del obsoleteSetups

      newParamSets = {}
      globalSet = ""
      for mod, params in setups.items():
        try: setup = stationSetups[mod]
        except:
          setup = seiscomp3.DataModel.Setup()
          setup.setName(mod)
          setup.setEnabled(True)
          cs.add(setup)

        paramSet = seiscomp3.DataModel.ParameterSet.Find(setup.parameterSetID())
        if not paramSet:
          paramSet = seiscomp3.DataModel.ParameterSet.Find("ParameterSet/%s/Station/%s/%s/%s" % (configMod.name(), cs.networkCode(), cs.stationCode(), setup.name()))
          if not paramSet:
            paramSet = seiscomp3.DataModel.ParameterSet.Create("ParameterSet/%s/Station/%s/%s/%s" % (configMod.name(), cs.networkCode(), cs.stationCode(), setup.name()))
            config.add(paramSet)
          paramSet.setModuleID(configMod.publicID())
          paramSet.setCreated(seiscomp3.Core.Time.GMT())
          newParamSets[paramSet.publicID()] = 1
          setup.setParameterSetID(paramSet.publicID())
          if mod in stationSetups: setup.update()
        elif paramSet.moduleID() != configMod.publicID():
          paramSet.setModuleID(configMod.publicID())
          paramSet.update()

        # Synchronize existing parameterset with the new parameters
        sync(paramSet, params)

        if setup.name() == "default": globalSet = paramSet.publicID()

      for i in range(cs.setupCount()):
        setup = cs.setup(i)
        paramSet = seiscomp3.DataModel.ParameterSet.Find(setup.parameterSetID())
        if not paramSet: continue

        if paramSet.publicID() != globalSet and paramSet.baseID() != globalSet:
          paramSet.setBaseID(globalSet)
          if not paramSet.publicID() in newParamSets:
            paramSet.update()

    ncount = seiscomp3.DataModel.Notifier.Size()
    if ncount > 0:
      sys.stdout.write("+ synchronize %d change" % ncount)
      if ncount > 1: sys.stdout.write("s")
      sys.stdout.write("\n")
    else:
      sys.stdout.write("- database is already up-to-date\n")
      return True

    #nmsg = seiscomp3.DataModel.Notifier.GetMessage(True)
    #ar = seiscomp3.IO.XMLArchive()
    #ar.create("-")
    #ar.setFormattedOutput(True)
    #ar.writeObject(nmsg)

    #return True

    # Send messages in a batch of 100 notifiers to not exceed the
    # maximum allowed message size of ~300kb.
    msg = seiscomp3.DataModel.NotifierMessage()
    nmsg = seiscomp3.DataModel.Notifier.GetMessage(False)
    count = 0
    sys.stdout.write("\r  + sending notifiers: %d%%" % (count*100/ncount))
    sys.stdout.flush()
    while nmsg:
      for o in nmsg:
        n = seiscomp3.DataModel.Notifier.Cast(o)
        if n: msg.attach(n)

      if msg.size() >= 100:
        count += msg.size()
        self.send("CONFIG", msg)
        msg.clear()
        self.sync()
        sys.stdout.write("\r  + sending notifiers: %d%%" % (count*100/ncount))
        sys.stdout.flush()

      nmsg = seiscomp3.DataModel.Notifier.GetMessage(False)

    if msg.size() > 0:
      count += msg.size()
      self.send("CONFIG", msg)
      msg.clear()
      self.sync()
      sys.stdout.write("\r  + sending notifiers: %d%%" % (count*100/ncount))
      sys.stdout.flush()

    sys.stdout.write("\n")

    return True



class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))
    # This is a config module which synchronizes bindings with the database
    self.isConfigModule = True

  def updateConfig(self):
    messaging = True
    messagingPort = 4803

    try: messaging = self.env.getBool("messaging.enable")
    except: pass
    try: messagingPort = self.env.getInt("messaging.port")
    except: pass

    # If messaging is disabled in kernel.cfg, do not do anything
    if not messaging:
      sys.stdout.write("- messaging disabled, nothing to do\n")
      return 0

    # Synchronize database configuration
    params = [self.name, '--console', '1', '-H', 'localhost:%d' % messagingPort]
    # Create the database update app and run it
    # This app implements a seiscomp3.Client.Application and connects
    # to localhost regardless of connections specified in global.cfg to
    # prevent updating a remote installation by accident.
    app = ConfigDBUpdater(len(params), params)
    return app()

  def setup(self, setup_config):
    cfgfile = os.path.join(self.env.SEISCOMP_ROOT, "etc", "global.cfg")

    cfg = seiscomp3.Config.Config()
    cfg.readConfig(cfgfile)
    try: cfg.setString("datacenterID", setup_config.getString("global.meta.datacenterID"))
    except: cfg.remove("datacenterID")

    try: cfg.setString("agencyID", setup_config.getString("global.meta.agencyID"))
    except: cfg.remove("agencyID")

    try: cfg.setString("organization", setup_config.getString("global.meta.organization"))
    except: cfg.remove("organization")

    cfg.writeConfig()

    return 0


if __name__ == "__main__":
    import seiscomp3.System
    app = ConfigDBUpdater(len(sys.argv), sys.argv)
    sys.exit(app())
