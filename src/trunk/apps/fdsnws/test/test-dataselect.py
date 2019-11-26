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
class TestDataSelect(FDSNWSTest):

    #--------------------------------------------------------------------------
    def test(self):
        print('Testing dataselect service')

        query = '{}/dataselect/1/query'.format(self.url)
        ctTXT = 'text/plain'
        ctMSeed = 'application/vnd.fdsn.mseed'
        resFile = self.rootdir + '/results/dataselect-{}.mseed'

        dAuth = HTTPDigestAuth('sysop', 'sysop')

        i = 1
        self.testGET('{}{}'.format(query, '?station=R0F05'), ctTXT,
                     retCode=403, testID=i)
        i += 1
        tests = [
            '?channel=EHZ',
            '?net=AM&sta=R187C&loc=00&cha=EHZ&starttime=2019-08-02T18:00:30&' \
                'endtime=2019-08-02T18:00:40',
            'auth?network=AM&station=R0F05&starttime=2019-08-02T12:00:00',
        ]
        for test in tests:
            self.testGET('{}{}'.format(query, test), ctMSeed,
                         dataFile=resFile.format(i), testID=i,
                         auth=dAuth if test.startswith('auth') else None)
            i += 1




#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestDataSelect()
    sys.exit(app())



# vim: ts=4 et tw=79
