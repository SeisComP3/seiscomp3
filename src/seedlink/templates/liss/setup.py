import os

'''
Plugin handler for the liss plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    try: host = seedlink.param('sources.liss.address')
    except: host = "$STATION.$NET.liss.org"
    try: port = seedlink.param('sources.liss.port')
    except:
      port = "4000"
      seedlink.setParam('sources.liss.port', port)

    host = host.replace("$STATION", seedlink.sta).replace("$NET", seedlink.net)
    seedlink.setParam('sources.liss.address', host)

    # key is station (one instance per station)
    return seedlink.net + "." + seedlink.sta

  # Flush does nothing
  def flush(self, seedlink):
    pass
