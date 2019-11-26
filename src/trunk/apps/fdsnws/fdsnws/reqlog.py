from __future__ import absolute_import, division, print_function

import os
import time
import datetime
import json
import hashlib
import subprocess
import logging
import logging.handlers
import GeoIP
import threading

mutex = threading.Lock()


class MyFileHandler(logging.handlers.TimedRotatingFileHandler):
    def __init__(self, filename):
        super(MyFileHandler, self).__init__(
            filename, when="midnight", utc=True)

    # The rotate() method is missing in Python 2, must override doRollover()
    # def rotate(self, source, dest):
    #     super(MyFileHandler, self).rotate(source, dest)
    #
    #     if os.path.exists(dest):
    #         subprocess.Popen(["bzip2", dest])

    def doRollover(self):
        t = self.rolloverAt - self.interval
        super(MyFileHandler, self).doRollover()

        currentTime = int(time.time())
        dstNow = time.localtime(currentTime)[-1]

        if self.utc:
            timeTuple = time.gmtime(t)

        else:
            timeTuple = time.localtime(t)
            dstThen = timeTuple[-1]

            if dstNow != dstThen:
                if dstNow:
                    addend = 3600

                else:
                    addend = -3600

                timeTuple = time.localtime(t + addend)

        dfn = self.baseFilename + "." + time.strftime(self.suffix, timeTuple)

        if os.path.exists(dfn):
            subprocess.Popen(["bzip2", dfn])


class Tracker(object):
    def __init__(self, logger, geoip, service, userName, userIP, clientID):
        self.__logger = logger
        self.__userName = userName

        if userName:
            userID = int(hashlib.md5(userName.lower()).hexdigest()[:8], 16)
        else:
            userID = int(hashlib.md5(userIP).hexdigest()[:8], 16)

        self.__data = {
            'service': service,
            'userID': userID,
            'clientID': clientID,
            'userEmail': None,
            'userLocation': {
                'country': geoip.country_code_by_addr(userIP)
            },
            'created': datetime.datetime.utcnow().isoformat() + 'Z'
        }

        if (userName and userName.lower().endswith("@gfz-potsdam.de")) or userIP.startswith("139.17."):
            self.__data['userLocation']['institution'] = "GFZ"

    def line_status(self, start_time, end_time, network, station, channel,
                    location, restricted, net_class, shared, constraints, volume, status, size, message):

        try:
            trace = self.__data['trace']

        except KeyError:
            trace = []
            self.__data['trace'] = trace

        trace.append({
            'net': network,
            'sta': station,
            'loc': location,
            'cha': channel,
            'start': start_time.iso(),
            'end': end_time.iso(),
            'restricted': restricted,
            'status': status,
            'bytes': size
        })

        if restricted and status == 'OK':
            self.__data['userEmail'] = self.__userName

    # FDSNWS requests have one volume, so volume_status() is called once per request
    def volume_status(self, volume, status, size, message):
        self.__data['status'] = status
        self.__data['bytes'] = size
        self.__data['finished'] = datetime.datetime.utcnow().isoformat() + 'Z'

    def request_status(self, status, message):
        with mutex:
            self.__logger.info(json.dumps(self.__data))


class RequestLog(object):
    def __init__(self, filename):
        self.__logger = logging.getLogger("seiscomp3.fdsnws.reqlog")
        self.__logger.addHandler(MyFileHandler(filename))
        self.__logger.setLevel(logging.INFO)
        self.__geoip = GeoIP.new(GeoIP.GEOIP_MEMORY_CACHE)

    def tracker(self, service, userName, userIP, clientID):
        return Tracker(self.__logger, self.__geoip, service, userName, userIP, clientID)
