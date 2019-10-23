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


###############################################################################
class TestStation(FDSNWSTest):

    #--------------------------------------------------------------------------
    def test(self):
        print('Testing station service')

        query = '{}/station/1/query'.format(self.url)
        ctTXT = 'text/plain'
        ctXML = 'application/xml'
        resFile = self.rootdir + '/results/station-'

        i = 1
        tests = [
            ('?format=text&level=channel', ctTXT, [], False),
            ('?format=text&includerestricted=false', ctTXT, [], True),
            ('?format=text&startbefore=2019-07-01', ctTXT, [], False),
            ('?level=channel&includeavailability=true', ctXML, [(172,198,7,0)], False),
            ('?format=sc3ml&network=AM&station=R0F05&location=00&channel=SHZ&latitude=52&longitude=13&maxradius=0.5&level=response&includeavailability=true', ctXML, [], True),
        ]
        for q, ct, ignoreRanges, concurrent in tests:
            self.testGET('{}{}'.format(query, q), ct, ignoreRanges, concurrent,
                         dataFile='{}{}.txt'.format(resFile, i), testID=i)
            i += 1


#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestStation()
    sys.exit(app())



# vim: ts=4 et tw=79
