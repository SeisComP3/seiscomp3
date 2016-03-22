import os

'''
Plugin handler for the mseedfifo plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    pass

  def push(self, seedlink):
    pass

  def flush(self, seedlink):
    # Check and set defaults
    address = "%s/%s" % (seedlink.run_dir,'mseedfifo')
    try: address = seedlink.param('plugins.mseedfifo.fifo', False)
    except: address = "%s/%s" % (seedlink.run_dir,'mseedfifo')

    seedlink.setParam('plugins.mseedfifo.fifo_param', address, False)

    noexit = ''
    try:
      noexit = seedlink.param('plugins.mseedfifo.noexit', False).lower() in ("yes", "true", "1")
      if noexit:
        noexit = ' -n '
      else:
        noexit = ''
    except: noexit = ''
    seedlink.setParam('plugins.mseedfifo.noexit_param', noexit, False)
