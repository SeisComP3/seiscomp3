import os

'''
Plugin handler for the Wave24 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.wave24.comport')
    except: seedlink.setParam('sources.wave24.comport', '/dev/data')

    try: seedlink.param('sources.wave24.baudrate')
    except: seedlink.setParam('sources.wave24.baudrate', 57600)

    try: seedlink.param('sources.wave24.proc')
    except: seedlink.setParam('sources.wave24.proc', 'wave24bb')

    return seedlink.param('sources.wave24.comport')


  # Flush does nothing
  def flush(self, seedlink):
    pass
