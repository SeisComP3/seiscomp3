import os

'''
Plugin handler for the Kelunji EchoPro plugin.
'''

class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.echopro_3ch100hz.comport')
    except: seedlink.setParam('sources.echopro_3ch100hz.comport', '/dev/ttyS0') 
    
    try: seedlink.param('sources.echopro_3ch100hz.proc')
    except: seedlink.setParam('sources.echopro_3ch100hz.proc', 'echopro_100')

    return seedlink.param('sources.echopro_3ch100hz.comport')

  # Flush does nothing
  def flush(self, seedlink): pass
