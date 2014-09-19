import os

'''
Plugin handler for the Geotech DR24 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.dr24.comport')
    except: seedlink.setParam('sources.dr24.comport', '/dev/data')

    try: seedlink.param('sources.dr24.baudrate')
    except: seedlink.setParam('sources.dr24.baudrate', 19200)

    try: seedlink.param('sources.dr24.proc')
    except: seedlink.setParam('sources.dr24.proc', 'dr24_50')

    return seedlink.net + "." + seedlink.sta


  # Flush does nothing
  def flush(self, seedlink):
    pass
