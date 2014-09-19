import os

'''
Plugin handler for the Guralp DM24 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.dm24.comport')
    except: seedlink.setParam('sources.dm24.comport', '/dev/data')

    try: seedlink.param('sources.dm24.baudrate')
    except: seedlink.setParam('sources.dm24.baudrate', 19200)

    try: seedlink.param('sources.dm24.proc')
    except: seedlink.setParam('sources.dm24.proc', 'dm24_20')

    return seedlink.net + "." + seedlink.sta


  # Flush does nothing
  def flush(self, seedlink):
    pass

