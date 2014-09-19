import os, sys, seiscomp3.Kernel


class Module(seiscomp3.Kernel.CoreModule):
  def __init__(self, env):
    seiscomp3.Kernel.CoreModule.__init__(self, env, env.moduleName(__file__))
    # High priority
    self.order = -100
    # This is a config module which writes the setup config to kernel.cfg
    self.isConfigModule = True


  def setup(self, setup_config):
    cfgfile = os.path.join(self.env.SEISCOMP_ROOT, "etc", self.name + ".cfg")

    cfg = seiscomp3.Config.Config()
    cfg.readConfig(cfgfile)
    try: cfg.setString("organization", setup_config.getString("kernel.global.organization"))
    except: cfg.remove("organization")

    cfg.writeConfig()

    return 0
