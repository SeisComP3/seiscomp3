import os, time

'''
Plugin handler for the Earthworm export plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.instances = {}
    self.channelMap = {}
    self.idMap = {}

  def push(self, seedlink):
    # Check and set defaults
    host = seedlink._get('sources.scream_ring.address')
    try: tcpport = int(seedlink.param('sources.scream_ring.tcpport'))
    except: tcpport = 1567
    try: port = int(seedlink.param('sources.scream_ring.port'))
    except: port = 1567

    seedlink.setParam('sources.scream_ring.tcpport', tcpport);
    seedlink.setParam('sources.scream_ring.port', port);

    try: rsize = int(seedlink.param('sources.scream_ring.rsize'))
    except: rsize = 1000

    seedlink.setParam('sources.scream_ring.rsize', rsize);

    try:
      if seedlink.param('sources.scream_ring.tcp').lower() in ("yes","true","1"):
        seedlink.setParam('sources.scream_ring.tcpFlag',' -tcp')
      else:
        seedlink.setParam('sources.scream_ring.tcpFlag','')
    except:
      seedlink.setParam('sources.scream_ring.tcpFlag','')

    key = (host, tcpport, port, seedlink.param('sources.scream_ring.tcpFlag'))

    try:
      screamId = self.instances[key]

    except KeyError:
      screamId = len(self.instances)
      self.instances[key] = screamId
      self.channelMap[screamId] = []


    try:
      channelItems = [ x.strip() for x in seedlink.param('sources.scream_ring.channels').split(',') ]
      map = os.path.join(seedlink.config_dir, "scream_ring2sl%d.map" % screamId)
      seedlink.setParam('sources.scream_ring.mapFlag',map)

    except KeyError:
      try:
        map = seedlink.param('sources.scream.map')
        if not os.path.isabs(map):
          map = os.path.join(seedlink.config_dir, map)
      except: map = os.path.join(seedlink.config_dir, 'scream2sl.map')

      seedlink.setParam('sources.scream.mapFlag',map)
      channelItems = []

    for item in channelItems:
      mapping = [x.strip() for x in item.split(':')]
      if len(mapping) != 2:
        raise Exception("Error: invalid channel mapping '%s' in %s" % (item, seedlink.station_config_file))
      if not mapping[0] or not mapping[1]:
        raise Exception("Error: invalid channel mapping '%s' in %s" % (item, seedlink.station_config_file))

      # Prepend current station id if not explicitely given
      if not " " in mapping[1]:
        mapping[1] = seedlink._get('seedlink.station.id') + " " + mapping[1]

      mapping[1] = [x.strip() for x in mapping[1].split()]

      # Add network code
      mapping.insert(1, seedlink._get('seedlink.station.network'))

      if not mapping in self.channelMap[screamId]:
        self.channelMap[screamId].append(mapping)

      try:
        if not mapping[1] in self.idMap[mapping[0]]:
          self.idMap[mapping[0]].append(mapping[1])
      except KeyError:
        self.idMap[mapping[0]] = [mapping[1]]

    return (key,map)


  # Flush does nothing
  def flush(self, seedlink):
      for x in self.idMap.keys():
        if len(self.idMap[x]) > 1:
          raise Exception("Error: Scream plugin has multiple mappings for id(s) %s" % ", ".join(self.idMap.keys()))

      cols = [0,0,0,0]

      for x in self.channelMap.keys():
        for c in self.channelMap[x]:
          vals = [c[0], c[1], c[2][0], c[2][1]]
          for i in range(len(vals)):
            if len(vals[i]) > cols[i]: cols[i] = len(vals[i])

      for x in self.channelMap.keys():
        mappings = self.channelMap[x]
        if len(mappings) == 0: continue

        scream2slmap = os.path.join(seedlink.config_dir, "scream_ring2sl%d.map" % x)

        fd = open(scream2slmap, "w")

        line_id = 1
        fd.write("# Generated at %s - Do not edit!\n" % time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime()))
        for c in mappings:
          fd.write("ChanInfo    %s%s    %s%s    %s%s    %s%s    %d\n" % (c[0], " "*(cols[0]-len(c[0])), c[1], " "*(cols[1]-len(c[1])), c[2][0], " "*(cols[2]-len(c[2][0])), c[2][1], " "*(cols[3]-len(c[2][1])), line_id))
          line_id = line_id + 1

        fd.close()

