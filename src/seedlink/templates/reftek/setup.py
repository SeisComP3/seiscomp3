import os, time

'''
Plugin handler for the Reftek export plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.instances = {}
    self.unitMap = {}
    self.idMap = {}

  def push(self, seedlink):
    # Check and set defaults
    address = "127.0.0.1"
    try: address = seedlink.param('sources.reftek.address')
    except: seedlink.setParam('sources.reftek.address', address)

    port = 2543
    try: port = int(seedlink.param('sources.reftek.port'))
    except: seedlink.setParam('sources.reftek.port', port)

    proc = "reftek"
    try: proc = seedlink.param('sources.reftek.proc')
    except: seedlink.setParam('sources.reftek.proc', proc)

    key = (address + ":" + str(port), proc)

    try:
      reftekId = self.instances[key]

    except KeyError:
      reftekId = len(self.instances)
      self.instances[key] = reftekId
      self.unitMap[reftekId] = []

    try:
      unit = seedlink.param('sources.reftek.unit')
      map = os.path.join(seedlink.config_dir, "reftek2sl%d.map" % reftekId)
      seedlink.setParam('sources.reftek.mapFlag', map)

      mapping = (unit, seedlink._get('seedlink.station.id'))
      if not mapping in self.unitMap[reftekId]:
        self.unitMap[reftekId].append(mapping)

      try:
        if not mapping[1] in self.idMap[mapping[0]]:
          self.idMap[mapping[0]].append(mapping[1])
      except KeyError:
        self.idMap[mapping[0]] = [mapping[1]]

    except KeyError:
      try:
        map = seedlink.param('sources.reftek.map')
        if not os.path.isabs(map):
          map = os.path.join(seedlink.config_dir, map)
      except: map = os.path.join(seedlink.config_dir, 'reftek2sl.map')
      seedlink.setParam('sources.reftek.mapFlag', map)

    timeout = 60
    try: timeout = int(seedlink.param('sources.reftek.timeout'))
    except: seedlink.setParam('sources.reftek.timeout', timeout)

    default_tq = 40
    try: default_tq = int(seedlink.param('sources.reftek.default_tq'))
    except: seedlink.setParam('sources.reftek.default_tq', default_tq)

    unlock_tq = 10
    try: unlock_tq = int(seedlink.param('sources.reftek.unlock_tq'))
    except: seedlink.setParam('sources.reftek.unlock_tq', unlock_tq)

    log_soh = "true"
    try:
      if seedlink.param('sources.reftek.log_soh').lower() in ("yes", "true", "1"):
        log_soh = "true"
      else:
        log_soh = "false"
    except: seedlink.setParam('sources.reftek.log_soh', log_soh)

    # Key is address (one instance per address)
    return (key, map)


  # Flush generates the map file(s)
  def flush(self, seedlink):
    for x in self.idMap.keys():
      if len(self.idMap[x]) > 1:
        raise Exception("Error: Reftek plugin has multiple mappings for unit %s" % x)

    for x in self.unitMap.keys():
      mappings = self.unitMap[x]
      if len(mappings) == 0: continue

      reftek2slmap = os.path.join(seedlink.config_dir, "reftek2sl%d.map" % x)

      fd = open(reftek2slmap, "w")

      fd.write("# Generated at %s - Do not edit!\n" % time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime()))
      for c in mappings:
        fd.write("%s %s\n" % (c[0], c[1]))

      fd.close()
