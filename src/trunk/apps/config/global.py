import os, glob, time, sys
import seiscomp3.Kernel, seiscomp3.bindings2cfg


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
    app = seiscomp3.bindings2cfg.ConfigDBUpdater(len(params), params)
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
    app = seiscomp3.bindings2cfg.ConfigDBUpdater(len(sys.argv), sys.argv)
    sys.exit(app())
