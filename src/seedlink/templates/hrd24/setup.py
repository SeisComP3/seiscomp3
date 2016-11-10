import os

'''
Plugin handler for the EarthData PS6-24 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.hrd24.comport')
    except: seedlink.setParam('sources.hrd24.comport', '/dev/data')

    try: seedlink.param('sources.hrd24.baudrate')
    except: seedlink.setParam('sources.hrd24.baudrate', 19200)

    try: seedlink.param('sources.hrd24.bundles')
    except: seedlink.setParam('sources.hrd24.bundles', 59)

    try: seedlink.param('sources.hrd24.proc')
    except: seedlink.setParam('sources.hrd24.proc', 'hrd24_100')

    return seedlink.net + "." + seedlink.sta


  # Flush does nothing
  def flush(self, seedlink):
    pass
