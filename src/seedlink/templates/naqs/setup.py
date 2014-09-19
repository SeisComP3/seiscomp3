import os

'''
Plugin handler for the NRTS plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    pass

  def push(self, seedlink):
    address = "localhost"
    try: address = seedlink.param('sources.naqs.address')
    except: seedlink.setParam('sources.naqs.address', address)

    port = 28000
    try: port = int(seedlink.param('sources.naqs.port'))
    except: seedlink.setParam('sources.naqs.port', port)

    try: seedlink.param('sources.naqs.proc')
    except: seedlink.setParam('sources.naqs.proc', 'naqs_bb40_sm100')

    # Key is address (one instance per address)
    return address + ":" + str(port)


  # Flush does nothing
  def flush(self, seedlink):
    pass
