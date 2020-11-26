#!/usr/bin/env seiscomp-python
import sys
from seiscomp3 import Client, DataModel, IO


class ObjectDumper(Client.Application):

    def __init__(self):
        argv = [bytes(a.encode()) for a in sys.argv]
        Client.Application.__init__(self, len(argv), argv)
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, False)
        self.setMessagingUsername("")

    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)
        self.commandline().addGroup("Dump")
        self.commandline().addStringOption("Dump", "public-id,P", "publicID")

    def loadEventParametersObject(self, publicID):
        for tp in \
                DataModel.Pick, DataModel.Amplitude, DataModel.Origin, \
                DataModel.Event, DataModel.FocalMechanism, \
                DataModel.Magnitude, DataModel.StationMagnitude:

            obj = self.query().loadObject(tp.TypeInfo(), publicID)
            obj = tp.Cast(obj)
            if obj:
                ep = DataModel.EventParameters()
                ep.add(obj)
                return ep

    def loadInventoryObject(self, publicID):
        for tp in \
                DataModel.Network, DataModel.Station, DataModel.Sensor, \
                DataModel.SensorLocation, DataModel.Stream:

            obj = self.query().loadObject(tp.TypeInfo(), publicID)
            obj = tp.Cast(obj)
            if obj:
                return obj

    def run(self):
        publicID = self.commandline().optionString("public-id")
        obj = self.loadEventParametersObject(publicID)
        if obj is None:
            obj = self.loadInventoryObject(publicID)
        if obj is None:
            raise ValueError("unknown object '" + publicID + "'")

        # dump formatted XML archive to stdout
        ar = IO.XMLArchive()
        ar.setFormattedOutput(True)
        ar.create("-")
        ar.writeObject(obj)
        ar.close()
        return True


if __name__ == "__main__":
    app = ObjectDumper()
    app()
