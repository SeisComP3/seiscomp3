#!/usr/bin/env python

###############################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
###############################################################################

from __future__ import print_function

import os
import requests
import signal
import socket
import subprocess
import sys
import time

from datetime import datetime, timedelta


###############################################################################
class FDSNWSTest:

    #--------------------------------------------------------------------------
    def __init__(self, port=8080):
        self.port = port
        self.url = 'http://localhost:{}/fdsnws'.format(self.port)
        self.service = None
        self.rootdir = os.environ.get('SEISCOMP_ROOT')
        self.sharedir = '{}/share/fdsnws'.format(self.rootdir)


    #--------------------------------------------------------------------------
    def __call__(self):
        if not self._startService():
            return 1

        try:
            self.test()
        except Exception as e:
            print(str(e))

            self._stopService()
            return 1

        self._stopService()
        return 0


    #--------------------------------------------------------------------------
    def _waitForSocket(self, timeout=10):
        print('waiting for port {} to become ready '.format(self.port),
              end='')
        maxTime = datetime.now() + timedelta(timeout)
        while self.service is not None and self.service.poll() == None:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            res = sock.connect_ex(('127.0.0.1', self.port))
            sock.close()
            if res == 0:
                print(' OK')
                return True

            if datetime.now() > maxTime:
                print(' TIMEOUT EXCEEDED')
                return False
            time.sleep(0.2)
            print('.', end='')

        print(' SERVICE TERMINATED')


    #--------------------------------------------------------------------------
    def _startService(self):
        cmd = self.command()
        print('starting FDSNWS service:', ' '.join(cmd))
        try:
            self.fdOut = open('fdsnws.stdout', 'w')
            self.fdErr = open('fdsnws.stderr', 'w')
            self.service = subprocess.Popen(cmd, stdout=self.fdOut,
                                            stderr=self.fdErr)
        except Exception, e:
            print('failed to start FDSNWS service:', str(e))
            return False

        if not self._waitForSocket():
            self._stopService()
            return False

        return True


    #--------------------------------------------------------------------------
    def _stopService(self, timeout=10):
        if self.service.poll() is not None:
            print('warning: FDSNWS service terminated ahead of time',
                  file=sys.stdout)
            return
        print('stopping FDSNWS service (PID: {}): '.format(self.service.pid),
              end='')
        maxTime = datetime.now() + timedelta(timeout)

        self.service.terminate()
        while self.service.poll() == None:
            print('.', end='')
            time.sleep(0.2)
            if datetime.now() > maxTime:
                print(' TIMEOUT EXCEEDED, sending kill signal',
                      file=sys.stdout)
                self.service.kill()
                return

        print(' OK')


    #--------------------------------------------------------------------------
    def test(self):
        pass


    #--------------------------------------------------------------------------
    def command(self):
        return [
            'python', '{}/../../fdsnws.py'.format(self.rootdir),
            '--verbosity=4',
            '--plugins=dbsqlite3,fdsnxml',
            '--database=sqlite3://{}/seiscomp3.sqlite3'.format(self.rootdir),
            '--serveAvailability=true', '--dataAvailability.enable=true',
            '--agencyID=Test',
            '--record-url=sdsarchive://{}/sds'.format(self.rootdir),
            '--htpasswd={}/fdsnws.htpasswd'.format(self.rootdir)
        ]



    #--------------------------------------------------------------------------
    def diff(self, expected, got, offset, ignoreRanges):
        if expected == got:
            return (None, None)
        len1 = len(got)
        len2 = len(expected)
        if len1 != len2:
            return (min(len1,len2)+offset, 'read {} bytes, expected {}'.format(
                        len1+offset, len2+offset))

        for i in range(0, len1):
            if expected[i] != got[i]:
                ignore = False
                for r in ignoreRanges:
                    if i >= r[0] and i <= r[1]:
                        ignore = True
                        break
                if not ignore:
                    return (i+offset, '... [{}] != [{}] ...'.format(
                            got[max(i-10,0):i+10], expected[max(i-10,0):i+10]))

        # should not happen
        return (None, None)


    #--------------------------------------------------------------------------
    def testGET(self, url, contentType='text/html', data=None,
                dataFile=None, retCode=200, testID=None, ignoreRanges=[],
                auth=None):
        if testID is not None:
            print('#{} '.format(testID), end='')
        print('{}: '.format(url), end='')
        stream = False if dataFile is None else True
        r = requests.get(url, stream=stream, auth=auth)
        if r.status_code != retCode:
            raise ValueError('Invalid status code, expected "{}", got "{}"' \
                             .format(retCode, r.status_code))

        if contentType != r.headers['content-type']:
            raise ValueError('Invalid content type, expected "{}", got "{}"' \
                             .format(contentType, r.headers['content-type']))

        if data is not None:
            errPos, errMsg = self.diff(r.content, data, 0, ignoreRanges)
            if errPos is not None:
                raise ValueError('Unexpected content at byte {}: {}'.format(
                                 errPos, errMsg))
        elif dataFile is not None:
            bufSize = 4096
            bytesRead = 0
            with open(dataFile, 'rb') as f:
                while True:
                    b1 = r.raw.read(bufSize)
                    b2 = f.read(bufSize)
                    errPos, errMsg = self.diff(b1, b2, bytesRead, ignoreRanges)
                    if errPos is not None:
                        raise ValueError('Unexpected content at byte {}: {}'\
                                         .format(errPos, errMsg))
                    if not b1:
                        break
                    bytesRead += len(b1)

        print('OK')
        sys.stdout.flush()


# vim: ts=4 et tw=79
