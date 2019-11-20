#!/usr/bin/env python

###############################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
###############################################################################

from __future__ import print_function

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
            (ctTXT, '?format=text&level=channel'),
            (ctTXT, '?format=text&includerestricted=false'),
            (ctTXT, '?format=text&startbefore=2019-07-01'),
            (ctXML, '?level=channel&includeavailability=true', [(172,198,7,0)]),
            (ctXML, '?format=sc3ml&network=AM&station=R0F05&location=00' \
                    '&channel=SHZ&latitude=52&longitude=13&maxradius=0.5' \
                    '&level=response&includeavailability=true'),
        ]
        for test in tests:
            ignoreRanges = [] if len(test) < 3 else test[2]
            self.testGET('{}{}'.format(query, test[1]), test[0],
                         dataFile='{}{}.txt'.format(resFile, i), testID=i,
                         ignoreRanges=ignoreRanges)
            i += 1




#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestStation()
    sys.exit(app())



# vim: ts=4 et tw=79
