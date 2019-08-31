import os

'''
Plugin handler for the Kelunji EchoPro plugin.
'''

class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.echopro_6ch200hz.comport')
    except: seedlink.setParam('sources.echopro_6ch200hz.comport', '/dev/ttyS0') 
    
    try: seedlink.param('sources.echopro_6ch200hz.proc')
    except: seedlink.setParam('sources.echopro_6ch200hz.proc', 'echopro_200')

    return seedlink.param('sources.echopro_6ch200hz.comport')

  # Flush does nothing
  def flush(self, seedlink): pass
