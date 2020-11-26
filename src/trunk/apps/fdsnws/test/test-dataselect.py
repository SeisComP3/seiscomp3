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
class TestDataSelect(FDSNWSTest):

    #--------------------------------------------------------------------------
    def test(self):
        print('Testing dataselect service')

        query = '{0}/dataselect/1/query'.format(self.url)
        ctTXT = 'text/plain'
        ctMSeed = 'application/vnd.fdsn.mseed'
        resFile = self.rootdir + '/results/dataselect-{0}.mseed'

        i = 1
        self.testGET('{0}{1}'.format(query, '?station=R0F05'), ctTXT,
                     retCode=403, testID=i)
        i += 1
        tests = [
            ('?channel=EHZ', False),
            ('?net=AM&sta=R187C&loc=00&cha=EHZ&starttime=2019-08-02T18:00:30&endtime=2019-08-02T18:00:40', False),
            ('auth?network=AM&station=R0F05&starttime=2019-08-02T12:00:00', True),
        ]
        for q, concurrent in tests:
            self.testGET('{0}{1}'.format(query, q), ctMSeed, [], concurrent,
                         dataFile=resFile.format(i), testID=i,
                         auth=q.startswith('auth'))
            i += 1


#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestDataSelect()
    sys.exit(app())



# vim: ts=4 et tw=79
