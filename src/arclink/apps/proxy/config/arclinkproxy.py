import os, string, time, re, glob
import seiscomp3.Kernel, seiscomp3.Config, seiscomp3.System

class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))
    self.archive_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", "archive")
    self.config_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", self.name)

  def _readConfig(self):
    self.rc = {}

    cfg = seiscomp3.Config.Config()

    # Defaults Global + App Cfg
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", "defaults", "global.cfg"))
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", "defaults", self.name + ".cfg"))

    # Config Global + App Cfg
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", "global.cfg"))
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", self.name + ".cfg"))

    # User Global + App Cfg
    cfg.readConfig(os.path.join(os.environ['HOME'], ".seiscomp3", "global.cfg"))
    cfg.readConfig(os.path.join(os.environ['HOME'], ".seiscomp3", self.name + ".cfg"))

    try:
      self.archive_dir = cfg.getString('archive')
      self.archive_dir = self.archive_dir.replace("@ROOTDIR@", self.env.SEISCOMP_ROOT)

      if not os.path.isabs(self.archive_dir):
        self.archive_dir = os.path.join(self.env.SEISCOMP_ROOT, self.archive_dir)
    except: pass

    return cfg

  def _run(self):
    cfg = self._readConfig()
    prog = "run_with_lock"
    params = self.env.lockFile(self.name) + ' ' + self.env.binaryFile(self.name)
    try:
        params += ' -N "%s"' % cfg.getString('nrt')
        params += ' -A %s' % self.archive_dir
    except:
        params += ' -N %s' % self.archive_dir
    try: params += ' -I "%s"' % cfg.getString('iso')
    except: pass
    try: params += ' -P %d' % cfg.getInt('port')
    except: pass
    try: params += ' -O "%s"' % cfg.getString('organization')
    except: pass
    try: params += ' -d "%s"' % cfg.getString('datacenterID')
    except: pass
    try: params += ' -S %d' % cfg.getInt('maxSessions')
    except: pass
    try: params += ' -Q %d' % cfg.getInt('maxQueued')
    except: pass
    try: params += ' -U %d' % cfg.getInt('maxQueuedPerUser')
    except: pass
    try: params += ' -X %d' % cfg.getInt('maxExecuting')
    except: pass
    try: params += ' -L %d' % cfg.getInt('maxLines')
    except: pass
    try: params += ' -T %d' % cfg.getInt('maxAge')
    except: pass
    try: params += ' -t %d' % cfg.getInt('socketTimeout')
    except: pass
    try: params += ' -R %d' % cfg.getInt('downloadRetries')
    except: pass
    try: params += ' -a %s' % cfg.getString('arclinkAddress')
    except: pass
    try: params += ' -p' * cfg.getBool('disableRouting')
    except: pass
    try: params += ' -l' * cfg.getBool('localOnly')
    except: params += ' -l'
    return self.env.start(self.name, prog, params, True)

