import os

'''
Plugin handler for the MWS plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.mws.comport')
    except: seedlink.setParam('sources.mws.comport', '/dev/weatherstation')

    try: seedlink.param('sources.mws.baudrate')
    except: seedlink.setParam('sources.mws.baudrate', 19200)

    try: seedlink.param('sources.mws.proc')
    except: seedlink.setParam('sources.mws.proc', 'mws')

    return seedlink.param('sources.mws.comport')


  # Flush does nothing
  def flush(self, seedlink):
    pass
