import os

'''
Plugin handler for the MK6 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    return seedlink.net + "." + seedlink.sta


  # Flush does nothing
  def flush(self, seedlink):
    pass
