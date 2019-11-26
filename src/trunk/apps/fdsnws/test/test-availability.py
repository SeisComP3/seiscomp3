#!/usr/bin/env python

###############################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
###############################################################################

from __future__ import absolute_import, division, print_function

import sys

from requests.auth import HTTPDigestAuth
from fdsnwstest import FDSNWSTest


###############################################################################
class TestAvailability(FDSNWSTest):

    #--------------------------------------------------------------------------
    def test(self):
        print('Testing availability service')

        query = '{}/availability/1/'.format(self.url)
        ctTXT = 'text/plain'
        ctCSV = 'text/csv'
        ctJSON = 'application/json'
        resFile = self.rootdir + '/results/availability-{}.txt'

        dAuth = HTTPDigestAuth('sysop', 'sysop')

        i = 1
        tests = [
            (ctTXT, 'extent', []),
            (ctTXT, 'extent?merge=samplerate', []),
            (ctTXT, 'extent?merge=quality', []),
            (ctTXT, 'extent?merge=quality,samplerate', []),
            (ctTXT, 'extent?starttime=2019-08-20T00:00:00&' \
             'orderby=latestupdate', []),
            (ctTXT, 'extent?network=AM&channel=HDF&' \
             'orderby=timespancount_desc', []),
            (ctCSV, 'extentauth?station=R0F05&orderby=latestupdate_desc&' \
             'includerestricted=true&format=geocsv', []),
            (ctTXT, 'extent?orderby=latestupdate_desc&format=request', []),
            (ctJSON, 'extentauth?orderby=latestupdate_desc&' \
             'includerestricted=true&format=json&merge=quality', [(12,32)]),
            (ctTXT, 'query?net=AM', []),
            (ctTXT, 'query?net=AM&mergegaps=10.0', []),
            (ctTXT, 'query?net=AM&mergegaps=10.0&merge=overlap', []),
            (ctTXT, 'query?net=AM&mergegaps=10.0&merge=overlap,samplerate',
             []),
            (ctTXT, 'query?net=AM&mergegaps=10.0&' \
             'merge=overlap,samplerate,quality', []),
            (ctTXT, 'query?net=AM&show=latestupdate&limit=3', []),
            (ctCSV, 'query?net=AM&format=geocsv&show=latestupdate', []),
            (ctJSON, 'query?net=AM&format=json&show=latestupdate', [(12,32)]),
            (ctJSON, 'query?net=AM&channel=HDF&format=json&' \
             'merge=quality,samplerate,overlap&latestupdate', [(12,32)]),
            (ctTXT, 'query?net=AM&format=request', []),
            (ctTXT, 'queryauth?net=AM&station=R0F05&includerestricted=true',
             []),

        ]
        for test in tests:
            ct, q, ignoreRanges = test
            auth = None
            if q.startswith('queryauth') or q.startswith('extentauth'):
                auth = dAuth
            self.testGET('{}{}'.format(query, q), ct, auth=auth,
                         dataFile=resFile.format(i), testID=i,
                         ignoreRanges=ignoreRanges)
            i += 1




#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestAvailability()
    sys.exit(app())



# vim: ts=4 et tw=79
