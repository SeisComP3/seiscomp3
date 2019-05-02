import os
import glob
import sys


class CAPSProfile:
    def __init__(self):
        self.streams = [] # List of strings, e.g. XY.ABCD.*.*
        self.stations = [] # List of network, station tuples, e.g. (XY,ABCD)
        self.oldStates = [] # Content of old state file


'''
Plugin handler for the CAPS plugin.
'''


class SeedlinkPluginHandler:
    # Create defaults
    def __init__(self):
        self.profiles = {}

    def push(self, seedlink):
        # Check and set defaults
        try:
            address = seedlink.param('sources.caps.address')
        except KeyError:
            address = "localhost:18002"

        try:
            streams = [chaId.strip() for chaId in seedlink.param(
                'sources.caps.streams').split(',')]
        except KeyError:
            seedlink.setParam('sources.caps.streams', "*.*")
            streams = ["*.*"]

        try:
            encoding = seedlink.param('sources.caps.encoding')
        except KeyError:
            seedlink.setParam('sources.caps.encoding', "STEIM2")

        # parse address URL and create capsId of form:
        # host[.port][_user]
        addressFormatError = "Error: invalid address format, expected " \
                             "[[caps|capss]://][user:pass@]host[:port]"

        # protocol
        toks = address.split("://")
        if len(toks) > 2:
            raise Exception(addressFormatError)
        elif len(toks) == 2:
            protocol = toks[0]
            address = toks[1]
            if protocol != "caps" and protocol != "capss":
                raise Exception(addressFormatError)
        else:
            protocol = "caps"

        # authentication
        toks = address.split("@")
        if len(toks) > 2:
            raise Exception(addressFormatError)
        elif len(toks) == 2:
            capsId = "%s_%s" % (toks[1].replace(
                ":", "."), toks[0].split(":")[0])
        else:
            capsId = address.replace(":", ".")

        address = "%s://%s" % (protocol, address)

        if capsId not in self.profiles:
            profile = CAPSProfile()
            self.profiles[capsId] = profile
        else:
            profile = self.profiles[capsId]

        for chaId in streams:
            toks = chaId.split('.')
            if len(toks) != 2:
                raise Exception(
                    "Error: invalid stream format, expected [LOC.CHA]")

            streamID = seedlink.net + "." + seedlink.sta + "." + chaId
            profile.streams.append(streamID)
            profile.stations.append((seedlink.net, seedlink.sta))

        log = os.path.join(seedlink.config_dir, "caps2sl.%s.state" % capsId)
        streamsFile = os.path.join(
            seedlink.config_dir, "caps2sl.%s.req" % capsId)
        seedlink.setParam('sources.caps.address', address)
        seedlink.setParam('sources.caps.log', log)
        seedlink.setParam('sources.caps.streamsFile', streamsFile)
        seedlink.setParam('seedlink.caps.id', capsId)

        return capsId

    def flush(self, seedlink):
        # Populate request file per address
        for id, profile in self.profiles.items():
            caps2slreq = os.path.join(
                seedlink.config_dir, "caps2sl.%s.req" % id)
            fd = open(caps2slreq, "w")
            for streamId in profile.streams:
                fd.write("%s\n" % streamId)
            fd.close()

            try:
                caps2slstate = os.path.join(seedlink.config_dir, "caps2sl.%s.state" % id)
                # Read existing state file
                fd = open(caps2slstate, "r")
                profile.oldStates = [line.strip() for line in fd.readlines()]
            except:
                pass

        # Delete all existing state files
        for fl in glob.glob(os.path.join(seedlink.config_dir, "caps2sl.*.state")):
            try:
                os.remove(fl)
            except:
                sys.stderr.write("Failed to remove old state file: %s\n" % str(fl))

        # Clean up state file to contain only configured stations
        for id, profile in self.profiles.items():
            caps2slstate = os.path.join(seedlink.config_dir, "caps2sl.%s.state" % id)

            newStates = []

            for (net, sta) in profile.stations:
                for line in profile.oldStates:
                    if line.startswith(net + "." + sta + "."):
                        newStates.append(line)

            if len(newStates) > 0:
                fd = open(caps2slstate, "w")
                for line in newStates:
                    fd.write(line + "\n")
                fd.close()
