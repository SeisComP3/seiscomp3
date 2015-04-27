import os

'''
Plugin handler for the Antelope ORB plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # key is station (one instance per station)
    return seedlink.net + "." + seedlink.sta

  # Flush does nothing
  def flush(self, seedlink):
    pass
