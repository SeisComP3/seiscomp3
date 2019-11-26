################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Thread-safe file logger
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

from __future__ import absolute_import, division, print_function

import os
import sys
import time
import threading

if sys.version_info[0] < 3:
    from Queue import Queue
else:
    from queue import Queue

#-------------------------------------------------------------------------------


def _worker(log):
    while True:
        msg = log._queue.get()
        log._write(str(msg))
        log._queue.task_done()


################################################################################
class Log:

    #---------------------------------------------------------------------------
    def __init__(self, filePath, archiveSize=7):
        self._filePath = filePath
        self._basePath = os.path.dirname(filePath)
        self._fileName = os.path.basename(filePath)
        self._archiveSize = archiveSize
        self._queue = Queue()
        self._lastLogTime = None
        self._fd = None

        if self._archiveSize < 0:
            self._archiveSize = 0

        # worker thread, responsible for writing messages to file
        t = threading.Thread(target=_worker, args=(self,))
        t.daemon = True
        t.start()

    #---------------------------------------------------------------------------
    def __del__(self):
        # wait for worker thread to write all pending log messages
        self._queue.join()

        if self._fd is not None:
            self._fd.close()

    #---------------------------------------------------------------------------
    def log(self, msg):
        self._queue.put(msg)

    #---------------------------------------------------------------------------
    def _rotate(self):
        self._fd.close()
        self._fd = None

        try:
            pattern = self._filePath + ".%i"
            for i in range(self._archiveSize, 1, -1):
                src = pattern % (i-1)
                if os.path.isfile(src):
                    os.rename(pattern % (i-1), pattern % i)
            os.rename(self._filePath, pattern % 1)
        except Exception as e:
            print("failed to rotate access log: %s\n" % str(e), file=sys.stderr)

        self._fd = open(self._filePath, 'w')

    #---------------------------------------------------------------------------
    def _write(self, msg):
        try:
            now = time.localtime()
            if self._fd is None:
                if self._basePath and not os.path.exists(self._basePath):
                    os.makedirs(self._basePath)
                self._fd = open(self._filePath, 'a')
            elif self._archiveSize > 0 and self._lastLogTime is not None and \
                (self._lastLogTime.tm_yday != now.tm_yday or
                 self._lastLogTime.tm_year != now.tm_year):
                self._rotate()

            self._fd.write("%s\n" % msg)
            self._fd.flush()
            self._lastLogTime = now
        except Exception as e:
            print("access log: %s\n" % str(e), file=sys.stderr)


# vim: ts=4 et
