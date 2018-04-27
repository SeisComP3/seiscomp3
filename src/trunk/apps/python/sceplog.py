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

import sys, os
import seiscomp3.Client


class EventParameterLog(seiscomp3.Client.Application):
  def __init__(self, argc, argv):
    seiscomp3.Client.Application.__init__(self, argc, argv)

    self.setMessagingEnabled(True)
    self.setDatabaseEnabled(False, False)
    self.setMessagingUsername("")
    self.setPrimaryMessagingGroup(seiscomp3.Communication.Protocol.LISTENER_GROUP)
    self.addMessagingSubscription("EVENT")
    self.addMessagingSubscription("LOCATION")
    self.addMessagingSubscription("MAGNITUDE")
    self.addMessagingSubscription("AMPLITUDE")
    self.addMessagingSubscription("PICK")

    self.setAutoApplyNotifierEnabled(True)
    self.setInterpretNotifierEnabled(True)

    # EventParameter object
    self._eventParameters = seiscomp3.DataModel.EventParameters()


  def run(self):
    if seiscomp3.Client.Application.run(self) == False:
      return False

    ar = seiscomp3.IO.XMLArchive()
    ar.setFormattedOutput(True)
    if ar.create("-") == True:
      ar.writeObject(self._eventParameters)
      ar.close()
      # Hack to avoid the "close failed in file object destructor"
      # exception
#     print ""
      sys.stdout.write("\n")

    return True


app = EventParameterLog(len(sys.argv), sys.argv)
sys.exit(app())
