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
        # (contentType, URL, ignoreChars, concurrent)
        tests = [
            ('extent', ctTXT, [], False),
            ('extent?merge=samplerate', ctTXT, [], False),
            ('extent?merge=quality', ctTXT, [], True),
            ('extent?merge=quality,samplerate', ctTXT, [], False),
            ('extent?starttime=2019-08-20T00:00:00&orderby=latestupdate', ctTXT, [], False),
            ('extent?network=AM&channel=HDF&orderby=timespancount_desc', ctTXT, [], False),
            ('extentauth?station=R0F05&orderby=latestupdate_desc&includerestricted=true&format=geocsv', ctCSV, [], False),
            ('extent?orderby=latestupdate_desc&format=request', ctTXT, [], False),
            ('extentauth?orderby=latestupdate_desc&includerestricted=true&format=json&merge=quality', ctJSON, [(12,32)], True),
            ('query?net=AM', ctTXT, [], False),
            ('query?net=AM&mergegaps=10.0', ctTXT, [], False),
            ('query?net=AM&mergegaps=10.0&merge=overlap', ctTXT, [], False),
            ('query?net=AM&mergegaps=10.0&merge=overlap,samplerate', ctTXT, [], True),
            ('query?net=AM&mergegaps=10.0&merge=overlap,samplerate,quality', ctTXT, [], False),
            ('query?net=AM&show=latestupdate&limit=3', ctTXT, [], False),
            ('query?net=AM&format=geocsv&show=latestupdate', ctCSV, [], False),
            ('query?net=AM&format=json&show=latestupdate', ctJSON, [(12,32)], False),
            ('query?net=AM&channel=HDF&format=json&merge=quality,samplerate,overlap&latestupdate', ctJSON, [(12,32)], False),
            ('query?net=AM&format=request', ctTXT, [], False),
            ('queryauth?net=AM&station=R0F05&includerestricted=true', ctTXT, [], True),

        ]
        for q, ct, ignoreRanges, concurrent in tests:
            auth = None
            if q.startswith('queryauth') or q.startswith('extentauth'):
                auth = dAuth
            self.testGET('{}{}'.format(query, q), ct, ignoreRanges, concurrent,
                         auth=auth, dataFile=resFile.format(i), testID=i)
            i += 1


#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestAvailability()
    sys.exit(app())



# vim: ts=4 et tw=79
