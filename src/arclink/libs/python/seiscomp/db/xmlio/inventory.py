#***************************************************************************** 
# inventory.py
#
# XML I/O for Inventory (new schema)
#
# (c) 2006 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import xmlwrap as _xmlwrap
from seiscomp.db import DBError

try:
    from xml.etree import cElementTree as ET  # Python 2.5?
except ImportError:
    import cElementTree as ET

_root_tag = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}inventory"

class _UsedSensor(object):
    def __init__(self):
        self.calibration = set()

    def reg_calibration(self, id):
        self.calibration.add(id)

class _UsedDatalogger(object):
    def __init__(self):
        self.decimation = set()
        self.calibration = set()

    def reg_decimation(self, sampleRateNumerator, sampleRateDenominator):
        self.decimation.add((sampleRateNumerator, sampleRateDenominator))

    def reg_calibration(self, id):
        self.calibration.add(id)

class _UsedAuxilliaryDevice(object):
    def __init__(self):
        self.source = set()

    def reg_source(self, id):
        self.source.add(id)

class _UsedInstruments(object):
    def __init__(self):
        self.sensor = {}
        self.datalogger = {}
        self.auxDevice = {}

    def reg_sensor(self, name):
        try:
            return self.sensor[name]

        except KeyError:
            obj = _UsedSensor()
            self.sensor[name] = obj
            return obj

    def reg_datalogger(self, name):
        try:
            return self.datalogger[name]

        except KeyError:
            obj = _UsedDatalogger()
            self.datalogger[name] = obj
            return obj

    def reg_auxDevice(self, name):
        try:
            return self.auxDevice[name]

        except KeyError:
            obj = _UsedAuxilliaryDevice()
            self.auxDevice[name] = obj
            return obj

class _UsedFilters(object):
    def __init__(self):
        self.filters = set()

    def reg_filter(self, name):
        self.filters.add(name)


#***************************************************************************** 
# XML IN (instruments)
#***************************************************************************** 

def _responseFIR_in(xresponseFIR, inventory):
    if xresponseFIR.action == "delete":
        try:
            inventory.remove_responseFIR(xresponseFIR.name)
        except KeyError:
            pass
        
        return

    try:
        responseFIR = inventory.responseFIR[xresponseFIR.name]
        if responseFIR.publicID != xresponseFIR.publicID:
            inventory.remove_responseFIR(xresponseFIR.name)
            raise KeyError

    except KeyError:
        responseFIR = inventory.insert_responseFIR(xresponseFIR.name, publicID=xresponseFIR.publicID)
        
    xresponseFIR._copy_to(responseFIR)

def _responseIIR_in(xresponseIIR, inventory):
    if xresponseIIR.action == "delete":
        try:
            inventory.remove_responseIIR(xresponseIIR.name)
        except KeyError:
            pass

        return

    try:
        responseIIR = inventory.responseIIR[xresponseIIR.name]
        if responseIIR.publicID != xresponseIIR.publicID:
            inventory.remove_responseIIR(xresponseIIR.name)
            raise KeyError

    except KeyError:
        responseIIR = inventory.insert_responseIIR(xresponseIIR.name, publicID=xresponseIIR.publicID)

    xresponseIIR._copy_to(responseIIR)

def _responsePAZ_in(xresponsePAZ, inventory):
    if xresponsePAZ.action == "delete":
        try:
            inventory.remove_responsePAZ(xresponsePAZ.name)
        except KeyError:
            pass
        
        return

    try:
        responsePAZ = inventory.responsePAZ[xresponsePAZ.name]
        if responsePAZ.publicID != xresponsePAZ.publicID:
            inventory.remove_responsePAZ(xresponsePAZ.name)
            raise KeyError

    except KeyError:
        responsePAZ = inventory.insert_responsePAZ(xresponsePAZ.name, publicID=xresponsePAZ.publicID)
        
    xresponsePAZ._copy_to(responsePAZ)

def _responsePolynomial_in(xresponsePolynomial, inventory):
    if xresponsePolynomial.action == "delete":
        try:
            inventory.remove_responsePolynomial(xresponsePolynomial.name)
        except KeyError:
            pass
        
        return

    try:
        responsePolynomial = inventory.responsePolynomial[xresponsePolynomial.name]
        if responsePolynomial.publicID != xresponsePolynomial.publicID:
            inventory.remove_responsePolynomial(xresponsePolynomial.name)
            raise KeyError

    except KeyError:
        responsePolynomial = inventory.insert_responsePolynomial(xresponsePolynomial.name, publicID=xresponsePolynomial.publicID)
        
    xresponsePolynomial._copy_to(responsePolynomial)

def _responseFAP_in(xresponseFAP, inventory):
    if xresponseFAP.action == "delete":
        try:
            inventory.remove_responseFAP(xresponseFAP.name)
        except KeyError:
            pass

        return

    try:
        responseFAP = inventory.responseFAP[xresponseFAP.name]
        if responseFAP.publicID != xresponseFAP.publicID:
            inventory.remove_responseFAP(xresponseFAP.name)
            raise KeyError

    except KeyError:
        responseFAP = inventory.insert_responseFAP(xresponseFAP.name, publicID=xresponseFAP.publicID)

    xresponseFAP._copy_to(responseFAP)

def _decimation_in(xdecim, device):
    if xdecim.action == "delete":
        try:
            device.remove_decimation(xdecim.sampleRateNumerator,
                xdecim.sampleRateDenominator)
        except KeyError:
            pass
        
        return

    try:
        decim = device.decimation[xdecim.sampleRateNumerator][xdecim.sampleRateDenominator]

    except KeyError:
        decim = device.insert_decimation(xdecim.sampleRateNumerator,
            xdecim.sampleRateDenominator)
        
    xdecim._copy_to(decim)




def _calibration_in(xcalib, device):
    if xcalib.action == "delete":
        try:
            device.remove_calibration(xcalib.serialNumber, xcalib.channel, xcalib.start)
        except KeyError:
            pass
        
        return

    try:
        calib = device.calibration[xcalib.serialNumber][xcalib.channel][xcalib.start]

    except KeyError:
        calib = device.insert_calibration(xcalib.serialNumber, xcalib.channel, xcalib.start)
        
    xcalib._copy_to(calib)


def _datalogger_in(xlogger, inventory):
    if xlogger.action == "delete":
        try:
            inventory.remove_datalogger(xlogger.name)
        except KeyError:
            pass
        
        return

    try:
        logger = inventory.datalogger[xlogger.name]
        if logger.publicID != xlogger.publicID:
            inventory.remove_datalogger(xlogger.name)
            raise KeyError

    except KeyError:
        logger = inventory.insert_datalogger(xlogger.name, publicID=xlogger.publicID)
        
    xlogger._copy_to(logger)

    for xdecim in xlogger.decimation:
        _decimation_in(xdecim, logger)

    for xcalib in xlogger.calibration:
        _calibration_in(xcalib, logger)

def _sensor_in(xsensor, inventory):
    if xsensor.action == "delete":
        try:
            inventory.remove_sensor(xsensor.name)
        except KeyError:
            pass
        
        return

    try:
        sensor = inventory.sensor[xsensor.name]
        if sensor.publicID != xsensor.publicID:
            inventory.remove_sensor(xsensor.name)
            raise KeyError

    except KeyError:
        sensor = inventory.insert_sensor(xsensor.name, publicID=xsensor.publicID)
        
    xsensor._copy_to(sensor)

    for xcalib in xsensor.calibration:
        _calibration_in(xcalib, sensor)

def _aux_source_in(xaux_source, auxDevice):
    if xaux_source.action == "delete":
        try:
            auxDevice.remove_auxSource(xaux_source.name)
        except KeyError:
            pass
        
        return

    try:
        aux_source = auxDevice.aux_source[xaux_source.name]

    except KeyError:
        aux_source = auxDevice.insert_auxSource(xaux_source.name)
        
    xaux_source._copy_to(aux_source)

def _auxDevice_in(xauxDevice, inventory):
    if xauxDevice.action == "delete":
        try:
            inventory.remove_auxDevice(xauxDevice.name)
        except KeyError:
            pass
        
        return

    try:
        auxDevice = inventory.auxDevice[xauxDevice.name]
        if auxDevice.publicID != xauxDevice.publicID:
            inventory.remove_auxDevice(xauxDevice.name)
            raise KeyError

    except KeyError:
        auxDevice = inventory.insert_auxDevice(xauxDevice.name, publicID=xauxDevice.publicID)
        
    xauxDevice._copy_to(auxDevice)

    for xaux_source in xauxDevice.source:
        _aux_source_in(xaux_source, auxDevice)

#***************************************************************************** 
# XML IN (stations)
#***************************************************************************** 

def _Comment_in(xcom, elem):
    if xcom.action == "delete":
        try:
            elem.remove_comment(xcom.id)
        except KeyError:
            pass
        return
    try:
        com = elem.comment[xcom.id]
    except KeyError:
        com = elem.insert_comment(xcom.id)
    xcom._copy_to(com)


def _AuxStream_in(xaux, sl):
    if xaux.action == "delete":
        try:
            sl.remove_auxStream(xaux.code, xaux.start)
        except KeyError:
            pass
        return
    try:
        aux = sl.auxStream[xaux.code][xaux.start]
    except KeyError:
        aux = sl.insert_auxStream(xaux.code, xaux.start)
    xaux._copy_to(aux)


def _Stream_in(xstrm, sta):
    if xstrm.action == "delete":
        try:
            sta.remove_Stream(xstrm.code, xstrm.start)
        except KeyError:
            pass
        return
    try:
        strm = sta.stream[xstrm.code][xstrm.start]
    except KeyError:
        strm = sta.insert_stream(xstrm.code, xstrm.start)
    xstrm._copy_to(strm)

    for xcom in xstrm.comment:
        _Comment_in(xcom, strm)


def _SensorLocation_in(xsl, sta):
    if xsl.action == "delete":
        try:
            sta.remove_sensorLocation(xsl.code, xsl.start)
        except KeyError:
            pass
        return
    try:
        sl = sta.sensorLocation[xsl.code][xsl.start]
        if sl.publicID != xsl.publicID:
            sta.remove_sensorLocation(xsl.code, xsl.start)
            raise KeyError

    except KeyError:
        sl = sta.insert_sensorLocation(xsl.code, xsl.start, publicID=xsl.publicID)
    xsl._copy_to(sl)
    for xStream in xsl.stream:
        _Stream_in(xStream, sl)
    for xAuxStream in xsl.auxStream:
        _AuxStream_in(xAuxStream, sl)

    for xcom in xsl.comment:
        _Comment_in(xcom, sl)


def _Station_in(xsta, net):
    if xsta.action == "delete":
        try:
            net.remove_station(xsta.code, xsta.start)
        except KeyError:
            pass
        return
    try:
        sta = net.station[xsta.code][xsta.start]
        if sta.publicID != xsta.publicID:
            net.remove_station(xsta.code, xsta.start)
            raise KeyError

    except KeyError:
        sta = net.insert_station(xsta.code, xsta.start, publicID=xsta.publicID)

    xsta._copy_to(sta)
    for xsensorLocation in xsta.sensorLocation:
        _SensorLocation_in(xsensorLocation, sta)

    for xcom in xsta.comment:
        _Comment_in(xcom, sta)


def _Network_in(xnet, inventory):
    if xnet.action == "delete":
        try:
            inventory.remove_network(xnet.code, xnet.start)
        except KeyError:
            pass
        return
    try:
        net = inventory.network[xnet.code][xnet.start]
        if net.publicID != xnet.publicID:
            inventory.remove_network(xnet.code, xnet.start)
            raise KeyError

    except KeyError:
        net = inventory.insert_network(xnet.code, xnet.start, publicID=xnet.publicID)
    xnet._copy_to(net)
    for xsta in xnet.station:
        _Station_in(xsta, net)

    for xcom in xnet.comment:
        _Comment_in(xcom, net)


def _StationReference_in(xsref, gr):
    if xsref.action == "delete":
        try:
            inventory.remove_stationReference(xsref.stationID)
        except KeyError:
            pass
        return
    try:
        sref = gr.stationReference[xsref.stationID]
    except KeyError:
        sref = gr.insert_stationReference(xsref.stationID)
    xsref._copy_to(sref)


def _StationGroup_in(xgr, inventory):
    if xgr.action == "delete":
        try:
            inventory.remove_stationGroup(xgr.code)
        except KeyError:
            pass
        return
    try:
        gr = inventory.stationGroup[xgr.code]
        if gr.publicID != xgr.publicID:
            inventory.remove_stationGroup(xgr.code)
            raise KeyError

    except KeyError:
        gr = inventory.insert_stationGroup(xgr.code, publicID=xgr.publicID)
    xgr._copy_to(gr)
    for xsref in xgr.stationReference:
        _StationReference_in(xsref, gr)


#***************************************************************************** 
# XML IN (doc)
#***************************************************************************** 

def _xmldoc_in(xinventory, inventory):
    for xresponseFIR in xinventory.responseFIR:
        _responseFIR_in(xresponseFIR, inventory)

    for xresponseIIR in xinventory.responseIIR:
        _responseIIR_in(xresponseIIR, inventory)

    for xresponsePAZ in xinventory.responsePAZ:
        _responsePAZ_in(xresponsePAZ, inventory)

    for xresponsePolynomial in xinventory.responsePolynomial:
        _responsePolynomial_in(xresponsePolynomial, inventory)

    for xresponseFAP in xinventory.responseFAP:
        _responseFAP_in(xresponseFAP, inventory)

    for xsensor in xinventory.sensor:
        _sensor_in(xsensor, inventory)

    for xlogger in xinventory.datalogger:
        _datalogger_in(xlogger, inventory)

    for xauxDevice in xinventory.auxDevice:
        _auxDevice_in(xauxDevice, inventory)
        
    for xnet in xinventory.network:
        _Network_in(xnet, inventory)
    
    for xgr in xinventory.stationGroup:
        _StationGroup_in(xgr, inventory)
    
#***************************************************************************** 
# XML OUT (instruments)
#***************************************************************************** 

def _responseFIR_out(xinventory, responseFIR, modified_after):
    if modified_after is None or responseFIR.last_modified >= modified_after:
        xresponseFIR = xinventory._new_responseFIR()
        xresponseFIR._copy_from(responseFIR)
        xinventory._append_child(xresponseFIR)
        return True

    return False

def _responseIIR_out(xinventory, responseIIR, modified_after):
    if modified_after is None or responseIIR.last_modified >= modified_after:
        xresponseIIR = xinventory._new_responseIIR()
        xresponseIIR._copy_from(responseIIR)
        xinventory._append_child(xresponseIIR)
        return True

    return False

def _responsePAZ_out(xinventory, responsePAZ, modified_after):
    if modified_after is None or responsePAZ.last_modified >= modified_after:
        xresponsePAZ = xinventory._new_responsePAZ()
        xresponsePAZ._copy_from(responsePAZ)
        xinventory._append_child(xresponsePAZ)
        return True

    return False
    
def _responsePolynomial_out(xinventory, responsePolynomial, modified_after):
    if modified_after is None or responsePolynomial.last_modified >= modified_after:
        xresponsePolynomial = xinventory._new_responsePolynomial()
        xresponsePolynomial._copy_from(responsePolynomial)
        xinventory._append_child(xresponsePolynomial)
        return True

    return False
    
def _responseFAP_out(xinventory, responseFAP, modified_after):
    if modified_after is None or responseFAP.last_modified >= modified_after:
        xresponseFAP = xinventory._new_responseFAP()
        xresponseFAP._copy_from(responseFAP)
        xinventory._append_child(xresponseFAP)
        return True

    return False

def _decimation_out(xdevice, decim, modified_after, filters):
    if filters is not None:
        for f in str(decim.digitalFilterChain).split():
                filters.reg_filter(f)
        for f in str(decim.analogueFilterChain).split():
                filters.reg_filter(f)

    if modified_after is None or decim.last_modified >= modified_after:
        xdecim = xdevice._new_decimation()
        xdecim._copy_from(decim)
        xdevice._append_child(xdecim)
        return True

    return False

def _gain_out(xcalib, gain, modified_after):
    if modified_after is None or gain.last_modified >= modified_after:
        xgain = xcalib._new_gain()
        xgain._copy_from(gain)
        xcalib._append_child(xgain)
        return True

    return False



def _dataloggerCalibration_out(xdevice, calib, modified_after):
    xcalib = xdevice._new_calibration()
    
    if modified_after is None or calib.last_modified >= modified_after:
        xcalib._copy_from(calib)
        retval = True
    else:
        xcalib.serialNumber = calib.serialNumber
        retval = False

    if retval:
        xdevice._append_child(xcalib)

    return retval

def _sensorCalibration_out(xdevice, calib, modified_after):
    xcalib = xdevice._new_calibration()
    
    if modified_after is None or calib.last_modified >= modified_after:
        xcalib._copy_from(calib)
        retval = True
    else:
        xcalib.serialNumber = calib.serialNumber
        retval = False

    if retval:
        xdevice._append_child(xcalib)

    return retval


def _datalogger_out(xinventory, logger, modified_after, used, filters):
    xlogger = xinventory._new_datalogger()

    if modified_after is None or logger.last_modified >= modified_after:
        xlogger._copy_from(logger)
        retval = True
    else:
        xlogger.publicID = logger.publicID
        retval = False

    for i in logger.decimation.itervalues():
        for j in i.itervalues():
            if used is None or \
                (j.sampleRateNumerator, j.sampleRateDenominator) in used.decimation:
                retval |= _decimation_out(xlogger, j, modified_after, filters)
    
    for c in [t for sn in logger.calibration.itervalues()\
        for ch in sn.itervalues()\
            for t in ch.itervalues()]:
        if used is None or c.serialNumber in used.calibration:
            retval |= _dataloggerCalibration_out(xlogger, c, modified_after)
    
    if retval:
        xinventory._append_child(xlogger)

    return retval

def _sensor_out(xinventory, sensor, modified_after, used, filters):
    xsensor = xinventory._new_sensor()
    if filters is not None and sensor.response != "":
        filters.reg_filter(sensor.response)
        
    if modified_after is None or sensor.last_modified >= modified_after:
        xsensor._copy_from(sensor)
        retval = True
    else:
        xsensor.publicID = sensor.publicID
        retval = False
    
    for c in [t for sn in sensor.calibration.itervalues()\
        for ch in sn.itervalues()\
            for t in ch.itervalues()]:
        if used is None or c.serialNumber in used.calibration:
            retval |= _sensorCalibration_out(xsensor, c, modified_after)

    if retval:
        xinventory._append_child(xsensor)

    return retval

def _aux_source_out(xauxDevice, aux_source, modified_after):
    if modified_after is None or aux_source.last_modified >= modified_after:
        xaux_source = xauxDevice._new_source()
        xaux_source._copy_from(aux_source)
        xauxDevice._append_child(xaux_source)
        return True

    return False

def _auxDevice_out(xinventory, auxDevice, modified_after, used):
    xauxDevice = xinventory._new_auxDevice()

    if modified_after is None or auxDevice.last_modified >= modified_after:
        xauxDevice._copy_from(auxDevice)
        retval = True
    else:
        xauxDevice.publicID = auxDevice.publicID
        retval = False

    for i in auxDevice.source.itervalues():
        if used is None or i.name in used.source:
            retval |= _aux_source_out(xauxDevice, i, modified_after)

    if retval:
        xinventory._append_child(xauxDevice)

    return retval

#***************************************************************************** 
# XML OUT (stations)
#***************************************************************************** 


def _Comment_out(xelem, com, modified_after, used_instr):
    if modified_after is None or com.last_modified >= modified_after:
        xcom = xelem._new_comment()
        xcom._copy_from(com)
        xelem._append_child(xcom)
        return True
    return False



def _Stream_out(xsl, strm, modified_after, used_instr):
    if used_instr is not None:
        used_sensor = used_instr.reg_sensor(strm.sensor)
        if strm.sensorSerialNumber != "":
            used_sensor.reg_calibration(strm.sensorSerialNumber)
        used_logger = used_instr.reg_datalogger(strm.datalogger)
        used_logger.reg_decimation(strm.sampleRateNumerator, strm.sampleRateDenominator)
        if strm.dataloggerSerialNumber != "":
            used_logger.reg_calibration(strm.dataloggerSerialNumber)
    if modified_after is None or strm.last_modified >= modified_after:
        xstrm = xsl._new_stream()
        xstrm._copy_from(strm)
        retval = True
    else:
        xstrm.code = strm.code
        xstrm.start = strm.start
        retval = False
    for i in strm.comment.itervalues():
        retval |= _Comment_out(xstrm, i, modified_after, used_instr)
    if retval:
        xsl._append_child(xstrm)
    return retval



def _AuxStream_out(xsl, aux, modified_after, used_instr):
    if used_instr is not None:
        used_auxDevice = used_instr.reg_auxDevice(aux.device)
        used_auxDevice.reg_source(aux.source)
    if modified_after is None or aux.last_modified >= modified_after:
        xaux = xsl._new_auxStream()
        xaux._copy_from(aux)
        xsl._append_child(xaux)
        return True
    return False




def _SensorLocation_out(xsta, sl, modified_after, used_instr):
    xsl = xsta._new_sensorLocation()
    if modified_after is None or sl.last_modified >= modified_after:
        xsl._copy_from(sl)
        retval = True
    else:
        xsl.code = sl.code
        xsl.start = sl.start
        retval = False
    for i in sl.stream.itervalues():
        for j in i.itervalues():
            retval |= _Stream_out(xsl, j, modified_after, used_instr)
    for i in sl.auxStream.itervalues():
        for j in i.itervalues():
            retval |= _AuxStream_out(xsl, j, modified_after, used_instr)
    for i in sl.comment.itervalues():
        retval |= _Comment_out(xsl, i, modified_after, used_instr)
    if retval:
        xsta._append_child(xsl)
    return retval




def _Station_out(xnet, sta, modified_after, used_instr):
    xsta = xnet._new_station()
    if modified_after is None or sta.last_modified >= modified_after:
        xsta._copy_from(sta)
        retval = True
    else:
        xsta.code = sta.code
        xsta.start = sta.start
        retval = False
    for i in sta.sensorLocation.itervalues():
        for j in i.itervalues():
            retval |= _SensorLocation_out(xsta, j, modified_after, used_instr)
    for i in sta.comment.itervalues():
        retval |= _Comment_out(xsta, i, modified_after, used_instr)
    if retval:
        xnet._append_child(xsta)
    return retval




def _Network_out(xinventory, net, modified_after, used_instr):
    xnet = xinventory._new_network()
    if modified_after is None or net.last_modified >= modified_after:
        xnet._copy_from(net)
        retval = True
    else:
        xnet.code = net.code
        xnet.start = net.start
        retval = False
    for i in net.station.itervalues():
        for j in i.itervalues():
            retval |= _Station_out(xnet, j, modified_after, used_instr)
    for i in net.comment.itervalues():
        retval |= _Comment_out(xnet, i, modified_after, used_instr)
    if retval:
        xinventory._append_child(xnet)
    return retval




def _StationReference_out(xgr, sref, modified_after, used_instr):
    xsref = xgr._new_stationReference()
    xsref._copy_from(sref)
    xgr._append_child(xsref)
    return True



def _StationGroup_out(xinventory, gr, modified_after, used_instr):
    if modified_after is None or gr.last_modified >= modified_after:
        xgr = xinventory._new_stationGroup()
        xgr._copy_from(gr)
        xinventory._append_child(xgr)
        for i in gr.stationReference.itervalues():
            _StationReference_out(xgr, i, modified_after, used_instr)
        return True

    return False

#***************************************************************************** 
# XML OUT (doc)
#***************************************************************************** 

def _xmldoc_out(xinventory, inventory, instr, modified_after):
    if instr == 0:
        for i in inventory.network.itervalues():
            for j in i.itervalues():
                _Network_out(xinventory, j, modified_after, None)

        for i in inventory.stationGroup.itervalues():
            _StationGroup_out(xinventory, i, modified_after, None)

    elif instr == 1:
        used_instr = _UsedInstruments()
        used_filters = _UsedFilters()

        for i in inventory.network.itervalues():
            for j in i.itervalues():
                _Network_out(xinventory, j, modified_after, used_instr)

        for i in inventory.stationGroup.itervalues():
            _StationGroup_out(xinventory, i, modified_after, None)

        for i in inventory.datalogger.itervalues():
            used_logger = used_instr.datalogger.get(i.publicID)
            if used_logger is not None:
                _datalogger_out(xinventory, i, modified_after, used_logger, used_filters)

        for i in inventory.sensor.itervalues():
            used_sensor = used_instr.sensor.get(i.publicID)
            if used_sensor is not None:
                _sensor_out(xinventory, i, modified_after, used_sensor, used_filters)

        for i in inventory.auxDevice.itervalues():
            used_auxDevice = used_instr.auxDevice.get(i.publicID)
            if used_auxDevice is not None:
                _auxDevice_out(xinventory, i, modified_after, used_auxDevice)

        for i in inventory.responseFIR.itervalues():
            if i.publicID in used_filters.filters:
                _responseFIR_out(xinventory, i, modified_after)

        for i in inventory.responseIIR.itervalues():
            if i.publicID in used_filters.filters:
                _responseIIR_out(xinventory, i, modified_after)

        for i in inventory.responsePAZ.itervalues():
            if i.publicID in used_filters.filters:
                _responsePAZ_out(xinventory, i, modified_after)

        for i in inventory.responsePolynomial.itervalues():
            if i.publicID in used_filters.filters:
                _responsePolynomial_out(xinventory, i, modified_after)

        for i in inventory.responseFAP.itervalues():
            if i.publicID in used_filters.filters:
                _responseFAP_out(xinventory, i, modified_after)
    elif instr == 2:
        for i in inventory.network.itervalues():
            for j in i.itervalues():
                _Network_out(xinventory, j, modified_after, None)
        
        for i in inventory.stationGroup.itervalues():
            _StationGroup_out(xinventory, i, modified_after, None)

        for i in inventory.datalogger.itervalues():
            _datalogger_out(xinventory, i, modified_after, None, None)

        for i in inventory.sensor.itervalues():
            _sensor_out(xinventory, i, modified_after, None, None)

        for i in inventory.auxDevice.itervalues():
            _auxDevice_out(xinventory, i, modified_after, used_auxDevice)

        for i in inventory.responseFIR.itervalues():
            _responseFIR_out(xinventory, i, modified_after)

        for i in inventory.responseIIR.itervalues():
            _responseIIR_out(xinventory, i, modified_after)

        for i in inventory.responsePAZ.itervalues():
            _responsePAZ_out(xinventory, i, modified_after)

        for i in inventory.responsePolynomial.itervalues():
            _responsePolynomial_out(xinventory, i, modified_after)

        for i in inventory.responseFAP.itervalues():
            _responseFAP_out(xinventory, i, modified_after)

#***************************************************************************** 
# Incremental Parser
#*****************************************************************************

class _IncrementalParser(object):
    def __init__(self, inventory):
        self.__inventory = inventory
        self.__p = ET.XMLTreeBuilder()

    def feed(self, s):
        self.__p.feed(s)

    def close(self):
        root = self.__p.close()
        if root.tag != _root_tag:
            raise DBError, "unrecognized root element: " + root.tag

        xinventory = _xmlwrap.xml_Inventory(root)
        _xmldoc_in(xinventory, self.__inventory)
        
#***************************************************************************** 
# Public functions
#*****************************************************************************

def make_parser(inventory):
    return _IncrementalParser(inventory)

def xml_in(inventory, src):
    doc = ET.parse(src)
    root = doc.getroot()
    if root.tag != _root_tag:
        raise DBError, "unrecognized root element: " + root.tag

    xinventory = _xmlwrap.xml_Inventory(root)
    _xmldoc_in(xinventory, inventory)

def _indent(elem, level=0):
    s = 1*"\t" # indent char
    i = "\n" + level*s
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + s
        for e in elem:
            _indent(e, level+1)
            if not e.tail or not e.tail.strip():
                e.tail = i + s
        if not e.tail or not e.tail.strip():
            e.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

def xml_out(inventory, dest, instr=0, modified_after=None, stylesheet=None, indent=True):
    xinventory = _xmlwrap.xml_Inventory()
    
    _xmldoc_out(xinventory, inventory, instr, modified_after)

    if isinstance(dest, basestring):
        fd = file(dest, "w")
    elif hasattr(dest, "write"):
        fd = dest
    else:
        raise TypeError, "invalid file object"

    try:
        filename = fd.name
    except AttributeError:
        filename = '<???>'

    fd.write('<?xml version="1.0" encoding="utf-8"?>\n')

    if stylesheet != None:
        fd.write('<?xml-stylesheet type="application/xml" href="%s"?>\n' % \
            (stylesheet,))
    
    if indent is True:
        _indent(xinventory._element)
    
    ET.ElementTree(xinventory._element).write(fd, encoding="utf-8")
    
    if isinstance(dest, basestring):
        fd.close()

