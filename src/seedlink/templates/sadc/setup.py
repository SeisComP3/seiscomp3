import os

'''
Plugin handler for the SADC10/18/20/30 plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.sadc.comport')
    except: seedlink.setParam('sources.sadc.comport', '/dev/data')

    try: seedlink.param('sources.sadc.baudrate')
    except: seedlink.setParam('sources.sadc.baudrate', 38400)

    try: seedlink.param('sources.sadc.pctime')
    except: seedlink.setParam('sources.sadc.pctime', 0)

    try: seedlink.param('sources.sadc.proc')
    except: seedlink.setParam('sources.sadc.proc', 'sadc_100')

    return seedlink.param('sources.sadc.comport')


  # Flush does nothing
  def flush(self, seedlink):
    pass
