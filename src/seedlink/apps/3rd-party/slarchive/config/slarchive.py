from __future__ import print_function
import os, string, time, re, glob
import seiscomp3.Kernel, seiscomp3.Config

class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))
    self.archive_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", "archive")
    self.config_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", self.name)
    self.host = "127.0.0.1"
    self.port = 18000
    self.buffer = 1000


  def _readConfig(self):
    self.rc = {}

    cfg = seiscomp3.Config.Config()
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", "defaults", self.name + ".cfg"))
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", self.name + ".cfg"))
    try: cfg.readConfig(os.path.join(os.environ['HOME'], ".seiscomp3", self.name + ".cfg"))
    except: pass
    self.params = dict([(x, ",".join(cfg.getStrings(x))) for x in cfg.names()])

    try: self.host = self.params['address']
    except: self.params['address'] = self.host

    try: self.port = int(self.params['port'])
    except: self.params['port'] = self.port

    try: self.buffer = self.params['buffer']
    except: self.params['buffer'] = self.buffer

    try:
      self.archive_dir = self.params['archive']
      if not os.path.isabs(self.archive_dir):
        self.archive_dir = os.path.join(self.env.SEISCOMP_ROOT, self.archive_dir)
    except: pass
    self.params['archive'] = self.archive_dir

    self.params['slarchive._config_dir'] = self.config_dir
    return cfg


  def _run(self):
    cfg = self._readConfig()

    mymodname = self.name + "_" + self.host + "_" + str(self.port)

    config_file = os.path.join(self.config_dir, self.name + ".streams")
    run_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "run", self.name)

    try: os.makedirs(run_dir)
    except: pass

    try: os.makedirs(self.archive_dir)
    except: pass

    prog = "run_with_lock"
    params = self.env.lockFile(self.name)
    params += " " + self.name + ' -b -x "' + os.path.join(run_dir, mymodname + ".seq") + ':1000000"'
    params += ' -SDS "%s"' % self.archive_dir
    try: params += ' -B %d' % cfg.getInt('buffer')
    except: pass
    try: params += ' -nt %d' % cfg.getInt('networkTimeout')
    except: params += ' -nt 900'
    try: params += ' -nd %d' % cfg.getInt('delay')
    except: pass
    try: params += ' -i %d' % cfg.getInt('idleTimeout')
    except: pass
    try: params += ' -k %d' % cfg.getInt('keepalive')
    except: pass
    params += ' -Fi:1 -Fc:900 -l "%s" %s:%d' % (config_file,self.host,self.port)
    return self.env.start(self.name, prog, params, True)


  def _processStation(self, key_dir, profile):
    if profile:
      station_config_file = "profile_%s" % (profile,)
    else:
      station_config_file = "station_%s_%s" % (self.net, self.sta)

    cfg = seiscomp3.Config.Config()
    cfg.readConfig(os.path.join(key_dir, station_config_file))
    line = self.net + " " + self.sta
    try: line += " " + cfg.getString("selectors")
    except: pass

    keepdays = 30
    try: keepdays = cfg.getInt("keep")
    except: pass

    rc = "STATION='%s'\n" % self.sta + \
         "NET='%s'\n" % self.net + \
         "ARCH_KEEP='%d'\n" % keepdays

    self.rc[self.net + "_" + self.sta] = rc

    return line

  def requiresKernelModules(self):
    return False

  def updateConfig(self):
    self._readConfig()
    template_dir = os.path.join(self.env.SEISCOMP_ROOT, "share", "templates", self.name)

    # Create purge_datafiles script
    tpl_paths = [template_dir]
    purge_script = self.env.processTemplate('purge_datafiles.tpl', tpl_paths, self.params, True)
    if purge_script:
      try: os.makedirs(self.config_dir)
      except: pass
      fd = open(os.path.join(self.config_dir, "purge_datafiles"), "w")
      fd.write(purge_script)
      fd.close()
      os.chmod(os.path.join(self.config_dir, "purge_datafiles"), 0o755)
    else:
      try: os.remove(os.path.join(self.config_dir, "purge_datafiles"))
      except: pass

    rx_binding = re.compile(r'(?P<module>[A-Za-z0-9_\.-]+)(:(?P<profile>[A-Za-z0-9_-]+))?$')

    bindings_dir = os.path.join(self.env.SEISCOMP_ROOT, "etc", "key")
    key_dir = os.path.join(bindings_dir, self.name)
    config_file = os.path.join(self.config_dir, "slarchive.streams")

    # Remove config file
    try: os.remove(config_file)
    except: pass

    config_fd = None
    files = glob.glob(os.path.join(bindings_dir, "station_*"))
    for f in files:
      try:
        (path, net, sta) = f.split('_')[-3:]
        if not path.endswith("station"):
          print("invalid path", f)

      except ValueError:
        print("invalid path", f)
        continue

      self.net = net
      self.sta = sta

      fd = open(f)
      line = fd.readline()
      while line:
        line = line.strip()
        if not line or line[0] == '#':
          line = fd.readline()
          continue

        m = rx_binding.match(line)
        if not m:
          print("invalid binding in %s: %s" % (f, line))
          line = fd.readline()
          continue

        if m.group('module') != self.name:
          line = fd.readline()
          continue

        profile = m.group('profile')
        line = self._processStation(key_dir, profile)
        if line:
          if not config_fd:
            try: os.makedirs(self.config_dir)
            except: pass
            try: config_fd = open(config_file, "w")
            except:
              raise Exception("Error: unable to create slarchive config file '%s'" % config_file)
          config_fd.write("%s\n" % line)
        break

      fd.close()

    # Create rc file
    rc_files = glob.glob(os.path.join(self.config_dir, "rc_*"))
    for (station_id, rc) in self.rc.items():
      fd = open(os.path.join(self.config_dir, "rc_%s" % (station_id,)), "w")
      fd.write(rc)
      fd.close()

    # Clean up unused rc_* files
    for rc in rc_files:
      if os.path.basename(rc)[3:] not in self.rc:
        try: os.remove(rc)
        except: pass

    return 0


  def printCrontab(self):
    print("20 3 * * * %s/purge_datafiles >/dev/null 2>&1" % (self.config_dir))
