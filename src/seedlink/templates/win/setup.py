import os

'''
Plugin handler for the WIN plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.instances = {}
    self.channelMap = {}
    self.idMap = {}

  def push(self, seedlink):
    # Check and set defaults
    udpport = 18000
    try: udpport = seedlink.param('sources.win.udpport')
    except: seedlink.setParam('sources.win.udpport', udpport)

    try: seedlink.param('sources.win.proc')
    except: seedlink.setParam('sources.win.proc', 'win')

    try:
      winId = self.instances[udpport]

    except KeyError:
      winId = len(self.instances)
      self.instances[udpport] = winId
      self.channelMap[winId] = []
    
    try:
      channelItems = [ x.strip() for x in seedlink.param('sources.win.channels').split(',') ]
      map = os.path.join(seedlink.config_dir, "win2sl%d.map" % winId)
      seedlink.setParam('sources.win.mapFlag',map)

    except KeyError:
      try:
        map = seedlink.param('sources.win.map')
        if not os.path.isabs(map):
          map = os.path.join(seedlink.config_dir, map)
      except: map = os.path.join(seedlink.config_dir, 'win2sl.map')

      seedlink.setParam('sources.win.mapFlag',map)
      channelItems = []

    for item in channelItems:
      mapping = [x.strip() for x in item.split(':')]
      if len(mapping) != 2:
        raise Exception("Error: invalid rename mapping '%s' in %s" % (item, seedlink.station_config_file))
      if not mapping[0] or not mapping[1]:
        raise Exception("Error: invalid rename mapping '%s' in %s" % (item, seedlink.station_config_file))

      # Prepend current station id if not explicitely given
      if not " " in mapping[1]:
        mapping[1] = seedlink._get('seedlink.station.id') + " " + mapping[1]
      if not mapping in self.channelMap[winId]:
        self.channelMap[winId].append(mapping)

      try:
        if not mapping[1] in self.idMap[mapping[0]]:
          self.idMap[mapping[0]].append(mapping[1])
      except KeyError:
        self.idMap[mapping[0]] = [mapping[1]]

    seedlink.setParam('seedlink.win.id', winId)
    
    return (udpport,map)

  def flush(self, seedlink):
    for x in self.idMap.keys():
      if len(self.idMap[x]) > 1:
        raise Exception("Error: WIN plugin has multiple mappings for id(s) %s" % ", ".join(self.idMap.keys()))

    for x in self.channelMap.keys():
      mappings = self.channelMap[x]
      if len(mappings) == 0: continue

      win2slmap = os.path.join(seedlink.config_dir, "win2sl%d.map" % x)

      fd = open(win2slmap, "w")

      for c in mappings:
        fd.write("%s %s\n" % tuple(c))

      fd.close()

