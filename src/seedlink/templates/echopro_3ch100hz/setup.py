import os

'''
Plugin handler for the Kelunji EchoPro plugin.
'''

class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.echopro.station')
    except: seedlink.setParam('sources.echopro.station', 'BER')

    try: seedlink.param('sources.echopro.comport')
    except: seedlink.setParam('sources.echopro.comport', '/dev/ttyS0') 
    
    try: seedlink.param('sources.echopro.proc')
    except: seedlink.setParam('sources.echopro.proc', 'echopro_100')

    return seedlink.net + "." + seedlink.sta

  # Flush does nothing
  def flush(self, seedlink): pass
