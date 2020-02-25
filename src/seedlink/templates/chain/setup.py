from __future__ import print_function
import os, glob

'''
Plugin handler for the chain plugin. The plugin handler needs
to support two methods: push and flush.

push   Pushes a Seedlink station binding. Can be used to either
       create configuration files immediately or to manage the
       information until flush is called.

flush  Flush the configuration and return a unique key for this
       station that is used by seedlink to group eg templates for
       plugins.ini.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.chain_group = {}
    self.activeDialupConnections = 0
    self.stations = {}


  # Generates the group tag
  def generateGroupTag(self, seedlink, source_address):
    try: maxDialupConnections = int(seedlink.param('plugins.chain.dialupConnections', False))
    except: maxDialupConnections = 0

    dialup = seedlink._get('sources.chain.dialup.enable').lower() in ("yes", "true", "1")

    status_file = source_address

    group_tag = \
    '  <group address="%s"\n' % source_address + \
    '         seqfile="%s/#status_file#.seq"\n' % seedlink.run_dir + \
    '         lockfile="%s/' % seedlink.run_dir

    if not dialup or maxDialupConnections <= 0:
      group_tag += '#status_file#.pid"'
    else:
      group_tag += 'dial%d.pid"' % self.activeDialupConnections
      self.activeDialupConnections += 1
      if self.activeDialupConnections >= maxDialupConnections:
        self.activeDialupConnections = 0

    try:
      group_tag += '\n' + \
      '         overlap_removal="%s"' % seedlink.param('sources.chain.overlapRemoval')
    except: pass

    try:
      if seedlink.param('sources.chain.batchmode').lower() in ("yes", "true", "1"):
        batchmode = "yes"
      else:
        batchmode = "no"
    except:
      # Default batchmode is "yes"
      batchmode = "yes"

    group_tag += '\n' + \
    '         batchmode="%s"' % batchmode

    if dialup:
      status_file += ".dial"
      group_tag += '\n' + \
      '         uptime="%s"\n' % seedlink._get('sources.chain.dialup.uptime') + \
      '         schedule="%s"' % seedlink._get('sources.chain.dialup.schedule')

      try:
        group_tag += '\n' + \
        '         ifup="%s"' % seedlink.param('sources.chain.dialup.ifup')
      except: pass
      try:
        group_tag += '\n' + \
        '         ifdown="%s"' % seedlink.param('sources.chain.dialup.ifdown')
      except: pass

    group_tag += '>\n'
    return group_tag, status_file


  # Generates the station tag, child of <group>
  def generateStationTag(self, seedlink):
    # Create station XML tag
    id = seedlink._get('seedlink.station.id')
    station_tpl = "    <station id=\"%s\"" % id

    # set station override if configured
    try: sta = seedlink.param("sources.chain.station")
    except: sta = seedlink.sta
    station_tpl += " name=\"%s\"" % sta

    # set out name to match the stations code if sta is not equal to
    # configured station code
    if sta != seedlink.sta:
      station_tpl += " out_name=\"%s\"" % seedlink.sta

    # set network override if configured
    try: net = seedlink.param("sources.chain.network")
    except: net = seedlink.net
    station_tpl += " network=\"%s\"" % net

    # set out network to match the stations network if net is not equal to
    # configured network code
    if net != seedlink.net:
      station_tpl += " out_network=\"%s\"" % seedlink.net

    station_tpl += " selectors=\"%s\"" % seedlink._get("sources.chain.selectors").replace(',', ' ')

    renameMap = []
    unpackMap = []

    # Read and parse channel rename map
    if seedlink._get('sources.chain.channels.rename'):
      renameItems = [x.strip() for x in seedlink._get('sources.chain.channels.rename').split(',')]
      for item in renameItems:
        mapping = [x.strip() for x in item.split(':')]
        if len(mapping) != 2:
          raise Exception("Error: invalid rename mapping '%s' in %s" % (item, seedlink.station_config_file))
        if not mapping[0] or not mapping[1]:
          raise Exception("Error: invalid rename mapping '%s' in %s" % (item, seedlink.station_config_file))
        renameMap.append(mapping)

    # Read and parse channel unpack map
    if seedlink._get('sources.chain.channels.unpack'):
      unpackItems = [x.strip() for x in seedlink._get('sources.chain.channels.unpack').split(',')]
      for item in unpackItems:
        mapping = [x.strip() for x in item.split(':')]
        if len(mapping) != 2 and len(mapping) != 3:
          raise Exception("Error: invalid unpack definition '%s' in %s" % (item, seedlink.station_config_file))
        if not mapping[0] or not mapping[1]:
          raise Exception("Error: invalid unpack definition '%s' in %s" % (item, seedlink.station_config_file))
        # If double_rate is not enabled, remove the last item
        if len(mapping) == 3 and mapping[2] != "1":
          mapping.pop(2)
        unpackMap.append(mapping)
    else:
      try: seedlink.param('sources.chain.proc')
      except: seedlink.setParam('sources.chain.proc', '')

    if len(renameMap) == 0 and len(unpackMap) == 0:
      # Close tag
      station_tpl += "/>\n"
    else:
      station_tpl += ">\n"
      for mapping in renameMap:
        station_tpl += '      <rename from="%s" to="%s"/>\n' % (mapping[0], mapping[1])
      for mapping in unpackMap:
        station_tpl += '      <unpack src="%s" dest="%s"' % (mapping[0], mapping[1])
        if len(mapping) == 3:
          station_tpl += ' double_rate="yes"'
        station_tpl += '/>\n'
      station_tpl += "    </station>\n"

    return station_tpl


  # Store the information internally and create chain.xml and
  # the seedlink.ini configuration when flush is called
  def push(self, seedlink):
    try: host = seedlink.param('sources.chain.address')
    except:
      host = "geofon.gfz-potsdam.de"
      seedlink.setParam('sources.chain.address', host)

    try: port = seedlink.param('sources.chain.port')
    except:
      port = "18000"
      seedlink.setParam('sources.chain.port', port)

    try: dialuptime = seedlink.param('sources.chain.dialup.uptime')
    except: seedlink.setParam('sources.chain.dialup.uptime', 600)

    try: dialuptime = seedlink.param('sources.chain.dialup.schedule')
    except: seedlink.setParam('sources.chain.dialup.schedule', '0,30 * * * *')

    # The default grouping behaviour is enabled
    try: group = seedlink.param('sources.chain.group')
    except: group = ""

    group = group.replace(' ', '_')\
                 .replace("$NET", seedlink.net)\
                 .replace("$STA", seedlink.sta)

    source_address = host + ":" + port
    group_tag, status_file = self.generateGroupTag(seedlink, source_address)
    if group: status_file += "." + group

    chain_key = (group_tag, status_file)

    # Find out what chain.xml instance to use
    station_key = seedlink.net + "." + seedlink.sta
    chain_instance = 0
    if station_key in self.stations:
      chain_instance = self.stations[station_key]+1

    self.stations[station_key] = chain_instance

    # Register the new chainX.xml instance
    if not chain_instance in self.chain_group:
      self.chain_group[chain_instance] = {}

    chain_group = self.chain_group[chain_instance]

    if not chain_key in chain_group:
      chain_group[chain_key] = group_tag

    station_tpl = self.generateStationTag(seedlink)

    # Register tag to be flushed when flush is called
    chain_group[chain_key] += station_tpl

    # Set the name of the current chainX.xml file
    seedlink._set("seedlink.chain.id", chain_instance, False)

    return chain_instance


  # Create chain.xml and return the plugin commands for all
  # chain plugins
  def flush(self, seedlink):
    status_map = {}

    # Create chainX.xml
    chains = {}
    for x in self.chain_group.keys():
      chainxmlbase = "chain%d.xml" % x
      chainxml = os.path.join(seedlink.config_dir, chainxmlbase)
      chain_group = self.chain_group[x]
      if chain_group:
        # Mark this file as used
        chains[chainxml] = 1
        fd = open(chainxml, "w")

        try:
          if seedlink.param('plugins.chain.loadTimeTable', False).lower() in ("yes", "true", "1"):
            loadTimeTable = True
          else:
            loadTimeTable = False
        except: loadTimeTable = True

        if loadTimeTable:
          fd.write(seedlink._process_template("chain_head.tpl", "chain", False))
        else:
          fd.write(seedlink._process_template("chain_head_notimetable.tpl", "chain", False))
        first = True
        for ((g,s),i) in chain_group.items():
          if not first: fd.write('\n')
          if s in status_map:
            status_map[s] += 1
            s += ".%d" % status_map[s]
          else:
            status_map[s] = 0

          fd.write(i.replace("#status_file#", s))
          fd.write("  </group>\n")
          first = False
        fd.write("</chain>\n")
        fd.close()
      else:
        # If no groups are configured, delete chainX.xml
        try: os.remove(chainxml)
        except: print("Warning: %s could not be removed" % chainxml)

    files = glob.glob(os.path.join(seedlink.config_dir, "chain*"))
    for f in files:
        if f in chains: continue
      try: os.remove(f)
      except: print("Warning: %s could not be removed" % f)
