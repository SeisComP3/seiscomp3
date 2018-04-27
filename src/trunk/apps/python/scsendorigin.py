#!/usr/bin/env python

############################################################################
#    Copyright (C) by GFZ Potsdam                                          #
#                                                                          #
#    You can redistribute and/or modify this program under the             #
#    terms of the SeisComP Public License.                                 #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    SeisComP Public License for more details.                             #
############################################################################

import sys
from seiscomp3 import Core, DataModel, Client, Logging

class SendOrigin(Client.Application):

    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
        self.setDatabaseEnabled(False, False)
        self.setMessagingEnabled(True)
        self.setPrimaryMessagingGroup("GUI")

    def init(self):
        if not Client.Application.init(self): return False

        try:
            cstr = self.commandline().optionString("coord")
            tstr  = self.commandline().optionString("time")
        except:
            sys.stderr.write("must specify origin using '--coord lat,lon,dep --time time'\n")
            return False

        self.origin = DataModel.Origin.Create()

        ci = DataModel.CreationInfo()
        ci.setAgencyID(self.agencyID())
        ci.setCreationTime(Core.Time.GMT())
        self.origin.setCreationInfo(ci)

        lat,lon,dep = map(float, cstr.split(","))
        self.origin.setLongitude(DataModel.RealQuantity(lon))
        self.origin.setLatitude(DataModel.RealQuantity(lat))
        self.origin.setDepth(DataModel.RealQuantity(dep))

        time = Core.Time() 
        time.fromString(tstr.replace("/","-") + ":0:0", "%F %T")
        self.origin.setTime(DataModel.TimeQuantity(time))

        return True

    def createCommandLineDescription(self):
        try:
            self.commandline().addGroup("Parameters")
            self.commandline().addStringOption("Parameters", "coord", "lat,lon,dep of origin")
            self.commandline().addStringOption("Parameters", "time", "time of origin")
        except:
            Logging.warning("caught unexpected error %s" % sys.exc_info())

    def run(self):
        msg = DataModel.ArtificialOriginMessage(self.origin)
        self.connection().send(msg)
        return True

app = SendOrigin(len(sys.argv), sys.argv)
#app.setName("scsendorigin")
app.setMessagingUsername("scsendorg")
sys.exit(app())
