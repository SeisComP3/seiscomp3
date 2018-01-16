from twisted.internet import reactor
from seiscomp3 import Core, Communication, DataModel


def callFromThread(f):
    def wrap(*args, **kwargs):
        reactor.callFromThread(f, *args, **kwargs)
    return wrap


def enableNotifier(f):
    def wrap(*args, **kwargs):
        saveState = DataModel.Notifier.IsEnabled()
        DataModel.Notifier.SetEnabled(True)
        f(*args, **kwargs)
        DataModel.Notifier.SetEnabled(saveState)
    return wrap


class RequestTrackerDB(object):

    def __init__(self, appName, msgConn, req_id, req_type, user, header, label, user_ip, client_ip):
        self.msgConn = msgConn
        self.arclinkRequest = DataModel.ArclinkRequest.Create()
        self.arclinkRequest.setCreated(Core.Time.GMT())
        self.arclinkRequest.setRequestID(req_id)
        self.arclinkRequest.setUserID(str(user))
        self.arclinkRequest.setClientID(appName)
        if user_ip: self.arclinkRequest.setUserIP(user_ip)
        if client_ip: self.arclinkRequest.setClientIP(client_ip)
        self.arclinkRequest.setType(req_type)
        self.arclinkRequest.setLabel(label)
        self.arclinkRequest.setHeader(header)

        self.averageTimeWindow = Core.TimeSpan(0.)
        self.totalLineCount = 0
        self.okLineCount = 0

        self.requestLines = []
        self.statusLines = []


    def send(self):
        msg = DataModel.Notifier.GetMessage(True)
        if msg:
            self.msgConn.send("LOGGING", msg)


    def line_status(self, start_time, end_time, network, station, channel,
        location, restricted, net_class, shared, constraints, volume, status, size, message):
        if network is None or network == "":
            network = "."
        if station is None or station == "":
            station = "."
        if channel is None or channel == "":
            channel = "."
        if location is None or location == "":
            location = "."
        if volume is None:
            volume = "NODATA"
        if size is None:
            size = 0
        if message is None:
            message = ""

        if isinstance(constraints, list):
            constr = " ".join(constraints)
        else:
            constr = " ".join([ a+"="+b for (a, b) in constraints.iteritems() ])

        arclinkRequestLine = DataModel.ArclinkRequestLine()
        arclinkRequestLine.setStart(start_time)
        arclinkRequestLine.setEnd(end_time)
        arclinkRequestLine.setStreamID(DataModel.WaveformStreamID(network[:8], station[:8], location[:8], channel[:8], ""))
        arclinkRequestLine.setConstraints(constr)
        if isinstance(restricted, bool): arclinkRequestLine.setRestricted(restricted)
        arclinkRequestLine.setNetClass(net_class)
        if isinstance(shared, bool):arclinkRequestLine.setShared(shared)
        #
        arclinkStatusLine = DataModel.ArclinkStatusLine()
        arclinkStatusLine.setVolumeID(volume)
        arclinkStatusLine.setStatus(status)
        arclinkStatusLine.setSize(size)
        arclinkStatusLine.setMessage(message)
        #
        arclinkRequestLine.setStatus(arclinkStatusLine)
        self.requestLines.append(arclinkRequestLine)

        self.averageTimeWindow += end_time - start_time
        self.totalLineCount += 1
        if status == "OK": self.okLineCount += 1


    def volume_status(self, volume, status, size, message):
        if volume is None:
            volume = "NODATA"
        if size is None:
            size = 0
        if message is None:
            message = ""

        arclinkStatusLine = DataModel.ArclinkStatusLine()
        arclinkStatusLine.setVolumeID(volume)
        arclinkStatusLine.setStatus(status)
        arclinkStatusLine.setSize(size)
        arclinkStatusLine.setMessage(message)
        self.statusLines.append(arclinkStatusLine)


    @callFromThread
    @enableNotifier
    def request_status(self, status, message):
        if message is None:
            message = ""

        self.arclinkRequest.setStatus(status)
        self.arclinkRequest.setMessage(message)

        ars = DataModel.ArclinkRequestSummary()
        tw = self.averageTimeWindow.seconds()
        if self.totalLineCount > 0:
            tw = self.averageTimeWindow.seconds() / self.totalLineCount # avarage request time window
        if tw >= 2**31: tw = -1 # prevent 32bit int overflow
        ars.setAverageTimeWindow(tw)
        ars.setTotalLineCount(self.totalLineCount)
        ars.setOkLineCount(self.okLineCount)
        self.arclinkRequest.setSummary(ars)

        al = DataModel.ArclinkLog()
        al.add(self.arclinkRequest)

        for obj in self.requestLines:
            self.arclinkRequest.add(obj)

        for obj in self.statusLines:
            self.arclinkRequest.add(obj)

        self.send()


    def __verseed_errors(self, volume):
        pass

    def verseed(self, volume, file):
        pass


