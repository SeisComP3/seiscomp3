import os

'''
Plugin handler for the WAGO (T-Eektronik) plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self): pass

  def push(self, seedlink):
    # Check and set defaults
    try: seedlink.param('sources.wago.port')
    except: seedlink.setParam('sources.wago.port', 502)

    try: seedlink.param('sources.wago.proc')
    except: seedlink.setParam('sources.wago.proc', 'wago')

    try:
      wago_chan = dict(zip(seedlink.param('sources.wago.channels').lower().split(','), range(26)))

    except:
      wago_chan = dict()
      
    for letter in range(ord('a'), ord('z') + 1):
      try: seedlink.param('sources.wago.channels.%s.sid' % chr(letter))
      except: seedlink.setParam('sources.wago.channels.%s.sid' % chr(letter), wago_chan.get(chr(letter), 255))

    return seedlink.net + "." + seedlink.sta

  # Flush does nothing
  def flush(self, seedlink):
    pass
