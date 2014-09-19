import os

'''
Plugin handler for the NRTS plugin.
'''
class SeedlinkPluginHandler:
  # Create defaults
  def __init__(self):
    self.server_streams = {}

  def push(self, seedlink):
    # Check and set defaults
    address = "idahub.ucsd.edu"
    try: address = seedlink.param('sources.nrts.address')
    except: seedlink.setParam('sources.nrts.address', address)

    port = 39136
    try: port = int(seedlink.param('sources.nrts.port'))
    except: seedlink.setParam('sources.nrts.port', port)

    try: seedlink.param('sources.nrts.proc')
    except: seedlink.setParam('sources.nrts.proc', 'nrts')

    try: station_streams = seedlink.param('sources.nrts.streams').split(',')
    except: station_streams = ['*.*']

    try: server_streams = self.server_streams[(address,port)] + '+'
    except: server_streams = ''

    server_streams += '+'.join(['%s.%s' % (seedlink.sta.lower(), p.lower()) for p in station_streams ])

    self.server_streams[(address,port)] = server_streams
    seedlink.setParam('seedlink.source.streams', server_streams)

    # Key is address (one instance per address)
    return address + ":" + str(port)


  # Flush does nothing
  def flush(self, seedlink):
    pass
