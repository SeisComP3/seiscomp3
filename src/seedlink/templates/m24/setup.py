import os

'''
Plugin handler for the Lemmartz M24 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.m24.comport')
    except: seedlink.setParam('sources.m24.comport', '/dev/data')

    try: seedlink.param('sources.m24.baudrate')
    except: seedlink.setParam('sources.m24.baudrate', 115200)

    try: seedlink.param('sources.m24.time_offset')
    except: seedlink.setParam('sources.m24.time_offset', 0)

    try: seedlink.param('sources.m24.proc')
    except: seedlink.setParam('sources.m24.proc', 'm24_100')

    return None


  # Flush does nothing
  def flush(self, seedlink):
    pass
