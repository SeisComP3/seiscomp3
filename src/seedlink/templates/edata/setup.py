import os

'''
Plugin handler for the EarthData PS6-24 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.edata.comport')
    except: seedlink.setParam('sources.edata.comport', '/dev/data')

    try: seedlink.param('sources.edata.baudrate')
    except: seedlink.setParam('sources.edata.baudrate', 115200)

    try: seedlink.param('sources.edata.proc')
    except: seedlink.setParam('sources.edata.proc', 'edata_100')

    return seedlink.net + "." + seedlink.sta


  # Flush does nothing
  def flush(self, seedlink):
    pass
