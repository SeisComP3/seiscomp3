import os


def updateKey(key, seedlink, name):
  key[0] += name + ":" + str(seedlink.param(name)) + ";"


def updateSwitch(key, seedlink, name, var):
  try:
    if seedlink.param(name).lower() in ("yes", "true", "1"):
      var = True
    else:
      var = False
  except: pass

  if var == True:
    seedlink.setParam(name + '_yesno', 'yes')
  else:
    seedlink.setParam(name + '_yesno', 'no')
  updateKey(key, seedlink, name + '_yesno')


def updateString(key, seedlink, name, var):
  try: var = seedlink.param(name)
  except: pass
  seedlink.setParam(name, var)
  updateKey(key, seedlink, name)


def updateInt(key, seedlink, name, var):
  try: var = int(seedlink.param(name))
  except: pass
  seedlink.setParam(name, var)
  updateKey(key, seedlink, name)


def updatePath(key, seedlink, name, var):
  try: var = seedlink.param(name)
  except: pass
  var = var.replace("@ROOTDIR@", seedlink.pkgroot)
  seedlink.setParam(name, var)
  updateKey(key, seedlink, name)


'''
Plugin handler for the FS plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.station_list = {}

  def push(self, seedlink):
    key = [""]

    # Check and set defaults
    updateString(key, seedlink, 'sources.fs_mseed.input_type', 'ddb')
    updateString(key, seedlink, 'sources.fs_mseed.data_format', 'mseed')
    updatePath(key, seedlink, 'sources.fs_mseed.location', os.path.join("@ROOTDIR@", "var", "lib", "seedlink", "indata"))

    try:
      pattern = seedlink.param('sources.fs_mseed.pattern')
      seedlink.setParam('sources.fs_mseed.pattern_def', 'pattern=' + pattern)
    except:
      seedlink.setParam('sources.fs_mseed.pattern_def', '*pattern = BH[NEZ]')
    updateKey(key, seedlink, 'sources.fs_mseed.pattern_def')

    updateInt(key, seedlink, 'sources.fs_mseed.scan_level', 2)
    updateSwitch(key, seedlink, 'sources.fs_mseed.move_files', True)
    updateSwitch(key, seedlink, 'sources.fs_mseed.delete_files', False)
    updateSwitch(key, seedlink, 'sources.fs_mseed.use_timestamp', False)

    updatePath(key, seedlink, 'sources.fs_mseed.timestamp_file', os.path.join("@ROOTDIR@", "var", "run", "seedlink", "fs_mseed.tim"))

    updateInt(key, seedlink, 'sources.fs_mseed.polltime', 10)
    updateInt(key, seedlink, 'sources.fs_mseed.delay', 30)
    updateInt(key, seedlink, 'sources.fs_mseed.verbosity', 1)
    updateInt(key, seedlink, 'sources.fs_mseed.zero_sample_limit', 10)
    updateInt(key, seedlink, 'sources.fs_mseed.default_timing_quality', -1)

    channel_map = ""

    for (name, value) in seedlink.station_params.items():
      if name.startswith("sources.fs_mseed.channels."):
        toks = name[len("sources.fs_mseed.channels."):].split('.')
        if len(toks) != 2: continue
        if toks[1] != "source_id": continue
        channel_map += "channel %s source_id = \"%s\"\n" % (toks[0], value)

    seedlink.setParam('sources.fs_mseed.channel_map', channel_map)
    key[0] += "sources.fs_mseed.channel_map:" + channel_map + ";"

    try: station_list = self.station_list[key[0]] + ','
    except: station_list = ''

    station_list += seedlink.sta
    self.station_list[key[0]] = station_list
    seedlink.setParam('sources.fs_mseed.station_list_def', 'station_list=' + station_list)

    # Key is per config
    return key[0]

  # Flush does nothing
  def flush(self, seedlink):
    pass
