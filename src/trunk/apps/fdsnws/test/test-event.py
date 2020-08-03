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
class TestEvent(FDSNWSTest):

    #--------------------------------------------------------------------------
    def test(self):
        print('Testing event service')

        query = '{0}/event/1/query'.format(self.url)
        ctTXT = 'text/plain'
        ctXML = 'application/xml'
        resFile = self.rootdir + '/results/event-'

        i = 1
        tests = [
            ('?format=text', ctTXT, False),
            ('?format=text&minmag=3&mindepth=20&minlon=150', ctTXT, False),
            ('?format=text&eventtype=earthquake,unknown', ctTXT, False),
            ('?format=sc3ml&eventid=rs2019qsodmc&formatted=true', ctXML, True),
            ('?format=sc3ml&includeallorigins=true&includeallmagnitudes=true&includearrivals=true&includefocalmechanism=true&includestationmts=true&includecomments=true&eventid=rs2019qsodmc&formatted=true', ctXML, False),
            ('', ctXML, False),
            ('?format=qml-rt', ctXML, False),
            ('?format=csv', ctTXT, False),
        ]
        for q, ct, concurrent in tests:
            self.testGET('{0}{1}'.format(query, q), ct, [], concurrent,
                         dataFile='{0}{1}.txt'.format(resFile, i), testID=i)
            i += 1



#------------------------------------------------------------------------------
if __name__ == '__main__':
    app = TestEvent()
    sys.exit(app())



# vim: ts=4 et tw=79
