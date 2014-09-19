import os

'''
Plugin handler for the liss plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    dir = os.path.join(seedlink.config_dir, "indata")
    try: address = seedlink.param('sources.mseedscan.dir')
    except: seedlink.setParam('sources.mseedscan.dir', dir)

    # key is directory
    return dir

  # Flush does nothing
  def flush(self, seedlink):
    pass
