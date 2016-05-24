# This file was created by a source code generator:
# genxml2wrap.py 
# Do not modify. Change the definition and
# run the generator again!
#
# (c) 2010 Mathias Hoffmann, GFZ Potsdam
#
#
import re
import time
import datetime
from seiscomp import logs
from seiscomp3 import DataModel, Core, Config

#(de)serialize real array string
def str2RealArray(arg):
	RA = DataModel.RealArray()
	if not arg:
		return RA
	sarg = arg.split()
	for s in sarg:
		if len(s):
			RA.content().push_back(float(s))
	return RA
def RealArray2str(arg):
	tmp = str()
	for r in arg:
		tmp += str(r)+' '
	return tmp.strip()

#de-serialize complex array string
def str2ComplexArray(arg):
	CA = DataModel.ComplexArray()
	if not arg:
		return CA
	sarg = re.findall(r'\((.+?)\)', arg)
	for s in sarg:
		r,i = s.split(',')
		if len(r) and len(i):
			CA.content().push_back(complex(float(r), float(i)))
	return CA
def ComplexArray2str(arg):
	tmp = str()
	for c in arg:
		tmp += '('+str(c.real)+','+str(c.imag)+') '
	return tmp.strip()
#
#


# package: QualityControl
class base_qclog(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_networkCode(self):
        try:
            return self.obj.waveformID().networkCode()
        except Core.ValueException:
            return None
    def __get_stationCode(self):
        try:
            return self.obj.waveformID().stationCode()
        except Core.ValueException:
            return None
    def __get_streamCode(self):
        try:
            return self.obj.waveformID().channelCode()
        except Core.ValueException:
            return None
    def __get_locationCode(self):
        try:
            return self.obj.waveformID().locationCode()
        except Core.ValueException:
            return None
    def __set_networkCode(self, arg):
        if self.__get_networkCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setNetworkCode(arg)
    def __set_stationCode(self, arg):
        if self.__get_stationCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setStationCode(arg)
    def __set_streamCode(self, arg):
        if self.__get_streamCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setChannelCode(arg)
    def __set_locationCode(self, arg):
        if self.__get_locationCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setLocationCode(arg)
    networkCode = property(__get_networkCode, __set_networkCode)
    stationCode = property(__get_stationCode, __set_stationCode)
    streamCode = property(__get_streamCode, __set_streamCode)
    locationCode = property(__get_locationCode, __set_locationCode)

    def __get_creatorID(self):
        try: # @return: const std::string&
            return self.obj.creatorID()
        except Core.ValueException:
            return None
    def __set_creatorID(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_creatorID() != value:
            self._needsUpdate = True
        self.obj.setCreatorID(value)
    creatorID = property(__get_creatorID, __set_creatorID)

    def __get_created(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.created().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_created(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_created()) != str(arg):
            self._needsUpdate = True
        self.obj.setCreated(value)
    created = property(__get_created, __set_created)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_message(self):
        try: # @return: const std::string&
            return self.obj.message()
        except Core.ValueException:
            return None
    def __set_message(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_message() != value:
            self._needsUpdate = True
        self.obj.setMessage(value)
    message = property(__get_message, __set_message)


# package: QualityControl
class base_waveformquality(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_networkCode(self):
        try:
            return self.obj.waveformID().networkCode()
        except Core.ValueException:
            return None
    def __get_stationCode(self):
        try:
            return self.obj.waveformID().stationCode()
        except Core.ValueException:
            return None
    def __get_streamCode(self):
        try:
            return self.obj.waveformID().channelCode()
        except Core.ValueException:
            return None
    def __get_locationCode(self):
        try:
            return self.obj.waveformID().locationCode()
        except Core.ValueException:
            return None
    def __set_networkCode(self, arg):
        if self.__get_networkCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setNetworkCode(arg)
    def __set_stationCode(self, arg):
        if self.__get_stationCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setStationCode(arg)
    def __set_streamCode(self, arg):
        if self.__get_streamCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setChannelCode(arg)
    def __set_locationCode(self, arg):
        if self.__get_locationCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setLocationCode(arg)
    networkCode = property(__get_networkCode, __set_networkCode)
    stationCode = property(__get_stationCode, __set_stationCode)
    streamCode = property(__get_streamCode, __set_streamCode)
    locationCode = property(__get_locationCode, __set_locationCode)

    def __get_creatorID(self):
        try: # @return: const std::string&
            return self.obj.creatorID()
        except Core.ValueException:
            return None
    def __set_creatorID(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_creatorID() != value:
            self._needsUpdate = True
        self.obj.setCreatorID(value)
    creatorID = property(__get_creatorID, __set_creatorID)

    def __get_created(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.created().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_created(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_created()) != str(arg):
            self._needsUpdate = True
        self.obj.setCreated(value)
    created = property(__get_created, __set_created)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_type(self):
        try: # @return: const std::string&
            return self.obj.type()
        except Core.ValueException:
            return None
    def __set_type(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_type() != value:
            self._needsUpdate = True
        self.obj.setType(value)
    type = property(__get_type, __set_type)

    def __get_parameter(self):
        try: # @return: const std::string&
            return self.obj.parameter()
        except Core.ValueException:
            return None
    def __set_parameter(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_parameter() != value:
            self._needsUpdate = True
        self.obj.setParameter(value)
    parameter = property(__get_parameter, __set_parameter)

    def __get_value(self):
        try: # @return: double
            return self.obj.value()
        except Core.ValueException:
            return None
    def __set_value(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_value() != value:
            self._needsUpdate = True
        self.obj.setValue(value)
    value = property(__get_value, __set_value)

    def __get_lowerUncertainty(self):
        # optional Attribute
        try: # @return: double
            return self.obj.lowerUncertainty()
        except Core.ValueException:
            return None
    def __set_lowerUncertainty(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_lowerUncertainty() != value:
            self._needsUpdate = True
        self.obj.setLowerUncertainty(value)
    lowerUncertainty = property(__get_lowerUncertainty, __set_lowerUncertainty)

    def __get_upperUncertainty(self):
        # optional Attribute
        try: # @return: double
            return self.obj.upperUncertainty()
        except Core.ValueException:
            return None
    def __set_upperUncertainty(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_upperUncertainty() != value:
            self._needsUpdate = True
        self.obj.setUpperUncertainty(value)
    upperUncertainty = property(__get_upperUncertainty, __set_upperUncertainty)

    def __get_windowLength(self):
        # optional Attribute
        try: # @return: double
            return self.obj.windowLength()
        except Core.ValueException:
            return None
    def __set_windowLength(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_windowLength() != value:
            self._needsUpdate = True
        self.obj.setWindowLength(value)
    windowLength = property(__get_windowLength, __set_windowLength)


# package: QualityControl
class base_outage(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_networkCode(self):
        try:
            return self.obj.waveformID().networkCode()
        except Core.ValueException:
            return None
    def __get_stationCode(self):
        try:
            return self.obj.waveformID().stationCode()
        except Core.ValueException:
            return None
    def __get_streamCode(self):
        try:
            return self.obj.waveformID().channelCode()
        except Core.ValueException:
            return None
    def __get_locationCode(self):
        try:
            return self.obj.waveformID().locationCode()
        except Core.ValueException:
            return None
    def __set_networkCode(self, arg):
        if self.__get_networkCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setNetworkCode(arg)
    def __set_stationCode(self, arg):
        if self.__get_stationCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setStationCode(arg)
    def __set_streamCode(self, arg):
        if self.__get_streamCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setChannelCode(arg)
    def __set_locationCode(self, arg):
        if self.__get_locationCode() != arg:
            self._needsUpdate = True
        self.obj.waveformID().setLocationCode(arg)
    networkCode = property(__get_networkCode, __set_networkCode)
    stationCode = property(__get_stationCode, __set_stationCode)
    streamCode = property(__get_streamCode, __set_streamCode)
    locationCode = property(__get_locationCode, __set_locationCode)

    def __get_creatorID(self):
        try: # @return: const std::string&
            return self.obj.creatorID()
        except Core.ValueException:
            return None
    def __set_creatorID(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_creatorID() != value:
            self._needsUpdate = True
        self.obj.setCreatorID(value)
    creatorID = property(__get_creatorID, __set_creatorID)

    def __get_created(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.created().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_created(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_created()) != str(arg):
            self._needsUpdate = True
        self.obj.setCreated(value)
    created = property(__get_created, __set_created)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)


# package: QualityControl
class base_qualitycontrol(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def _new_qclog(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.QCLog.Find(publicID): publicID = None
        if publicID: obj = DataModel.QCLog.Create(publicID)
        else: obj = DataModel.QCLog.Create()
        try: obj.setWaveformID(args["waveformID"])
        except KeyError: pass
        try: obj.setCreatorID(args["creatorID"])
        except KeyError: pass
        try:
            if args["created"] is None:
                obj.setCreated(None)
            else:
                obj.setCreated(Core.Time.FromString(str(args["created"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setMessage(args["message"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.QualityControl: error adding QCLog"
        return obj
    def __get_qclog(self):
        list = []
        if dbQuery is None:
            if (self.obj.qCLogCount()):
                for i in xrange(self.obj.qCLogCount()):
                    obj = self.obj.qCLog(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_qclog(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.QCLog.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.QCLog.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_qclog(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _qCLog = property(__get_qclog)

    def _new_waveformquality(self, **args):
        try: obj = DataModel.WaveformQuality()
        except KeyError: pass
        try: obj.setWaveformID(args["waveformID"])
        except KeyError: pass
        try: obj.setCreatorID(args["creatorID"])
        except KeyError: pass
        try:
            if args["created"] is None:
                obj.setCreated(None)
            else:
                obj.setCreated(Core.Time.FromString(str(args["created"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setType(args["type"])
        except KeyError: pass
        try: obj.setParameter(args["parameter"])
        except KeyError: pass
        try: obj.setValue(args["value"])
        except KeyError: pass
        try: obj.setLowerUncertainty(args["lowerUncertainty"])
        except KeyError: pass
        try: obj.setUpperUncertainty(args["upperUncertainty"])
        except KeyError: pass
        try: obj.setWindowLength(args["windowLength"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.QualityControl: error adding WaveformQuality"
        return obj
    def __get_waveformquality(self):
        list = []
        if dbQuery is None:
            if (self.obj.waveformQualityCount()):
                for i in xrange(self.obj.waveformQualityCount()):
                    obj = self.obj.waveformQuality(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_waveformquality(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.waveformQualityCount()
            while objects_left > 0:
                try:
                    obj = self.obj.waveformQuality(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_waveformquality(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_waveformquality(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_waveformquality(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _waveformQuality = property(__get_waveformquality)

    def _new_outage(self, **args):
        try: obj = DataModel.Outage()
        except KeyError: pass
        try: obj.setWaveformID(args["waveformID"])
        except KeyError: pass
        try: obj.setCreatorID(args["creatorID"])
        except KeyError: pass
        try:
            if args["created"] is None:
                obj.setCreated(None)
            else:
                obj.setCreated(Core.Time.FromString(str(args["created"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.QualityControl: error adding Outage"
        return obj
    def __get_outage(self):
        list = []
        if dbQuery is None:
            if (self.obj.outageCount()):
                for i in xrange(self.obj.outageCount()):
                    obj = self.obj.outage(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_outage(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.outageCount()
            while objects_left > 0:
                try:
                    obj = self.obj.outage(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_outage(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_outage(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_outage(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _outage = property(__get_outage)


# package: Inventory
class base_stationreference(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_stationID(self):
        try: # @return: const std::string&
            return self.obj.stationID()
        except Core.ValueException:
            return None
    def __set_stationID(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_stationID() != value:
            self._needsUpdate = True
        self.obj.setStationID(value)
    stationID = property(__get_stationID, __set_stationID)


# package: Inventory
class base_stationgroup(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_type(self):
        # optional Attribute
        try: # @return: StationGroupType
            return self.obj.type()
        except Core.ValueException:
            return None
    def __set_type(self, arg):
        if self.__get_type() != arg:
            self._needsUpdate = True
        self.obj.setType(arg)
    type = property(__get_type, __set_type)

    def __get_code(self):
        try: # @return: const std::string&
            return self.obj.code()
        except Core.ValueException:
            return None
    def __set_code(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_code() != value:
            self._needsUpdate = True
        self.obj.setCode(value)
    code = property(__get_code, __set_code)

    def __get_start(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_latitude(self):
        # optional Attribute
        try: # @return: double
            return self.obj.latitude()
        except Core.ValueException:
            return None
    def __set_latitude(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_latitude() != value:
            self._needsUpdate = True
        self.obj.setLatitude(value)
    latitude = property(__get_latitude, __set_latitude)

    def __get_longitude(self):
        # optional Attribute
        try: # @return: double
            return self.obj.longitude()
        except Core.ValueException:
            return None
    def __set_longitude(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_longitude() != value:
            self._needsUpdate = True
        self.obj.setLongitude(value)
    longitude = property(__get_longitude, __set_longitude)

    def __get_elevation(self):
        # optional Attribute
        try: # @return: double
            return self.obj.elevation()
        except Core.ValueException:
            return None
    def __set_elevation(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_elevation() != value:
            self._needsUpdate = True
        self.obj.setElevation(value)
    elevation = property(__get_elevation, __set_elevation)


    def _new_stationreference(self, **args):
        try: obj = DataModel.StationReference()
        except KeyError: pass
        try: obj.setStationID(args["stationID"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.StationGroup: error adding StationReference"
        return obj
    def __get_stationreference(self):
        list = []
        if dbQuery is None:
            if (self.obj.stationReferenceCount()):
                for i in xrange(self.obj.stationReferenceCount()):
                    obj = self.obj.stationReference(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_stationreference(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.stationReferenceCount()
            while objects_left > 0:
                try:
                    obj = self.obj.stationReference(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_stationreference(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_stationreference(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_stationreference(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _stationReference = property(__get_stationreference)


# package: Inventory
class base_auxsource(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_unit(self):
        try: # @return: const std::string&
            return self.obj.unit()
        except Core.ValueException:
            return None
    def __set_unit(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_unit() != value:
            self._needsUpdate = True
        self.obj.setUnit(value)
    unit = property(__get_unit, __set_unit)

    def __get_conversion(self):
        try: # @return: const std::string&
            return self.obj.conversion()
        except Core.ValueException:
            return None
    def __set_conversion(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_conversion() != value:
            self._needsUpdate = True
        self.obj.setConversion(value)
    conversion = property(__get_conversion, __set_conversion)

    def __get_sampleRateNumerator(self):
        # optional Attribute
        try: # @return: int
            return self.obj.sampleRateNumerator()
        except Core.ValueException:
            return None
    def __set_sampleRateNumerator(self, arg):
        if self.__get_sampleRateNumerator() != arg:
            self._needsUpdate = True
        self.obj.setSampleRateNumerator(arg)
    sampleRateNumerator = property(__get_sampleRateNumerator, __set_sampleRateNumerator)

    def __get_sampleRateDenominator(self):
        # optional Attribute
        try: # @return: int
            return self.obj.sampleRateDenominator()
        except Core.ValueException:
            return None
    def __set_sampleRateDenominator(self, arg):
        if self.__get_sampleRateDenominator() != arg:
            self._needsUpdate = True
        self.obj.setSampleRateDenominator(arg)
    sampleRateDenominator = property(__get_sampleRateDenominator, __set_sampleRateDenominator)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_auxdevice(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_model(self):
        try: # @return: const std::string&
            return self.obj.model()
        except Core.ValueException:
            return None
    def __set_model(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_model() != value:
            self._needsUpdate = True
        self.obj.setModel(value)
    model = property(__get_model, __set_model)

    def __get_manufacturer(self):
        try: # @return: const std::string&
            return self.obj.manufacturer()
        except Core.ValueException:
            return None
    def __set_manufacturer(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_manufacturer() != value:
            self._needsUpdate = True
        self.obj.setManufacturer(value)
    manufacturer = property(__get_manufacturer, __set_manufacturer)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


    def _new_auxsource(self, **args):
        try: obj = DataModel.AuxSource()
        except KeyError: pass
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setUnit(args["unit"])
        except KeyError: pass
        try: obj.setConversion(args["conversion"])
        except KeyError: pass
        try: obj.setSampleRateNumerator(args["sampleRateNumerator"])
        except KeyError: pass
        try: obj.setSampleRateDenominator(args["sampleRateDenominator"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.AuxDevice: error adding AuxSource"
        return obj
    def __get_auxsource(self):
        list = []
        if dbQuery is None:
            if (self.obj.auxSourceCount()):
                for i in xrange(self.obj.auxSourceCount()):
                    obj = self.obj.auxSource(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_auxsource(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.auxSourceCount()
            while objects_left > 0:
                try:
                    obj = self.obj.auxSource(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_auxsource(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_auxsource(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_auxsource(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _auxSource = property(__get_auxsource)


# package: Inventory
class base_sensorcalibration(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_serialNumber(self):
        try: # @return: const std::string&
            return self.obj.serialNumber()
        except Core.ValueException:
            return None
    def __set_serialNumber(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_serialNumber() != value:
            self._needsUpdate = True
        self.obj.setSerialNumber(value)
    serialNumber = property(__get_serialNumber, __set_serialNumber)

    def __get_channel(self):
        try: # @return: int
            return self.obj.channel()
        except Core.ValueException:
            return None
    def __set_channel(self, arg):
        if self.__get_channel() != arg:
            self._needsUpdate = True
        self.obj.setChannel(arg)
    channel = property(__get_channel, __set_channel)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_gainFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gainFrequency()
        except Core.ValueException:
            return None
    def __set_gainFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gainFrequency() != value:
            self._needsUpdate = True
        self.obj.setGainFrequency(value)
    gainFrequency = property(__get_gainFrequency, __set_gainFrequency)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_sensor(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_model(self):
        try: # @return: const std::string&
            return self.obj.model()
        except Core.ValueException:
            return None
    def __set_model(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_model() != value:
            self._needsUpdate = True
        self.obj.setModel(value)
    model = property(__get_model, __set_model)

    def __get_manufacturer(self):
        try: # @return: const std::string&
            return self.obj.manufacturer()
        except Core.ValueException:
            return None
    def __set_manufacturer(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_manufacturer() != value:
            self._needsUpdate = True
        self.obj.setManufacturer(value)
    manufacturer = property(__get_manufacturer, __set_manufacturer)

    def __get_type(self):
        try: # @return: const std::string&
            return self.obj.type()
        except Core.ValueException:
            return None
    def __set_type(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_type() != value:
            self._needsUpdate = True
        self.obj.setType(value)
    type = property(__get_type, __set_type)

    def __get_unit(self):
        try: # @return: const std::string&
            return self.obj.unit()
        except Core.ValueException:
            return None
    def __set_unit(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_unit() != value:
            self._needsUpdate = True
        self.obj.setUnit(value)
    unit = property(__get_unit, __set_unit)

    def __get_lowFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.lowFrequency()
        except Core.ValueException:
            return None
    def __set_lowFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_lowFrequency() != value:
            self._needsUpdate = True
        self.obj.setLowFrequency(value)
    lowFrequency = property(__get_lowFrequency, __set_lowFrequency)

    def __get_highFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.highFrequency()
        except Core.ValueException:
            return None
    def __set_highFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_highFrequency() != value:
            self._needsUpdate = True
        self.obj.setHighFrequency(value)
    highFrequency = property(__get_highFrequency, __set_highFrequency)

    def __get_response(self):
        try: # @return: const std::string&
            return self.obj.response()
        except Core.ValueException:
            return None
    def __set_response(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_response() != value:
            self._needsUpdate = True
        self.obj.setResponse(value)
    response = property(__get_response, __set_response)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


    def _new_sensorcalibration(self, **args):
        try: obj = DataModel.SensorCalibration()
        except KeyError: pass
        try: obj.setSerialNumber(args["serialNumber"])
        except KeyError: pass
        try: obj.setChannel(args["channel"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setGainFrequency(args["gainFrequency"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Sensor: error adding SensorCalibration"
        return obj
    def __get_sensorcalibration(self):
        list = []
        if dbQuery is None:
            if (self.obj.sensorCalibrationCount()):
                for i in xrange(self.obj.sensorCalibrationCount()):
                    obj = self.obj.sensorCalibration(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_sensorcalibration(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.sensorCalibrationCount()
            while objects_left > 0:
                try:
                    obj = self.obj.sensorCalibration(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_sensorcalibration(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_sensorcalibration(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_sensorcalibration(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _sensorCalibration = property(__get_sensorcalibration)


# package: Inventory
class base_responsepaz(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_type(self):
        try: # @return: const std::string&
            return self.obj.type()
        except Core.ValueException:
            return None
    def __set_type(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_type() != value:
            self._needsUpdate = True
        self.obj.setType(value)
    type = property(__get_type, __set_type)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_gainFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gainFrequency()
        except Core.ValueException:
            return None
    def __set_gainFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gainFrequency() != value:
            self._needsUpdate = True
        self.obj.setGainFrequency(value)
    gainFrequency = property(__get_gainFrequency, __set_gainFrequency)

    def __get_normalizationFactor(self):
        # optional Attribute
        try: # @return: double
            return self.obj.normalizationFactor()
        except Core.ValueException:
            return None
    def __set_normalizationFactor(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_normalizationFactor() != value:
            self._needsUpdate = True
        self.obj.setNormalizationFactor(value)
    normalizationFactor = property(__get_normalizationFactor, __set_normalizationFactor)

    def __get_normalizationFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.normalizationFrequency()
        except Core.ValueException:
            return None
    def __set_normalizationFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_normalizationFrequency() != value:
            self._needsUpdate = True
        self.obj.setNormalizationFrequency(value)
    normalizationFrequency = property(__get_normalizationFrequency, __set_normalizationFrequency)

    def __get_numberOfZeros(self):
        # optional Attribute
        try: # @return: int
            return self.obj.numberOfZeros()
        except Core.ValueException:
            return None
    def __set_numberOfZeros(self, arg):
        if self.__get_numberOfZeros() != arg:
            self._needsUpdate = True
        self.obj.setNumberOfZeros(arg)
    numberOfZeros = property(__get_numberOfZeros, __set_numberOfZeros)

    def __get_numberOfPoles(self):
        # optional Attribute
        try: # @return: int
            return self.obj.numberOfPoles()
        except Core.ValueException:
            return None
    def __set_numberOfPoles(self, arg):
        if self.__get_numberOfPoles() != arg:
            self._needsUpdate = True
        self.obj.setNumberOfPoles(arg)
    numberOfPoles = property(__get_numberOfPoles, __set_numberOfPoles)

    def __get_zeros(self):
        # optional Attribute
        try: # @return: ComplexArray
            return ComplexArray2str(self.obj.zeros().content())
        except Core.ValueException:
            return None
    def __set_zeros(self, arg):
        try: value = str2ComplexArray(arg)
        except: value = None
        ret = self.__get_zeros()
        if  not str2ComplexArray(ret) == value:
            self._needsUpdate = True
        self.obj.setZeros(value)
    zeros = property(__get_zeros, __set_zeros)

    def __get_poles(self):
        # optional Attribute
        try: # @return: ComplexArray
            return ComplexArray2str(self.obj.poles().content())
        except Core.ValueException:
            return None
    def __set_poles(self, arg):
        try: value = str2ComplexArray(arg)
        except: value = None
        ret = self.__get_poles()
        if  not str2ComplexArray(ret) == value:
            self._needsUpdate = True
        self.obj.setPoles(value)
    poles = property(__get_poles, __set_poles)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_responsepolynomial(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_gainFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gainFrequency()
        except Core.ValueException:
            return None
    def __set_gainFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gainFrequency() != value:
            self._needsUpdate = True
        self.obj.setGainFrequency(value)
    gainFrequency = property(__get_gainFrequency, __set_gainFrequency)

    def __get_frequencyUnit(self):
        try: # @return: const std::string&
            return self.obj.frequencyUnit()
        except Core.ValueException:
            return None
    def __set_frequencyUnit(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_frequencyUnit() != value:
            self._needsUpdate = True
        self.obj.setFrequencyUnit(value)
    frequencyUnit = property(__get_frequencyUnit, __set_frequencyUnit)

    def __get_approximationType(self):
        try: # @return: const std::string&
            return self.obj.approximationType()
        except Core.ValueException:
            return None
    def __set_approximationType(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_approximationType() != value:
            self._needsUpdate = True
        self.obj.setApproximationType(value)
    approximationType = property(__get_approximationType, __set_approximationType)

    def __get_approximationLowerBound(self):
        # optional Attribute
        try: # @return: double
            return self.obj.approximationLowerBound()
        except Core.ValueException:
            return None
    def __set_approximationLowerBound(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_approximationLowerBound() != value:
            self._needsUpdate = True
        self.obj.setApproximationLowerBound(value)
    approximationLowerBound = property(__get_approximationLowerBound, __set_approximationLowerBound)

    def __get_approximationUpperBound(self):
        # optional Attribute
        try: # @return: double
            return self.obj.approximationUpperBound()
        except Core.ValueException:
            return None
    def __set_approximationUpperBound(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_approximationUpperBound() != value:
            self._needsUpdate = True
        self.obj.setApproximationUpperBound(value)
    approximationUpperBound = property(__get_approximationUpperBound, __set_approximationUpperBound)

    def __get_approximationError(self):
        # optional Attribute
        try: # @return: double
            return self.obj.approximationError()
        except Core.ValueException:
            return None
    def __set_approximationError(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_approximationError() != value:
            self._needsUpdate = True
        self.obj.setApproximationError(value)
    approximationError = property(__get_approximationError, __set_approximationError)

    def __get_numberOfCoefficients(self):
        # optional Attribute
        try: # @return: int
            return self.obj.numberOfCoefficients()
        except Core.ValueException:
            return None
    def __set_numberOfCoefficients(self, arg):
        if self.__get_numberOfCoefficients() != arg:
            self._needsUpdate = True
        self.obj.setNumberOfCoefficients(arg)
    numberOfCoefficients = property(__get_numberOfCoefficients, __set_numberOfCoefficients)

    def __get_coefficients(self):
        # optional Attribute
        try: # @return: RealArray
            return RealArray2str(self.obj.coefficients().content())
        except Core.ValueException:
            return None
    def __set_coefficients(self, arg):
        try: value = str2RealArray(arg)
        except: value = None
        ret = self.__get_coefficients()
        if not str2RealArray(ret) == value:
            self._needsUpdate = True
        self.obj.setCoefficients(value)
    coefficients = property(__get_coefficients, __set_coefficients)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_responsefap(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_gainFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gainFrequency()
        except Core.ValueException:
            return None
    def __set_gainFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gainFrequency() != value:
            self._needsUpdate = True
        self.obj.setGainFrequency(value)
    gainFrequency = property(__get_gainFrequency, __set_gainFrequency)

    def __get_numberOfTuples(self):
        # optional Attribute
        try: # @return: int
            return self.obj.numberOfTuples()
        except Core.ValueException:
            return None
    def __set_numberOfTuples(self, arg):
        if self.__get_numberOfTuples() != arg:
            self._needsUpdate = True
        self.obj.setNumberOfTuples(arg)
    numberOfTuples = property(__get_numberOfTuples, __set_numberOfTuples)

    def __get_tuples(self):
        # optional Attribute
        try: # @return: RealArray
            return RealArray2str(self.obj.tuples().content())
        except Core.ValueException:
            return None
    def __set_tuples(self, arg):
        try: value = str2RealArray(arg)
        except: value = None
        ret = self.__get_tuples()
        if not str2RealArray(ret) == value:
            self._needsUpdate = True
        self.obj.setTuples(value)
    tuples = property(__get_tuples, __set_tuples)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_dataloggercalibration(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_serialNumber(self):
        try: # @return: const std::string&
            return self.obj.serialNumber()
        except Core.ValueException:
            return None
    def __set_serialNumber(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_serialNumber() != value:
            self._needsUpdate = True
        self.obj.setSerialNumber(value)
    serialNumber = property(__get_serialNumber, __set_serialNumber)

    def __get_channel(self):
        try: # @return: int
            return self.obj.channel()
        except Core.ValueException:
            return None
    def __set_channel(self, arg):
        if self.__get_channel() != arg:
            self._needsUpdate = True
        self.obj.setChannel(arg)
    channel = property(__get_channel, __set_channel)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_gainFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gainFrequency()
        except Core.ValueException:
            return None
    def __set_gainFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gainFrequency() != value:
            self._needsUpdate = True
        self.obj.setGainFrequency(value)
    gainFrequency = property(__get_gainFrequency, __set_gainFrequency)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_decimation(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_sampleRateNumerator(self):
        try: # @return: int
            return self.obj.sampleRateNumerator()
        except Core.ValueException:
            return None
    def __set_sampleRateNumerator(self, arg):
        if self.__get_sampleRateNumerator() != arg:
            self._needsUpdate = True
        self.obj.setSampleRateNumerator(arg)
    sampleRateNumerator = property(__get_sampleRateNumerator, __set_sampleRateNumerator)

    def __get_sampleRateDenominator(self):
        try: # @return: int
            return self.obj.sampleRateDenominator()
        except Core.ValueException:
            return None
    def __set_sampleRateDenominator(self, arg):
        if self.__get_sampleRateDenominator() != arg:
            self._needsUpdate = True
        self.obj.setSampleRateDenominator(arg)
    sampleRateDenominator = property(__get_sampleRateDenominator, __set_sampleRateDenominator)

    def __get_analogueFilterChain(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.analogueFilterChain()
            return B.content()
        except Core.ValueException:
            return None
    def __set_analogueFilterChain(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_analogueFilterChain() != value:
            self._needsUpdate = True
        self.obj.setAnalogueFilterChain(blob)
    analogueFilterChain = property(__get_analogueFilterChain, __set_analogueFilterChain)

    def __get_digitalFilterChain(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.digitalFilterChain()
            return B.content()
        except Core.ValueException:
            return None
    def __set_digitalFilterChain(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_digitalFilterChain() != value:
            self._needsUpdate = True
        self.obj.setDigitalFilterChain(blob)
    digitalFilterChain = property(__get_digitalFilterChain, __set_digitalFilterChain)


# package: Inventory
class base_datalogger(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_digitizerModel(self):
        try: # @return: const std::string&
            return self.obj.digitizerModel()
        except Core.ValueException:
            return None
    def __set_digitizerModel(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_digitizerModel() != value:
            self._needsUpdate = True
        self.obj.setDigitizerModel(value)
    digitizerModel = property(__get_digitizerModel, __set_digitizerModel)

    def __get_digitizerManufacturer(self):
        try: # @return: const std::string&
            return self.obj.digitizerManufacturer()
        except Core.ValueException:
            return None
    def __set_digitizerManufacturer(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_digitizerManufacturer() != value:
            self._needsUpdate = True
        self.obj.setDigitizerManufacturer(value)
    digitizerManufacturer = property(__get_digitizerManufacturer, __set_digitizerManufacturer)

    def __get_recorderModel(self):
        try: # @return: const std::string&
            return self.obj.recorderModel()
        except Core.ValueException:
            return None
    def __set_recorderModel(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_recorderModel() != value:
            self._needsUpdate = True
        self.obj.setRecorderModel(value)
    recorderModel = property(__get_recorderModel, __set_recorderModel)

    def __get_recorderManufacturer(self):
        try: # @return: const std::string&
            return self.obj.recorderManufacturer()
        except Core.ValueException:
            return None
    def __set_recorderManufacturer(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_recorderManufacturer() != value:
            self._needsUpdate = True
        self.obj.setRecorderManufacturer(value)
    recorderManufacturer = property(__get_recorderManufacturer, __set_recorderManufacturer)

    def __get_clockModel(self):
        try: # @return: const std::string&
            return self.obj.clockModel()
        except Core.ValueException:
            return None
    def __set_clockModel(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_clockModel() != value:
            self._needsUpdate = True
        self.obj.setClockModel(value)
    clockModel = property(__get_clockModel, __set_clockModel)

    def __get_clockManufacturer(self):
        try: # @return: const std::string&
            return self.obj.clockManufacturer()
        except Core.ValueException:
            return None
    def __set_clockManufacturer(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_clockManufacturer() != value:
            self._needsUpdate = True
        self.obj.setClockManufacturer(value)
    clockManufacturer = property(__get_clockManufacturer, __set_clockManufacturer)

    def __get_clockType(self):
        try: # @return: const std::string&
            return self.obj.clockType()
        except Core.ValueException:
            return None
    def __set_clockType(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_clockType() != value:
            self._needsUpdate = True
        self.obj.setClockType(value)
    clockType = property(__get_clockType, __set_clockType)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_maxClockDrift(self):
        # optional Attribute
        try: # @return: double
            return self.obj.maxClockDrift()
        except Core.ValueException:
            return None
    def __set_maxClockDrift(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_maxClockDrift() != value:
            self._needsUpdate = True
        self.obj.setMaxClockDrift(value)
    maxClockDrift = property(__get_maxClockDrift, __set_maxClockDrift)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


    def _new_dataloggercalibration(self, **args):
        try: obj = DataModel.DataloggerCalibration()
        except KeyError: pass
        try: obj.setSerialNumber(args["serialNumber"])
        except KeyError: pass
        try: obj.setChannel(args["channel"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setGainFrequency(args["gainFrequency"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Datalogger: error adding DataloggerCalibration"
        return obj
    def __get_dataloggercalibration(self):
        list = []
        if dbQuery is None:
            if (self.obj.dataloggerCalibrationCount()):
                for i in xrange(self.obj.dataloggerCalibrationCount()):
                    obj = self.obj.dataloggerCalibration(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_dataloggercalibration(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.dataloggerCalibrationCount()
            while objects_left > 0:
                try:
                    obj = self.obj.dataloggerCalibration(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_dataloggercalibration(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_dataloggercalibration(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_dataloggercalibration(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _dataloggerCalibration = property(__get_dataloggercalibration)

    def _new_decimation(self, **args):
        try: obj = DataModel.Decimation()
        except KeyError: pass
        try: obj.setSampleRateNumerator(args["sampleRateNumerator"])
        except KeyError: pass
        try: obj.setSampleRateDenominator(args["sampleRateDenominator"])
        except KeyError: pass
        try: obj.setAnalogueFilterChain(args["analogueFilterChain"])
        except KeyError: pass
        try: obj.setDigitalFilterChain(args["digitalFilterChain"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Datalogger: error adding Decimation"
        return obj
    def __get_decimation(self):
        list = []
        if dbQuery is None:
            if (self.obj.decimationCount()):
                for i in xrange(self.obj.decimationCount()):
                    obj = self.obj.decimation(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_decimation(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.decimationCount()
            while objects_left > 0:
                try:
                    obj = self.obj.decimation(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_decimation(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_decimation(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_decimation(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _decimation = property(__get_decimation)


# package: Inventory
class base_responsefir(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_name(self):
        try: # @return: const std::string&
            return self.obj.name()
        except Core.ValueException:
            return None
    def __set_name(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_name() != value:
            self._needsUpdate = True
        self.obj.setName(value)
    name = property(__get_name, __set_name)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_decimationFactor(self):
        # optional Attribute
        try: # @return: int
            return self.obj.decimationFactor()
        except Core.ValueException:
            return None
    def __set_decimationFactor(self, arg):
        if self.__get_decimationFactor() != arg:
            self._needsUpdate = True
        self.obj.setDecimationFactor(arg)
    decimationFactor = property(__get_decimationFactor, __set_decimationFactor)

    def __get_delay(self):
        # optional Attribute
        try: # @return: double
            return self.obj.delay()
        except Core.ValueException:
            return None
    def __set_delay(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_delay() != value:
            self._needsUpdate = True
        self.obj.setDelay(value)
    delay = property(__get_delay, __set_delay)

    def __get_correction(self):
        # optional Attribute
        try: # @return: double
            return self.obj.correction()
        except Core.ValueException:
            return None
    def __set_correction(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_correction() != value:
            self._needsUpdate = True
        self.obj.setCorrection(value)
    correction = property(__get_correction, __set_correction)

    def __get_numberOfCoefficients(self):
        # optional Attribute
        try: # @return: int
            return self.obj.numberOfCoefficients()
        except Core.ValueException:
            return None
    def __set_numberOfCoefficients(self, arg):
        if self.__get_numberOfCoefficients() != arg:
            self._needsUpdate = True
        self.obj.setNumberOfCoefficients(arg)
    numberOfCoefficients = property(__get_numberOfCoefficients, __set_numberOfCoefficients)

    def __get_symmetry(self):
        try: # @return: const std::string&
            return self.obj.symmetry()
        except Core.ValueException:
            return None
    def __set_symmetry(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_symmetry() != value:
            self._needsUpdate = True
        self.obj.setSymmetry(value)
    symmetry = property(__get_symmetry, __set_symmetry)

    def __get_coefficients(self):
        # optional Attribute
        try: # @return: RealArray
            return RealArray2str(self.obj.coefficients().content())
        except Core.ValueException:
            return None
    def __set_coefficients(self, arg):
        try: value = str2RealArray(arg)
        except: value = None
        ret = self.__get_coefficients()
        if not str2RealArray(ret) == value:
            self._needsUpdate = True
        self.obj.setCoefficients(value)
    coefficients = property(__get_coefficients, __set_coefficients)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


# package: Inventory
class base_auxstream(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_code(self):
        try: # @return: const std::string&
            return self.obj.code()
        except Core.ValueException:
            return None
    def __set_code(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_code() != value:
            self._needsUpdate = True
        self.obj.setCode(value)
    code = property(__get_code, __set_code)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_device(self):
        try: # @return: const std::string&
            return self.obj.device()
        except Core.ValueException:
            return None
    def __set_device(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_device() != value:
            self._needsUpdate = True
        self.obj.setDevice(value)
    device = property(__get_device, __set_device)

    def __get_deviceSerialNumber(self):
        try: # @return: const std::string&
            return self.obj.deviceSerialNumber()
        except Core.ValueException:
            return None
    def __set_deviceSerialNumber(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_deviceSerialNumber() != value:
            self._needsUpdate = True
        self.obj.setDeviceSerialNumber(value)
    deviceSerialNumber = property(__get_deviceSerialNumber, __set_deviceSerialNumber)

    def __get_source(self):
        try: # @return: const std::string&
            return self.obj.source()
        except Core.ValueException:
            return None
    def __set_source(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_source() != value:
            self._needsUpdate = True
        self.obj.setSource(value)
    source = property(__get_source, __set_source)

    def __get_format(self):
        try: # @return: const std::string&
            return self.obj.format()
        except Core.ValueException:
            return None
    def __set_format(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_format() != value:
            self._needsUpdate = True
        self.obj.setFormat(value)
    format = property(__get_format, __set_format)

    def __get_flags(self):
        try: # @return: const std::string&
            return self.obj.flags()
        except Core.ValueException:
            return None
    def __set_flags(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_flags() != value:
            self._needsUpdate = True
        self.obj.setFlags(value)
    flags = property(__get_flags, __set_flags)

    def __get_restricted(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.restricted()
        except Core.ValueException:
            return None
    def __set_restricted(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_restricted() != value:
            self._needsUpdate = True
        self.obj.setRestricted(value)
    restricted = property(__get_restricted, __set_restricted)


# package: Inventory
class base_stream(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_code(self):
        try: # @return: const std::string&
            return self.obj.code()
        except Core.ValueException:
            return None
    def __set_code(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_code() != value:
            self._needsUpdate = True
        self.obj.setCode(value)
    code = property(__get_code, __set_code)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_datalogger(self):
        try: # @return: const std::string&
            return self.obj.datalogger()
        except Core.ValueException:
            return None
    def __set_datalogger(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_datalogger() != value:
            self._needsUpdate = True
        self.obj.setDatalogger(value)
    datalogger = property(__get_datalogger, __set_datalogger)

    def __get_dataloggerSerialNumber(self):
        try: # @return: const std::string&
            return self.obj.dataloggerSerialNumber()
        except Core.ValueException:
            return None
    def __set_dataloggerSerialNumber(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_dataloggerSerialNumber() != value:
            self._needsUpdate = True
        self.obj.setDataloggerSerialNumber(value)
    dataloggerSerialNumber = property(__get_dataloggerSerialNumber, __set_dataloggerSerialNumber)

    def __get_dataloggerChannel(self):
        # optional Attribute
        try: # @return: int
            return self.obj.dataloggerChannel()
        except Core.ValueException:
            return None
    def __set_dataloggerChannel(self, arg):
        if self.__get_dataloggerChannel() != arg:
            self._needsUpdate = True
        self.obj.setDataloggerChannel(arg)
    dataloggerChannel = property(__get_dataloggerChannel, __set_dataloggerChannel)

    def __get_sensor(self):
        try: # @return: const std::string&
            return self.obj.sensor()
        except Core.ValueException:
            return None
    def __set_sensor(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_sensor() != value:
            self._needsUpdate = True
        self.obj.setSensor(value)
    sensor = property(__get_sensor, __set_sensor)

    def __get_sensorSerialNumber(self):
        try: # @return: const std::string&
            return self.obj.sensorSerialNumber()
        except Core.ValueException:
            return None
    def __set_sensorSerialNumber(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_sensorSerialNumber() != value:
            self._needsUpdate = True
        self.obj.setSensorSerialNumber(value)
    sensorSerialNumber = property(__get_sensorSerialNumber, __set_sensorSerialNumber)

    def __get_sensorChannel(self):
        # optional Attribute
        try: # @return: int
            return self.obj.sensorChannel()
        except Core.ValueException:
            return None
    def __set_sensorChannel(self, arg):
        if self.__get_sensorChannel() != arg:
            self._needsUpdate = True
        self.obj.setSensorChannel(arg)
    sensorChannel = property(__get_sensorChannel, __set_sensorChannel)

    def __get_clockSerialNumber(self):
        try: # @return: const std::string&
            return self.obj.clockSerialNumber()
        except Core.ValueException:
            return None
    def __set_clockSerialNumber(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_clockSerialNumber() != value:
            self._needsUpdate = True
        self.obj.setClockSerialNumber(value)
    clockSerialNumber = property(__get_clockSerialNumber, __set_clockSerialNumber)

    def __get_sampleRateNumerator(self):
        # optional Attribute
        try: # @return: int
            return self.obj.sampleRateNumerator()
        except Core.ValueException:
            return None
    def __set_sampleRateNumerator(self, arg):
        if self.__get_sampleRateNumerator() != arg:
            self._needsUpdate = True
        self.obj.setSampleRateNumerator(arg)
    sampleRateNumerator = property(__get_sampleRateNumerator, __set_sampleRateNumerator)

    def __get_sampleRateDenominator(self):
        # optional Attribute
        try: # @return: int
            return self.obj.sampleRateDenominator()
        except Core.ValueException:
            return None
    def __set_sampleRateDenominator(self, arg):
        if self.__get_sampleRateDenominator() != arg:
            self._needsUpdate = True
        self.obj.setSampleRateDenominator(arg)
    sampleRateDenominator = property(__get_sampleRateDenominator, __set_sampleRateDenominator)

    def __get_depth(self):
        # optional Attribute
        try: # @return: double
            return self.obj.depth()
        except Core.ValueException:
            return None
    def __set_depth(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_depth() != value:
            self._needsUpdate = True
        self.obj.setDepth(value)
    depth = property(__get_depth, __set_depth)

    def __get_azimuth(self):
        # optional Attribute
        try: # @return: double
            return self.obj.azimuth()
        except Core.ValueException:
            return None
    def __set_azimuth(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_azimuth() != value:
            self._needsUpdate = True
        self.obj.setAzimuth(value)
    azimuth = property(__get_azimuth, __set_azimuth)

    def __get_dip(self):
        # optional Attribute
        try: # @return: double
            return self.obj.dip()
        except Core.ValueException:
            return None
    def __set_dip(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_dip() != value:
            self._needsUpdate = True
        self.obj.setDip(value)
    dip = property(__get_dip, __set_dip)

    def __get_gain(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gain()
        except Core.ValueException:
            return None
    def __set_gain(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gain() != value:
            self._needsUpdate = True
        self.obj.setGain(value)
    gain = property(__get_gain, __set_gain)

    def __get_gainFrequency(self):
        # optional Attribute
        try: # @return: double
            return self.obj.gainFrequency()
        except Core.ValueException:
            return None
    def __set_gainFrequency(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_gainFrequency() != value:
            self._needsUpdate = True
        self.obj.setGainFrequency(value)
    gainFrequency = property(__get_gainFrequency, __set_gainFrequency)

    def __get_gainUnit(self):
        try: # @return: const std::string&
            return self.obj.gainUnit()
        except Core.ValueException:
            return None
    def __set_gainUnit(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_gainUnit() != value:
            self._needsUpdate = True
        self.obj.setGainUnit(value)
    gainUnit = property(__get_gainUnit, __set_gainUnit)

    def __get_format(self):
        try: # @return: const std::string&
            return self.obj.format()
        except Core.ValueException:
            return None
    def __set_format(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_format() != value:
            self._needsUpdate = True
        self.obj.setFormat(value)
    format = property(__get_format, __set_format)

    def __get_flags(self):
        try: # @return: const std::string&
            return self.obj.flags()
        except Core.ValueException:
            return None
    def __set_flags(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_flags() != value:
            self._needsUpdate = True
        self.obj.setFlags(value)
    flags = property(__get_flags, __set_flags)

    def __get_restricted(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.restricted()
        except Core.ValueException:
            return None
    def __set_restricted(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_restricted() != value:
            self._needsUpdate = True
        self.obj.setRestricted(value)
    restricted = property(__get_restricted, __set_restricted)

    def __get_shared(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.shared()
        except Core.ValueException:
            return None
    def __set_shared(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_shared() != value:
            self._needsUpdate = True
        self.obj.setShared(value)
    shared = property(__get_shared, __set_shared)


# package: Inventory
class base_sensorlocation(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_code(self):
        try: # @return: const std::string&
            return self.obj.code()
        except Core.ValueException:
            return None
    def __set_code(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_code() != value:
            self._needsUpdate = True
        self.obj.setCode(value)
    code = property(__get_code, __set_code)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_latitude(self):
        # optional Attribute
        try: # @return: double
            return self.obj.latitude()
        except Core.ValueException:
            return None
    def __set_latitude(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_latitude() != value:
            self._needsUpdate = True
        self.obj.setLatitude(value)
    latitude = property(__get_latitude, __set_latitude)

    def __get_longitude(self):
        # optional Attribute
        try: # @return: double
            return self.obj.longitude()
        except Core.ValueException:
            return None
    def __set_longitude(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_longitude() != value:
            self._needsUpdate = True
        self.obj.setLongitude(value)
    longitude = property(__get_longitude, __set_longitude)

    def __get_elevation(self):
        # optional Attribute
        try: # @return: double
            return self.obj.elevation()
        except Core.ValueException:
            return None
    def __set_elevation(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_elevation() != value:
            self._needsUpdate = True
        self.obj.setElevation(value)
    elevation = property(__get_elevation, __set_elevation)


    def _new_auxstream(self, **args):
        try: obj = DataModel.AuxStream()
        except KeyError: pass
        try: obj.setCode(args["code"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setDevice(args["device"])
        except KeyError: pass
        try: obj.setDeviceSerialNumber(args["deviceSerialNumber"])
        except KeyError: pass
        try: obj.setSource(args["source"])
        except KeyError: pass
        try: obj.setFormat(args["format"])
        except KeyError: pass
        try: obj.setFlags(args["flags"])
        except KeyError: pass
        try: obj.setRestricted(args["restricted"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.SensorLocation: error adding AuxStream"
        return obj
    def __get_auxstream(self):
        list = []
        if dbQuery is None:
            if (self.obj.auxStreamCount()):
                for i in xrange(self.obj.auxStreamCount()):
                    obj = self.obj.auxStream(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_auxstream(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.auxStreamCount()
            while objects_left > 0:
                try:
                    obj = self.obj.auxStream(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_auxstream(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_auxstream(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_auxstream(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _auxStream = property(__get_auxstream)

    def _new_stream(self, **args):
        try: obj = DataModel.Stream()
        except KeyError: pass
        try: obj.setCode(args["code"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setDatalogger(args["datalogger"])
        except KeyError: pass
        try: obj.setDataloggerSerialNumber(args["dataloggerSerialNumber"])
        except KeyError: pass
        try: obj.setDataloggerChannel(args["dataloggerChannel"])
        except KeyError: pass
        try: obj.setSensor(args["sensor"])
        except KeyError: pass
        try: obj.setSensorSerialNumber(args["sensorSerialNumber"])
        except KeyError: pass
        try: obj.setSensorChannel(args["sensorChannel"])
        except KeyError: pass
        try: obj.setClockSerialNumber(args["clockSerialNumber"])
        except KeyError: pass
        try: obj.setSampleRateNumerator(args["sampleRateNumerator"])
        except KeyError: pass
        try: obj.setSampleRateDenominator(args["sampleRateDenominator"])
        except KeyError: pass
        try: obj.setDepth(args["depth"])
        except KeyError: pass
        try: obj.setAzimuth(args["azimuth"])
        except KeyError: pass
        try: obj.setDip(args["dip"])
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setGainFrequency(args["gainFrequency"])
        except KeyError: pass
        try: obj.setGainUnit(args["gainUnit"])
        except KeyError: pass
        try: obj.setFormat(args["format"])
        except KeyError: pass
        try: obj.setFlags(args["flags"])
        except KeyError: pass
        try: obj.setRestricted(args["restricted"])
        except KeyError: pass
        try: obj.setShared(args["shared"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.SensorLocation: error adding Stream"
        return obj
    def __get_stream(self):
        list = []
        if dbQuery is None:
            if (self.obj.streamCount()):
                for i in xrange(self.obj.streamCount()):
                    obj = self.obj.stream(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_stream(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.streamCount()
            while objects_left > 0:
                try:
                    obj = self.obj.stream(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_stream(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_stream(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_stream(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _stream = property(__get_stream)


# package: Inventory
class base_station(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_code(self):
        try: # @return: const std::string&
            return self.obj.code()
        except Core.ValueException:
            return None
    def __set_code(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_code() != value:
            self._needsUpdate = True
        self.obj.setCode(value)
    code = property(__get_code, __set_code)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_latitude(self):
        # optional Attribute
        try: # @return: double
            return self.obj.latitude()
        except Core.ValueException:
            return None
    def __set_latitude(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_latitude() != value:
            self._needsUpdate = True
        self.obj.setLatitude(value)
    latitude = property(__get_latitude, __set_latitude)

    def __get_longitude(self):
        # optional Attribute
        try: # @return: double
            return self.obj.longitude()
        except Core.ValueException:
            return None
    def __set_longitude(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_longitude() != value:
            self._needsUpdate = True
        self.obj.setLongitude(value)
    longitude = property(__get_longitude, __set_longitude)

    def __get_elevation(self):
        # optional Attribute
        try: # @return: double
            return self.obj.elevation()
        except Core.ValueException:
            return None
    def __set_elevation(self, arg):
        try: value = float(arg)
        except: value = None
        if self.__get_elevation() != value:
            self._needsUpdate = True
        self.obj.setElevation(value)
    elevation = property(__get_elevation, __set_elevation)

    def __get_place(self):
        try: # @return: const std::string&
            return self.obj.place()
        except Core.ValueException:
            return None
    def __set_place(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_place() != value:
            self._needsUpdate = True
        self.obj.setPlace(value)
    place = property(__get_place, __set_place)

    def __get_country(self):
        try: # @return: const std::string&
            return self.obj.country()
        except Core.ValueException:
            return None
    def __set_country(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_country() != value:
            self._needsUpdate = True
        self.obj.setCountry(value)
    country = property(__get_country, __set_country)

    def __get_affiliation(self):
        try: # @return: const std::string&
            return self.obj.affiliation()
        except Core.ValueException:
            return None
    def __set_affiliation(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_affiliation() != value:
            self._needsUpdate = True
        self.obj.setAffiliation(value)
    affiliation = property(__get_affiliation, __set_affiliation)

    def __get_type(self):
        try: # @return: const std::string&
            return self.obj.type()
        except Core.ValueException:
            return None
    def __set_type(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_type() != value:
            self._needsUpdate = True
        self.obj.setType(value)
    type = property(__get_type, __set_type)

    def __get_archive(self):
        try: # @return: const std::string&
            return self.obj.archive()
        except Core.ValueException:
            return None
    def __set_archive(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_archive() != value:
            self._needsUpdate = True
        self.obj.setArchive(value)
    archive = property(__get_archive, __set_archive)

    def __get_archiveNetworkCode(self):
        try: # @return: const std::string&
            return self.obj.archiveNetworkCode()
        except Core.ValueException:
            return None
    def __set_archiveNetworkCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_archiveNetworkCode() != value:
            self._needsUpdate = True
        self.obj.setArchiveNetworkCode(value)
    archiveNetworkCode = property(__get_archiveNetworkCode, __set_archiveNetworkCode)

    def __get_restricted(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.restricted()
        except Core.ValueException:
            return None
    def __set_restricted(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_restricted() != value:
            self._needsUpdate = True
        self.obj.setRestricted(value)
    restricted = property(__get_restricted, __set_restricted)

    def __get_shared(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.shared()
        except Core.ValueException:
            return None
    def __set_shared(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_shared() != value:
            self._needsUpdate = True
        self.obj.setShared(value)
    shared = property(__get_shared, __set_shared)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


    def _new_sensorlocation(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.SensorLocation.Find(publicID): publicID = None
        if publicID: obj = DataModel.SensorLocation.Create(publicID)
        else: obj = DataModel.SensorLocation.Create()
        try: obj.setCode(args["code"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setLatitude(args["latitude"])
        except KeyError: pass
        try: obj.setLongitude(args["longitude"])
        except KeyError: pass
        try: obj.setElevation(args["elevation"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Station: error adding SensorLocation"
        return obj
    def __get_sensorlocation(self):
        list = []
        if dbQuery is None:
            if (self.obj.sensorLocationCount()):
                for i in xrange(self.obj.sensorLocationCount()):
                    obj = self.obj.sensorLocation(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_sensorlocation(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.SensorLocation.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.SensorLocation.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_sensorlocation(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _sensorLocation = property(__get_sensorlocation)


# package: Inventory
class base_network(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_code(self):
        try: # @return: const std::string&
            return self.obj.code()
        except Core.ValueException:
            return None
    def __set_code(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_code() != value:
            self._needsUpdate = True
        self.obj.setCode(value)
    code = property(__get_code, __set_code)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_description(self):
        try: # @return: const std::string&
            return self.obj.description()
        except Core.ValueException:
            return None
    def __set_description(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_description() != value:
            self._needsUpdate = True
        self.obj.setDescription(value)
    description = property(__get_description, __set_description)

    def __get_institutions(self):
        try: # @return: const std::string&
            return self.obj.institutions()
        except Core.ValueException:
            return None
    def __set_institutions(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_institutions() != value:
            self._needsUpdate = True
        self.obj.setInstitutions(value)
    institutions = property(__get_institutions, __set_institutions)

    def __get_region(self):
        try: # @return: const std::string&
            return self.obj.region()
        except Core.ValueException:
            return None
    def __set_region(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_region() != value:
            self._needsUpdate = True
        self.obj.setRegion(value)
    region = property(__get_region, __set_region)

    def __get_type(self):
        try: # @return: const std::string&
            return self.obj.type()
        except Core.ValueException:
            return None
    def __set_type(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_type() != value:
            self._needsUpdate = True
        self.obj.setType(value)
    type = property(__get_type, __set_type)

    def __get_netClass(self):
        try: # @return: const std::string&
            return self.obj.netClass()
        except Core.ValueException:
            return None
    def __set_netClass(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_netClass() != value:
            self._needsUpdate = True
        self.obj.setNetClass(value)
    netClass = property(__get_netClass, __set_netClass)

    def __get_archive(self):
        try: # @return: const std::string&
            return self.obj.archive()
        except Core.ValueException:
            return None
    def __set_archive(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_archive() != value:
            self._needsUpdate = True
        self.obj.setArchive(value)
    archive = property(__get_archive, __set_archive)

    def __get_restricted(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.restricted()
        except Core.ValueException:
            return None
    def __set_restricted(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_restricted() != value:
            self._needsUpdate = True
        self.obj.setRestricted(value)
    restricted = property(__get_restricted, __set_restricted)

    def __get_shared(self):
        # optional Attribute
        try: # @return: bool
            return self.obj.shared()
        except Core.ValueException:
            return None
    def __set_shared(self, arg):
        try: value = bool(arg)
        except: value = None
        if self.__get_shared() != value:
            self._needsUpdate = True
        self.obj.setShared(value)
    shared = property(__get_shared, __set_shared)

    def __get_remark(self):
        # optional Attribute
        try: # @return: Blob
            B = self.obj.remark()
            return B.content()
        except Core.ValueException:
            return None
    def __set_remark(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
            blob = DataModel.Blob()
            if value:
                blob.setContent(value)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_remark() != value:
            self._needsUpdate = True
        self.obj.setRemark(blob)
    remark = property(__get_remark, __set_remark)


    def _new_station(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.Station.Find(publicID): publicID = None
        if publicID: obj = DataModel.Station.Create(publicID)
        else: obj = DataModel.Station.Create()
        try: obj.setCode(args["code"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setLatitude(args["latitude"])
        except KeyError: pass
        try: obj.setLongitude(args["longitude"])
        except KeyError: pass
        try: obj.setElevation(args["elevation"])
        except KeyError: pass
        try: obj.setPlace(args["place"])
        except KeyError: pass
        try: obj.setCountry(args["country"])
        except KeyError: pass
        try: obj.setAffiliation(args["affiliation"])
        except KeyError: pass
        try: obj.setType(args["type"])
        except KeyError: pass
        try: obj.setArchive(args["archive"])
        except KeyError: pass
        try: obj.setArchiveNetworkCode(args["archiveNetworkCode"])
        except KeyError: pass
        try: obj.setRestricted(args["restricted"])
        except KeyError: pass
        try: obj.setShared(args["shared"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Network: error adding Station"
        return obj
    def __get_station(self):
        list = []
        if dbQuery is None:
            if (self.obj.stationCount()):
                for i in xrange(self.obj.stationCount()):
                    obj = self.obj.station(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_station(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.Station.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.Station.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_station(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _station = property(__get_station)


# package: Inventory
class base_inventory(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def _new_stationgroup(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.StationGroup.Find(publicID): publicID = None
        if publicID: obj = DataModel.StationGroup.Create(publicID)
        else: obj = DataModel.StationGroup.Create()
        try: obj.setType(args["type"])
        except KeyError: pass
        try: obj.setCode(args["code"])
        except KeyError: pass
        try:
            if args["start"] is None:
                obj.setStart(None)
            else:
                obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setLatitude(args["latitude"])
        except KeyError: pass
        try: obj.setLongitude(args["longitude"])
        except KeyError: pass
        try: obj.setElevation(args["elevation"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding StationGroup"
        return obj
    def __get_stationgroup(self):
        list = []
        if dbQuery is None:
            if (self.obj.stationGroupCount()):
                for i in xrange(self.obj.stationGroupCount()):
                    obj = self.obj.stationGroup(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_stationgroup(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.StationGroup.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.StationGroup.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_stationgroup(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _stationGroup = property(__get_stationgroup)

    def _new_auxdevice(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.AuxDevice.Find(publicID): publicID = None
        if publicID: obj = DataModel.AuxDevice.Create(publicID)
        else: obj = DataModel.AuxDevice.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setModel(args["model"])
        except KeyError: pass
        try: obj.setManufacturer(args["manufacturer"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding AuxDevice"
        return obj
    def __get_auxdevice(self):
        list = []
        if dbQuery is None:
            if (self.obj.auxDeviceCount()):
                for i in xrange(self.obj.auxDeviceCount()):
                    obj = self.obj.auxDevice(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_auxdevice(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.AuxDevice.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.AuxDevice.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_auxdevice(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _auxDevice = property(__get_auxdevice)

    def _new_sensor(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.Sensor.Find(publicID): publicID = None
        if publicID: obj = DataModel.Sensor.Create(publicID)
        else: obj = DataModel.Sensor.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setModel(args["model"])
        except KeyError: pass
        try: obj.setManufacturer(args["manufacturer"])
        except KeyError: pass
        try: obj.setType(args["type"])
        except KeyError: pass
        try: obj.setUnit(args["unit"])
        except KeyError: pass
        try: obj.setLowFrequency(args["lowFrequency"])
        except KeyError: pass
        try: obj.setHighFrequency(args["highFrequency"])
        except KeyError: pass
        try: obj.setResponse(args["response"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding Sensor"
        return obj
    def __get_sensor(self):
        list = []
        if dbQuery is None:
            if (self.obj.sensorCount()):
                for i in xrange(self.obj.sensorCount()):
                    obj = self.obj.sensor(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_sensor(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.Sensor.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.Sensor.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_sensor(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _sensor = property(__get_sensor)

    def _new_datalogger(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.Datalogger.Find(publicID): publicID = None
        if publicID: obj = DataModel.Datalogger.Create(publicID)
        else: obj = DataModel.Datalogger.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setDigitizerModel(args["digitizerModel"])
        except KeyError: pass
        try: obj.setDigitizerManufacturer(args["digitizerManufacturer"])
        except KeyError: pass
        try: obj.setRecorderModel(args["recorderModel"])
        except KeyError: pass
        try: obj.setRecorderManufacturer(args["recorderManufacturer"])
        except KeyError: pass
        try: obj.setClockModel(args["clockModel"])
        except KeyError: pass
        try: obj.setClockManufacturer(args["clockManufacturer"])
        except KeyError: pass
        try: obj.setClockType(args["clockType"])
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setMaxClockDrift(args["maxClockDrift"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding Datalogger"
        return obj
    def __get_datalogger(self):
        list = []
        if dbQuery is None:
            if (self.obj.dataloggerCount()):
                for i in xrange(self.obj.dataloggerCount()):
                    obj = self.obj.datalogger(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_datalogger(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.Datalogger.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.Datalogger.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_datalogger(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _datalogger = property(__get_datalogger)

    def _new_responsepaz(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.ResponsePAZ.Find(publicID): publicID = None
        if publicID: obj = DataModel.ResponsePAZ.Create(publicID)
        else: obj = DataModel.ResponsePAZ.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setType(args["type"])
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setGainFrequency(args["gainFrequency"])
        except KeyError: pass
        try: obj.setNormalizationFactor(args["normalizationFactor"])
        except KeyError: pass
        try: obj.setNormalizationFrequency(args["normalizationFrequency"])
        except KeyError: pass
        try: obj.setNumberOfZeros(args["numberOfZeros"])
        except KeyError: pass
        try: obj.setNumberOfPoles(args["numberOfPoles"])
        except KeyError: pass
        try: obj.setZeros(args["zeros"])
        except KeyError: pass
        try: obj.setPoles(args["poles"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding ResponsePAZ"
        return obj
    def __get_responsepaz(self):
        list = []
        if dbQuery is None:
            if (self.obj.responsePAZCount()):
                for i in xrange(self.obj.responsePAZCount()):
                    obj = self.obj.responsePAZ(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_responsepaz(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.ResponsePAZ.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.ResponsePAZ.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_responsepaz(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _responsePAZ = property(__get_responsepaz)

    def _new_responsefir(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.ResponseFIR.Find(publicID): publicID = None
        if publicID: obj = DataModel.ResponseFIR.Create(publicID)
        else: obj = DataModel.ResponseFIR.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setDecimationFactor(args["decimationFactor"])
        except KeyError: pass
        try: obj.setDelay(args["delay"])
        except KeyError: pass
        try: obj.setCorrection(args["correction"])
        except KeyError: pass
        try: obj.setNumberOfCoefficients(args["numberOfCoefficients"])
        except KeyError: pass
        try: obj.setSymmetry(args["symmetry"])
        except KeyError: pass
        try: obj.setCoefficients(args["coefficients"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding ResponseFIR"
        return obj
    def __get_responsefir(self):
        list = []
        if dbQuery is None:
            if (self.obj.responseFIRCount()):
                for i in xrange(self.obj.responseFIRCount()):
                    obj = self.obj.responseFIR(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_responsefir(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.ResponseFIR.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.ResponseFIR.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_responsefir(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _responseFIR = property(__get_responsefir)

    def _new_responsepolynomial(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.ResponsePolynomial.Find(publicID): publicID = None
        if publicID: obj = DataModel.ResponsePolynomial.Create(publicID)
        else: obj = DataModel.ResponsePolynomial.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setGainFrequency(args["gainFrequency"])
        except KeyError: pass
        try: obj.setFrequencyUnit(args["frequencyUnit"])
        except KeyError: pass
        try: obj.setApproximationType(args["approximationType"])
        except KeyError: pass
        try: obj.setApproximationLowerBound(args["approximationLowerBound"])
        except KeyError: pass
        try: obj.setApproximationUpperBound(args["approximationUpperBound"])
        except KeyError: pass
        try: obj.setApproximationError(args["approximationError"])
        except KeyError: pass
        try: obj.setNumberOfCoefficients(args["numberOfCoefficients"])
        except KeyError: pass
        try: obj.setCoefficients(args["coefficients"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding ResponsePolynomial"
        return obj
    def __get_responsepolynomial(self):
        list = []
        if dbQuery is None:
            if (self.obj.responsePolynomialCount()):
                for i in xrange(self.obj.responsePolynomialCount()):
                    obj = self.obj.responsePolynomial(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_responsepolynomial(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.ResponsePolynomial.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.ResponsePolynomial.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_responsepolynomial(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _responsePolynomial = property(__get_responsepolynomial)

    def _new_responsefap(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.ResponseFAP.Find(publicID): publicID = None
        if publicID: obj = DataModel.ResponseFAP.Create(publicID)
        else: obj = DataModel.ResponseFAP.Create()
        try: obj.setName(args["name"])
        except KeyError: pass
        try: obj.setGain(args["gain"])
        except KeyError: pass
        try: obj.setGainFrequency(args["gainFrequency"])
        except KeyError: pass
        try: obj.setNumberOfTuples(args["numberOfTuples"])
        except KeyError: pass
        try: obj.setTuples(args["tuples"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding ResponseFAP"
        return obj
    def __get_responsefap(self):
        list = []
        if dbQuery is None:
            if (self.obj.responseFAPCount()):
                for i in xrange(self.obj.responseFAPCount()):
                    obj = self.obj.responseFAP(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_responsefap(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.ResponseFAP.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.ResponseFAP.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_responsefap(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _responseFAP = property(__get_responsefap)

    def _new_network(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.Network.Find(publicID): publicID = None
        if publicID: obj = DataModel.Network.Create(publicID)
        else: obj = DataModel.Network.Create()
        try: obj.setCode(args["code"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setDescription(args["description"])
        except KeyError: pass
        try: obj.setInstitutions(args["institutions"])
        except KeyError: pass
        try: obj.setRegion(args["region"])
        except KeyError: pass
        try: obj.setType(args["type"])
        except KeyError: pass
        try: obj.setNetClass(args["netClass"])
        except KeyError: pass
        try: obj.setArchive(args["archive"])
        except KeyError: pass
        try: obj.setRestricted(args["restricted"])
        except KeyError: pass
        try: obj.setShared(args["shared"])
        except KeyError: pass
        try: obj.setRemark(args["remark"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Inventory: error adding Network"
        return obj
    def __get_network(self):
        list = []
        if dbQuery is None:
            if (self.obj.networkCount()):
                for i in xrange(self.obj.networkCount()):
                    obj = self.obj.network(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_network(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.Network.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.Network.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_network(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _network = property(__get_network)


# package: Routing
class base_routearclink(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_address(self):
        try: # @return: const std::string&
            return self.obj.address()
        except Core.ValueException:
            return None
    def __set_address(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_address() != value:
            self._needsUpdate = True
        self.obj.setAddress(value)
    address = property(__get_address, __set_address)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)

    def __get_priority(self):
        # optional Attribute
        try: # @return: int
            return self.obj.priority()
        except Core.ValueException:
            return None
    def __set_priority(self, arg):
        if self.__get_priority() != arg:
            self._needsUpdate = True
        self.obj.setPriority(arg)
    priority = property(__get_priority, __set_priority)


# package: Routing
class base_routeseedlink(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_address(self):
        try: # @return: const std::string&
            return self.obj.address()
        except Core.ValueException:
            return None
    def __set_address(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_address() != value:
            self._needsUpdate = True
        self.obj.setAddress(value)
    address = property(__get_address, __set_address)

    def __get_priority(self):
        # optional Attribute
        try: # @return: int
            return self.obj.priority()
        except Core.ValueException:
            return None
    def __set_priority(self, arg):
        if self.__get_priority() != arg:
            self._needsUpdate = True
        self.obj.setPriority(arg)
    priority = property(__get_priority, __set_priority)


# package: Routing
class base_route(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def __get_networkCode(self):
        try: # @return: const std::string&
            return self.obj.networkCode()
        except Core.ValueException:
            return None
    def __set_networkCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_networkCode() != value:
            self._needsUpdate = True
        self.obj.setNetworkCode(value)
    networkCode = property(__get_networkCode, __set_networkCode)

    def __get_stationCode(self):
        try: # @return: const std::string&
            return self.obj.stationCode()
        except Core.ValueException:
            return None
    def __set_stationCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_stationCode() != value:
            self._needsUpdate = True
        self.obj.setStationCode(value)
    stationCode = property(__get_stationCode, __set_stationCode)

    def __get_locationCode(self):
        try: # @return: const std::string&
            return self.obj.locationCode()
        except Core.ValueException:
            return None
    def __set_locationCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_locationCode() != value:
            self._needsUpdate = True
        self.obj.setLocationCode(value)
    locationCode = property(__get_locationCode, __set_locationCode)

    def __get_streamCode(self):
        try: # @return: const std::string&
            return self.obj.streamCode()
        except Core.ValueException:
            return None
    def __set_streamCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_streamCode() != value:
            self._needsUpdate = True
        self.obj.setStreamCode(value)
    streamCode = property(__get_streamCode, __set_streamCode)


    def _new_routearclink(self, **args):
        try: obj = DataModel.RouteArclink()
        except KeyError: pass
        try: obj.setAddress(args["address"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try: obj.setPriority(args["priority"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Route: error adding RouteArclink"
        return obj
    def __get_routearclink(self):
        list = []
        if dbQuery is None:
            if (self.obj.routeArclinkCount()):
                for i in xrange(self.obj.routeArclinkCount()):
                    obj = self.obj.routeArclink(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_routearclink(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.routeArclinkCount()
            while objects_left > 0:
                try:
                    obj = self.obj.routeArclink(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_routearclink(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_routearclink(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_routearclink(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _routeArclink = property(__get_routearclink)

    def _new_routeseedlink(self, **args):
        try: obj = DataModel.RouteSeedlink()
        except KeyError: pass
        try: obj.setAddress(args["address"])
        except KeyError: pass
        try: obj.setPriority(args["priority"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Route: error adding RouteSeedlink"
        return obj
    def __get_routeseedlink(self):
        list = []
        if dbQuery is None:
            if (self.obj.routeSeedlinkCount()):
                for i in xrange(self.obj.routeSeedlinkCount()):
                    obj = self.obj.routeSeedlink(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_routeseedlink(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.routeSeedlinkCount()
            while objects_left > 0:
                try:
                    obj = self.obj.routeSeedlink(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_routeseedlink(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_routeseedlink(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_routeseedlink(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _routeSeedlink = property(__get_routeseedlink)


# package: Routing
class base_access(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_networkCode(self):
        try: # @return: const std::string&
            return self.obj.networkCode()
        except Core.ValueException:
            return None
    def __set_networkCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_networkCode() != value:
            self._needsUpdate = True
        self.obj.setNetworkCode(value)
    networkCode = property(__get_networkCode, __set_networkCode)

    def __get_stationCode(self):
        try: # @return: const std::string&
            return self.obj.stationCode()
        except Core.ValueException:
            return None
    def __set_stationCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_stationCode() != value:
            self._needsUpdate = True
        self.obj.setStationCode(value)
    stationCode = property(__get_stationCode, __set_stationCode)

    def __get_locationCode(self):
        try: # @return: const std::string&
            return self.obj.locationCode()
        except Core.ValueException:
            return None
    def __set_locationCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_locationCode() != value:
            self._needsUpdate = True
        self.obj.setLocationCode(value)
    locationCode = property(__get_locationCode, __set_locationCode)

    def __get_streamCode(self):
        try: # @return: const std::string&
            return self.obj.streamCode()
        except Core.ValueException:
            return None
    def __set_streamCode(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_streamCode() != value:
            self._needsUpdate = True
        self.obj.setStreamCode(value)
    streamCode = property(__get_streamCode, __set_streamCode)

    def __get_user(self):
        try: # @return: const std::string&
            return self.obj.user()
        except Core.ValueException:
            return None
    def __set_user(self, arg):
        try:
            if isinstance(arg, unicode):
                value = arg.encode("utf-8", "replace")
            else:
                value = str(arg)
        except Exception, e:
            logs.error(str(e))
            return
        if self.__get_user() != value:
            self._needsUpdate = True
        self.obj.setUser(value)
    user = property(__get_user, __set_user)

    def __get_start(self):
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.start().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_start(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_start()) != str(arg):
            self._needsUpdate = True
        self.obj.setStart(value)
    start = property(__get_start, __set_start)

    def __get_end(self):
        # optional Attribute
        try: # @return: Seiscomp::Core::Time
            return datetime.datetime(
                *(time.strptime(
                    self.obj.end().toString("%Y-%m-%dT%H:%M:%SZ"), 
                    "%Y-%m-%dT%H:%M:%SZ")[0:6]
                 )
            )
                
        except Core.ValueException:
            return None
    def __set_end(self, arg):
        value = None
        if arg is not None:
            try: value = Core.Time.FromString(str(arg), "%Y-%m-%d %H:%M:%S")
            except: pass
        if str(self.__get_end()) != str(arg):
            self._needsUpdate = True
        self.obj.setEnd(value)
    end = property(__get_end, __set_end)


# package: Routing
class base_routing(object):
    def __init__(self, obj):
        self.obj = obj
        self._needsUpdate = False

    def _sync_update(self):
        if self._needsUpdate:
            self.obj.lastModified = Core.Time.GMT()
            self.obj.update()
            self._needsUpdate = False

    def _delete(self):
        self.obj.detach()

    def __get_last_modified(self):
        return datetime.datetime(
            *(time.strptime(
                self.obj.lastModified.toString("%Y-%m-%dT%H:%M:%SZ"),
                "%Y-%m-%dT%H:%M:%SZ")[0:6]
            )
        )    
    last_modified = property(__get_last_modified)

    def __get_publicID(self):
        return self.obj.publicID()

    def __set_publicID(self, arg):
        if self.__get_publicID() != arg:
            self._needsUpdate = True
        self.obj.setPublicID(arg)
    publicID = property(__get_publicID,__set_publicID)

    def _new_route(self, **args):
        publicID = args.get("publicID")
        if publicID and DataModel.Route.Find(publicID): publicID = None
        if publicID: obj = DataModel.Route.Create(publicID)
        else: obj = DataModel.Route.Create()
        try: obj.setNetworkCode(args["networkCode"])
        except KeyError: pass
        try: obj.setStationCode(args["stationCode"])
        except KeyError: pass
        try: obj.setLocationCode(args["locationCode"])
        except KeyError: pass
        try: obj.setStreamCode(args["streamCode"])
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Routing: error adding Route"
        return obj
    def __get_route(self):
        list = []
        if dbQuery is None:
            if (self.obj.routeCount()):
                for i in xrange(self.obj.routeCount()):
                    obj = self.obj.route(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_route(obj))
        else:
            # HACK to make last_modified usable ...
            it = dbQuery.getObjects(self.obj, DataModel.Route.TypeInfo())
            while it.get():
                try:
                    obj = DataModel.Route.Cast(it.get())
                    obj.lastModified = it.lastModified()
                    list.append(base_route(obj))
                except Core.ValueException, e:
                    print e.what()
                it.step()
        return list
    _route = property(__get_route)

    def _new_access(self, **args):
        try: obj = DataModel.Access()
        except KeyError: pass
        try: obj.setNetworkCode(args["networkCode"])
        except KeyError: pass
        try: obj.setStationCode(args["stationCode"])
        except KeyError: pass
        try: obj.setLocationCode(args["locationCode"])
        except KeyError: pass
        try: obj.setStreamCode(args["streamCode"])
        except KeyError: pass
        try: obj.setUser(args["user"])
        except KeyError: pass
        try: obj.setStart(Core.Time.FromString(str(args["start"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        try:
            if args["end"] is None:
                obj.setEnd(None)
            else:
                obj.setEnd(Core.Time.FromString(str(args["end"]), "%Y-%m-%d %H:%M:%S"))
        except KeyError: pass
        if not self.obj.add(obj):
            print "seiscomp3.DataModel.Routing: error adding Access"
        return obj
    def __get_access(self):
        list = []
        if dbQuery is None:
            if (self.obj.accessCount()):
                for i in xrange(self.obj.accessCount()):
                    obj = self.obj.access(i)
                    obj.lastModified = Core.Time.GMT()
                    list.append(base_access(obj))
        else:
            # HACK to make last_modified usable ...
            i = 0
            objects_left = self.obj.accessCount()
            while objects_left > 0:
                try:
                    obj = self.obj.access(i)
                    try:
                        obj.lastModified = self.obj.lastModified
                        list.append(base_access(obj))
                        objects_left -= 1
                    except AttributeError:
                        try:
                            obj.lastModified = Core.Time.GMT()
                            list.append(base_access(obj))
                            objects_left -= 1
                        except:
                            logs.debug("got " + repr(obj) + " in __get_access(), objects_left=" + str(objects_left))
                    i += 1
                except Core.ValueException, e:
                    print e.what()
        return list
    _access = property(__get_access)


