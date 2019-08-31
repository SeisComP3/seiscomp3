import os

'''
Plugin handler for the NRTS plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.__channels = set()
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

    self.__channels.add(seedlink.param('seedlink.station.code'))

    # Key is address and network code
    return (address + ":" + str(port), seedlink.param('seedlink.station.network'))


  # Flush does nothing
  def flush(self, seedlink):
    seedlink.setParam('sources.naqs.channels', ",".join(self.__channels))
    pass
