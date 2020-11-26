#!/usr/bin/env python

###############################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
###############################################################################

from __future__ import absolute_import, division, print_function

import os
import socket
import subprocess
import sys
import time
import traceback

from threading import Thread
from datetime import datetime, timedelta

import requests

if sys.version_info[0] < 3:
    from Queue import Queue #pylint: disable=E0401
else:
    from queue import Queue

from fdsnws.utils import py3bstr # pylint: disable=C0413

###############################################################################
class FDSNWSTest:

    #--------------------------------------------------------------------------
    def __init__(self, port=8080):
        self.port = port
        self.url = 'http://localhost:{0}/fdsnws'.format(self.port)
        self.service = None
        self.rootdir = os.environ.get('SEISCOMP_ROOT')
        self.sharedir = '{0}/share/fdsnws'.format(self.rootdir)


    #--------------------------------------------------------------------------
    def __call__(self):
        if not self._startService():
            return 1

        try:
            self.test()
        except Exception:
            traceback.print_exc()

            self._stopService()
            return 1

        self._stopService()
        return 0


    #--------------------------------------------------------------------------
    def _waitForSocket(self, timeout=10):
        print('waiting for port {0} to become ready '.format(self.port),
              end='')
        maxTime = datetime.now() + timedelta(timeout)
        while self.service is not None and self.service.poll() is None:
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
            fdOut = open('fdsnws.stdout', 'w')
            fdErr = open('fdsnws.stderr', 'w')
            self.service = subprocess.Popen(cmd, stdout=fdOut, stderr=fdErr)
        except Exception as e:
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
        print('stopping FDSNWS service (PID: {0}): '.format(self.service.pid),
              end='')
        maxTime = datetime.now() + timedelta(timeout)

        self.service.terminate()
        while self.service.poll() is None:
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
            sys.executable, '{0}/../../fdsnws.py'.format(self.rootdir),
            '--debug', '--plugins=dbsqlite3,fdsnxml',
            '--database=sqlite3://{0}/seiscomp3.sqlite3'.format(self.rootdir),
            '--serveAvailability=true', '--dataAvailability.enable=true',
            '--agencyID=Test',
            '--record-url=sdsarchive://{0}/sds'.format(self.rootdir),
            '--htpasswd={0}/fdsnws.htpasswd'.format(self.rootdir),
            '--stationFilter={0}/stationFilter.cfg'.format(self.rootdir)
        ]



    #--------------------------------------------------------------------------
    @staticmethod
    def diff(expected, got, ignoreRanges):
        if expected == got:
            return (None, None)
        lenExp = minLen = maxLen = len(expected)
        lenGot = len(got)
        if ignoreRanges:
            for r in ignoreRanges:
                if len(r) > 2:
                    minLen -= r[2]
                if len(r) > 3:
                    maxLen += r[3]

        if lenGot == 0 and minLen <= 0:
            return (None, None)
        if lenGot < minLen or lenGot > maxLen:
            return (min(lenExp, lenGot), 'read {0} bytes, expected {1}'\
                    .format(lenGot, minLen if minLen == maxLen \
                                else '{0}-{1}'.format(minLen, maxLen)))

        # offset between got and expected index may result from variable length
        # result data, e.g. microseconds of time stamps
        iGot = iExp = 0
        while iExp < lenExp:
            if iGot >= lenGot:
                iGot = lenGot + 1
                break

            if got[iGot] == expected[iExp]:
                iExp += 1
                iGot += 1
                continue

            # bytes do not match, check ignore Range
            ignoreRange = None
            if ignoreRanges:
                for r in ignoreRanges:
                    if r[0] <= iExp < r[1]:
                        ignoreRange = r
                        break

            if ignoreRange:
                rEnd = ignoreRange[1]
                rLeft = rEnd - iExp
                rFewer = ignoreRange[2] if len(ignoreRange) > 2 else 0
                rMore = ignoreRange[3] if len(ignoreRange) > 3 else 0
                varLen = rFewer + rMore

                # advance expected pointer behind range
                iExp = rEnd
                exp = expected[iExp] if iExp < lenExp else None

                # static range length: advance got pointer behind range
                if varLen == 0:
                    iGot += rLeft
                    continue

                # dynamic ignore range length: search end of range indicated
                # by current exp pointer but limited by rLenDiff
                iGot += min(rLeft, rLeft - rFewer)

                # expected data ends on ignore range
                if exp is None:
                    iGot += min(lenGot-iGot, varLen)
                    continue

                # search range end in data
                pos = got[iGot:iGot+varLen+1].find(exp)
                if pos >= 0:
                    iGot += pos
                    continue

            return (iGot, '... [ {0} ] != [ {1} ] ...'\
                    .format(got[max(0, iGot-10):min(lenGot, iGot+11)],
                            expected[max(0, iExp-10):min(lenExp, iExp+11)]))

        if iGot < lenGot:
            return (lenGot, 'read {0} more bytes than expected' \
                            .format(lenGot-iGot))
        if iGot > lenGot:
            return (lenGot, 'read {0} fewer bytes than expected' \
                            .format(iGot-lenGot))

        # should not happen
        return (None, None)


    #--------------------------------------------------------------------------
    def testGET(self, url, contentType='text/html', ignoreRanges=None,
                concurrent=False, retCode=200, testID=None, auth=False,
                data=None, dataFile=None, diffContent=True, silent=False):
        if concurrent:
            self.testGETConcurrent(url, contentType, data, dataFile, retCode,
                                   testID, ignoreRanges, auth, diffContent)
        else:
            self.testGETOneShot(url, contentType, data, dataFile, retCode,
                                testID, ignoreRanges, auth, diffContent,
                                silent)


    #--------------------------------------------------------------------------
    def testGETOneShot(self, url, contentType='text/html', data=None,
                       dataFile=None, retCode=200, testID=None,
                       ignoreRanges=None, auth=False, diffContent=True,
                       silent=False):
        if not silent:
            if testID is not None:
                print('#{0} '.format(testID), end='')
            print('{0}: '.format(url), end='')
        stream = dataFile is not None
        dAuth = requests.auth.HTTPDigestAuth('sysop', 'sysop') if auth else None
        r = requests.get(url, stream=stream, auth=dAuth)
        if r.status_code != retCode:
            raise ValueError('Invalid status code, expected "{0}", got "{1}"' \
                             .format(retCode, r.status_code))

        if contentType != r.headers['content-type']:
            raise ValueError('Invalid content type, expected "{0}", got "{1}"' \
                             .format(contentType, r.headers['content-type']))

        expected = None
        if data is not None:
            expected = py3bstr(data)
        elif dataFile is not None:
            with open(dataFile, 'rb') as f:
                expected = f.read()


        if expected is not None:
            if diffContent:
                errPos, errMsg = self.diff(expected, r.content, ignoreRanges)
                if errPos is not None:
                    raise ValueError('Unexpected content at byte {0}: {1}' \
                                     .format(errPos, errMsg))
            else:
                if len(expected) != len(r.content):
                    raise ValueError('Unexpected content length, expected {0}, '
                                     'got {1}'.format(len(expected),
                                                      len(r.content)))

        if not silent:
            print('OK')
        sys.stdout.flush()


    #--------------------------------------------------------------------------
    def testGETConcurrent(self, url, contentType='text/html', data=None,
                          dataFile=None, retCode=200, testID=None,
                          ignoreRanges=None, auth=False, diffContent=True,
                          repetitions=1000, numThreads=10):
        if testID is not None:
            print('#{0} '.format(testID), end='')
        print('concurrent [{0}/{1}] {2}: '.format(repetitions, numThreads, url),
              end='')
        sys.stdout.flush()

        def doWork():
            while True:
                try:
                    i = q.get()
                    if i is None:
                        break
                    self.testGETOneShot(url, contentType, data, dataFile,
                                        retCode, testID, ignoreRanges, auth,
                                        diffContent, True)
                    print('.', end='')
                    sys.stdout.flush()
                except ValueError as e:
                    errors.append("error in job #{0}: {1}".format(i, str(e)))
                finally:
                    q.task_done()

        # queue
        q = Queue()
        errors = []

        # start worker threads
        threads = []
        for i in range(numThreads):
            t = Thread(target=doWork)
            t.start()
            threads.append(t)

        # populate queue with work
        for i in range(repetitions):
            q.put(i)
        q.join()

        # stop worker
        for i in range(numThreads):
            q.put(None)
        for t in threads:
            t.join()

        if errors:
            raise ValueError("{0} errors occured, first one is: {1}" \
                             .format(len(errors), errors[0]))

        print(' OK')
        sys.stdout.flush()



# vim: ts=4 et tw=79
