import os

'''
Plugin handler for the Quanterra/330 plugin.
'''
class SeedlinkPluginHandler:
	# Create defaults
	def __init__(self): pass

	def push(self, seedlink):
		# Check and set defaults
		try: seedlink.param('sources.ps2400_eth.address')
		except: seedlink.setParam('sources.ps2400_eth.address', '127.0.0.1')

		try: seedlink.param('sources.ps2400_eth.port')
		except: seedlink.setParam('sources.ps2400_eth.port', 1411)

		try: proc = seedlink.param('sources.ps2400_eth.proc')
		except: seedlink.setParam('sources.ps2400_eth.proc', 'ps2400_eth_edata_100')

		# Key is per station and configuration settings
		key = ";".join([
			str(seedlink.param('seedlink.station.id')),
			str(seedlink.param('sources.ps2400_eth.address')),
			str(seedlink.param('sources.ps2400_eth.port'))])
		return key


	# Flush does nothing
	def flush(self, seedlink):
		pass
