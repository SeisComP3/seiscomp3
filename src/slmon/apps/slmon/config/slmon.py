from __future__ import print_function
import os, string, time, re, glob
import seiscomp3.Kernel, seiscomp3.Config

class Module(seiscomp3.Kernel.Module):
  def __init__(self, env):
    seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))
    self.config_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", self.name)
    self.rc_dir = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", "rc")


  def _readConfig(self):
    self.rc = {}

    cfg = seiscomp3.Config.Config()
    cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", "defaults", self.name + ".cfg"))
    try: cfg.readConfig(os.path.join(self.env.SEISCOMP_ROOT, "etc", self.name + ".cfg"))
    except: pass
    try: cfg.readConfig(os.path.join(os.environ['HOME'], ".seiscomp3", self.name + ".cfg"))
    except: pass

    self.params = dict([(x, ",".join(cfg.getStrings(x))) for x in cfg.names()])

    try: self.params['title']
    except: self.params['title'] = "SeedLink Monitor"

    try: self.params['refresh']
    except: self.params['refresh'] = "180"
    
    try: self.params['address']
    except: self.params['address'] = "127.0.0.1"

    try: int(self.params['port'])
    except: self.params['port'] = 18000

    try: self.params['email']
    except: self.params['email'] = ""

    try: self.params['wwwdir'] = self.params['wwwdir'].replace("@ROOTDIR@", self.env.SEISCOMP_ROOT).replace("@NAME@", self.name)
    except: self.params['wwwdir'] = os.path.join(self.env.SEISCOMP_ROOT, "var", "run", "slmon")

    # yet to be implemente correctly:
    # live seismograms, lin  in footer:
    try: self.params['liveurl']
    except: self.params['liveurl'] = "http://geofon.gfz-potsdam.de/waveform/liveseis.php?station=%s"
    
    # favicon:
    try: self.params['icon']
    except: self.params['icon'] = "http://www.gfz-potsdam.de/favicon.ico"
    
    # link name to external site in footer
    try: self.params['linkname']
    except: self.params['linkname'] = "GEOFON"
    
    # link to external site in footer
    try: self.params['linkurl']
    except: self.params['linkurl'] = "http://www.gfz-potsdam.de/geofon/"

    return cfg


  def _run(self):
    station_file = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", self.name, "stations.ini")
    config_file = os.path.join(self.env.SEISCOMP_ROOT, "var", "lib", self.name, "config.ini")

    prog = "run_with_lock"
    params = self.env.lockFile(self.name)
    params += " " + self.name + ' -s "' + station_file + '" -c "' + config_file + '"'
    return self.env.start(self.name, prog, params, True)


  def _processStation(self, key_dir, profile):
    if profile:
      station_config_file = "profile_%s" % (profile,)
    else:
      station_config_file = "station_%s_%s" % (self.net, self.sta)

    cfg = seiscomp3.Config.Config()
    cfg.readConfig(os.path.join(key_dir, station_config_file))
    try: group = cfg.getString("group")
    except: group = "local"

    description = ""

    try:
        rc = seiscomp3.Config.Config()
        rc.readConfig(os.path.join(self.rc_dir, "station_%s_%s" % (self.net, self.sta)))
        description = rc.getString("description")
    except Exception as e:
        # Maybe the rc file doesn't exist, maybe there's no readable description.
        pass

    if len(description) == 0:
        description = self.sta

    content  = "[" + self.net + "_" + self.sta + "]\n"
    content += "net   = %s\n" % self.net
    content += "sta   = %s\n" % self.sta
    content += "info  = %s\n" % description
    content += "group = %s\n" % group
    content += "type  = real\n"

    return content


  def updateConfig(self):
    self._readConfig()
    template_dir = os.path.join(self.env.SEISCOMP_ROOT, "share", "templates", self.name)

    # Create purge_datafiles script
    tpl_paths = [template_dir]
    config_file = self.env.processTemplate('config.tpl', tpl_paths, self.params, True)
    if config_file:
      try: os.makedirs(self.config_dir)
      except: pass
      fd = open(os.path.join(self.config_dir, "config.ini"), "w")
      fd.write(config_file)
      fd.close()
      os.chmod(os.path.join(self.config_dir, "config.ini"), 0o755)
    else:
      try: os.remove(os.path.join(self.config_dir, "config.ini"))
      except: pass

    rx_binding = re.compile(r'(?P<module>[A-Za-z0-9_\.-]+)(:(?P<profile>[A-Za-z0-9_-]+))?$')

    bindings_dir = os.path.join(self.env.SEISCOMP_ROOT, "etc", "key")
    key_dir = os.path.join(bindings_dir, self.name)
    config_file = os.path.join(self.config_dir, "stations.ini")

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
        content = self._processStation(key_dir, profile)
        if content:
          if not config_fd:
            try: os.makedirs(self.config_dir)
            except: pass
            try: config_fd = open(config_file, "w")
            except:
              raise Exception("Error: unable to create slarchive config file '%s'" % config_file)
          config_fd.write("%s\n" % content)
        break

      fd.close()

    return 0
