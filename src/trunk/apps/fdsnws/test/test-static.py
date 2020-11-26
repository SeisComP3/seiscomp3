#!/usr/bin/env python

###############################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
###############################################################################

from __future__ import absolute_import, division, print_function

import sys

from fdsnwstest import FDSNWSTest
from fdsnws import event, station, dataselect, availability


###############################################################################
class TestStatic(FDSNWSTest):

    #--------------------------------------------------------------------------
    def test(self):
        print('Testing static files')

        ctTXT = 'text/plain'
        ctXML = 'application/xml'
        share = self.sharedir + '/'

        # main url
        self.testGET(self.url)

        # event
        self.testGET('{0}/event'.format(self.url))
        self.testGET('{0}/event/1'.format(self.url),
                     dataFile=share+'event.html')
        self.testGET('{0}/event/1/version'.format(self.url), ctTXT,
                     data=event.VERSION)
        self.testGET('{0}/event/1/application.wadl'.format(self.url), ctXML,
                     dataFile=share+'event.wadl')
        self.testGET('{0}/event/1/catalogs'.format(self.url), ctXML,
                     dataFile=share+'catalogs.xml')
        self.testGET('{0}/event/1/contributors'.format(self.url), ctXML,
                     dataFile=share+'contributors.xml')
        self.testGET('{0}/event/1/builder'.format(self.url),
                     dataFile=share+'event-builder.html')

        # station
        self.testGET('{0}/station'.format(self.url))
        self.testGET('{0}/station/1'.format(self.url),
                     dataFile=share+'station.html')
        self.testGET('{0}/station/1/version'.format(self.url), ctTXT,
                     data=station.VERSION)
        self.testGET('{0}/station/1/application.wadl'.format(self.url), ctXML,
                     )#dataFile=share+'station.wadl')
        self.testGET('{0}/station/1/builder'.format(self.url),
                     dataFile=share+'station-builder.html')

        # dataselect
        self.testGET('{0}/dataselect'.format(self.url))
        self.testGET('{0}/dataselect/1'.format(self.url),
                     dataFile=share+'dataselect.html')
        self.testGET('{0}/dataselect/1/version'.format(self.url), ctTXT,
                     data=dataselect.VERSION)
        self.testGET('{0}/dataselect/1/application.wadl'.format(self.url),
                     ctXML, dataFile=share+'dataselect.wadl')
        self.testGET('{0}/dataselect/1/builder'.format(self.url),
                     dataFile=share+'dataselect-builder.html')

        # availability
        self.testGET('{0}/availability'.format(self.url))
        self.testGET('{0}/availability/1'.format(self.url),
                     dataFile=share+'availability.html')
        self.testGET('{0}/availability/1/version'.format(self.url), ctTXT,
                     data=availability.VERSION)
        self.testGET('{0}/availability/1/application.wadl'.format(self.url),
                     ctXML, dataFile=share+'availability.wadl')
        self.testGET('{0}/availability/1/builder-query'.format(self.url),
                     dataFile=share+'availability-builder-query.html')
        self.testGET('{0}/availability/1/builder-extent'.format(self.url),
                     dataFile=share+'availability-builder-extent.html')


#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestStatic()
    sys.exit(app())



# vim: ts=4 et tw=79
