#***************************************************************************** 
# inventory.py
#
# seiscomp3-based Inventory implementation
#
# (c) 2006 Andres Heinloo, GFZ Potsdam
# (c) 2007 Mathias Hoffmann, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import re,sys
import datetime
from seiscomp.db.seiscomp3 import sc3wrap as _sc3wrap
from seiscomp.db.xmlio import inventory as _xmlio
from seiscomp.db import DBError

from seiscomp3 import DataModel, Core

#
# arclink's  i n v e n t o r y  representation
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class _ResponseFIR(_sc3wrap.base_responsefir):
	def __init__(self, so):
		_sc3wrap.base_responsefir.__init__(self, so)
	
	def flush(self):
		self._sync_update()

class _ResponsePAZ(_sc3wrap.base_responsepaz):
	def __init__(self, so):
		_sc3wrap.base_responsepaz.__init__(self, so)
	
	def flush(self):
		self._sync_update()

class _ResponsePolynomial(_sc3wrap.base_responsepolynomial):
	def __init__(self, so):
		_sc3wrap.base_responsepolynomial.__init__(self, so)
	
	def flush(self):
		self._sync_update()

class _Decimation(_sc3wrap.base_decimation):
	def __init__(self, so):
		_sc3wrap.base_decimation.__init__(self, so)
		self.mydevice = None
	
	def flush(self):
		self._sync_update()

class _DataloggerCalibration(_sc3wrap.base_dataloggercalibration):
	def __init__(self, so):
		_sc3wrap.base_dataloggercalibration.__init__(self, so)
		self.mydevice = None
	
	def flush(self):
		self._sync_update()

class _Datalogger(_sc3wrap.base_datalogger):
	def __init__(self, so):
		_sc3wrap.base_datalogger.__init__(self, so)
		self.decimation = {}
		self.calibration = {}
	
	def _link_decimation(self, obj):
		if obj.sampleRateNumerator not in self.decimation:
			self.decimation[obj.sampleRateNumerator] = {}

		self.decimation[obj.sampleRateNumerator][obj.sampleRateDenominator] = obj
		obj.mydevice = self

	def _link_calibration(self, obj):
		if obj.serialNumber not in self.calibration:
			self.calibration[obj.serialNumber] = {}

		if obj.channel not in self.calibration[obj.serialNumber]:
			self.calibration[obj.serialNumber][obj.channel] = {}
		
		self.calibration[obj.serialNumber][obj.channel][obj.start] = obj
		obj.mydevice = self

	def insert_decimation(self, sampleRateNumerator, sampleRateDenominator, **args):
		obj = _Decimation(self._new_decimation(sampleRateNumerator=sampleRateNumerator,
			sampleRateDenominator=sampleRateDenominator, **args))
		self._link_decimation(obj)
		return obj

	def insert_calibration(self, serialNumber, channel, start, **args):
		obj = _DataloggerCalibration(self._new_dataloggercalibration(serialNumber=serialNumber, channel=channel, start=start, **args))
		self._link_calibration(obj)
		return obj

	def remove_decimation(self, sampleRateNumerator, sampleRateDenominator):
		try:
			self.decimation[(sampleRateNumerator, sampleRateDenominator)]._delete()
			del self.decimation[(sampleRateNumerator, sampleRateDenominator)]

		except KeyError:
			raise DBError, "decimation %d/%d not found" % \
				(sampleRateNumerator, sampleRateDenominator)

	def remove_calibration(self, serialNumber, channel, start):
		try:
			self.calibration[serialNumber][channel][start]._delete()
			del self.calibration[serialNumber][channel][start]
			if len(self.calibration[serialNumber][channel]) == 0:
				del self.calibration[serialNumber][channel]
				if len(self.calibration[serialNumber]) == 0:
					del self.calibration[serialNumber]

		except KeyError:
			raise DBError, "calibration of datalogger [%s][%s][%s] not found" % (serialNumber, channeli, start)

	def flush(self):
		self._sync_update()

		for i in self.decimation.itervalues():
			for j in i.itervalues():
				j.flush()

		for i in self.calibration.itervalues():
			for j in i.itervalues():
				for k in j.itervalues():
					k.flush()

class _SeismometerCalibration(_sc3wrap.base_sensorcalibration):
	def __init__(self, so):
		_sc3wrap.base_sensorcalibration.__init__(self, so)
		self.mydevice = None
	
	def flush(self):
		self._sync_update()

class _Seismometer(_sc3wrap.base_sensor):
	def __init__(self, so):
		_sc3wrap.base_sensor.__init__(self, so)
		self.calibration = {}

	def _link_calibration(self, obj):
		if obj.serialNumber not in self.calibration:
			self.calibration[obj.serialNumber] = {}

		if obj.channel not in self.calibration[obj.serialNumber]:
			self.calibration[obj.serialNumber][obj.channel] = {}
		
		self.calibration[obj.serialNumber][obj.channel][obj.start] = obj
		obj.mysensor = self

	def insert_calibration(self, serialNumber, channel, start, **args):
		obj = _DataloggerCalibration(self._new_sensorcalibration(serialNumber=serialNumber, channel=channel, start=start, **args))
		self._link_calibration(obj)
		return obj

	def remove_calibration(self, serialNumber, channel, start):
		try:
			self.calibration[serialNumber][channel][start]._delete()
			del self.calibration[serialNumber][channel][start]
			if len(self.calibration[serialNumber][channel]) == 0:
				del self.calibration[serialNumber][channel]
				if len(self.calibration[serialNumber]) == 0:
					del self.calibration[serialNumber]

		except KeyError:
			raise DBError, "calibration of sensor [%s][%s][%s] not found" % (serialNumber, channeli, start)

	def flush(self):
		self._sync_update()
		for i in self.calibration.itervalues():
			for j in i.itervalues():
				for k in j.itervalues():
					k.flush()


class _AuxilliarySource(_sc3wrap.base_auxsource):
	def __init__(self, so):
		_sc3wrap.base_auxsource.__init__(self, so)
		self.mydevice = None

	def flush(self):
		self._sync_update()

class _AuxilliaryDevice(_sc3wrap.base_auxdevice):
	def __init__(self, so):
		_sc3wrap.base_auxdevice.__init__(self, so)
		self.source = {}

	def _link_source(self, obj):
		self.source[obj.source_id] = obj
		obj.mydevice = self

	def insert_source(self, source_id, **args):
		obj = _AuxilliarySource(self._new_auxsource(source_id=source_id, **args))
		self._link_source(obj)
		return obj

	def remove_source(self, source_id):
		try:
			self.source[source_id]._delete()
			del self.source[source_id]

		except KeyError:
			raise DBError, "auxilliary source %s not found" % (source_id,)

	def flush(self):
		self._sync_update()
		for i in self.source.itervalues():
			i.flush()

class _Stream(_sc3wrap.base_stream):
	def __init__(self, so):
		_sc3wrap.base_stream.__init__(self, so)
		self.mySensorLocation = None

	def flush(self):
		self._sync_update()

class _AuxilliaryStream(_sc3wrap.base_auxstream):
	def __init__(self, so):
		_sc3wrap.base_auxstream.__init__(self, so)
		self.mySensorLocation = None

	def flush(self):
		self._sync_update()

class _SensorLocation(_sc3wrap.base_sensorlocation):
	def __init__(self, so):
		_sc3wrap.base_sensorlocation.__init__(self, so)
		self.myStation = None
		self.stream = {}
		self.auxStream = {}

	def _link_stream(self, obj):
		if obj.code not in self.stream:
			self.stream[obj.code] = {}

		self.stream[obj.code][obj.start] = obj
		obj.mySensorLocation = self
	
	def _link_auxStream(self, obj):
		if obj.code not in self.auxStream:
			self.auxStream[obj.code] = {}

		self.auxStream[obj.code][obj.start] = obj
		obj.mySensorLocation = self
	
	def insert_stream(self, code, start, **args):
		obj = _Stream(self._new_stream(code=code, start=start, **args))
		self._link_stream(obj)
		return obj

	def insert_auxStream(self, code, start, **args):
		obj = _AuxilliaryStream(self._new_auxstream(code=code, start=start, **args))
		self._link_auxStream(obj)
		return obj

	def remove_stream(self, code, start):
		try:
			self.stream[code][start]._delete()
			del self.stream[code][start]
			if len(self.stream[code]) == 0:
				del self.stream[code]

		except KeyError:
			raise DBError, "stream [%s,%s][%s] not found" % (code, loc, start)

	def remove_auxStream(self, code, start):
		try:
			self.auxStream[code][start]._delete()
			del self.auxStream[code][start]
			if len(self.auxStream[code]) == 0:
				del self.auxStream[code]
				
		except KeyError:
			raise DBError, "stream [%s,%s][%s] not found" % (code, loc, start)

	def flush(self):
		self._sync_update()
		for i in self.stream.itervalues():
			for j in i.itervalues():
				j.flush()
		for i in self.auxStream.itervalues():
			for j in i.itervalues():
				j.flush()

class _Station(_sc3wrap.base_station):
	def __init__(self, so):
		_sc3wrap.base_station.__init__(self, so)
		self.myNetwork = None
		self.sensorLocation = {}

	def _link_sensorLocation(self, obj):
		if obj.code not in self.sensorLocation:
			self.sensorLocation[obj.code] = {}

		self.sensorLocation[obj.code][obj.start] = obj
		obj.myStation = self
	
	def insert_sensorLocation(self, code, start, **args):
		obj = _SensorLocation(self._new_sensorlocation(code=code, start=start, **args))
		self._link_sensorLocation(obj)
		return obj

	def remove_sensorLocation(self, code, start):
		try:
			self.sensorLocation[code][start]._delete()
			del self.sensorLocation[code][start]
			if len(self.sensorLocation[code]) == 0:
				del self.sensorLocation[code]

		except KeyError:
			raise DBError, "sensor location [%s][%s] not found" % (code, start)

	def flush(self):
		self._sync_update()
		for i in self.sensorLocation.itervalues():
			for j in i.itervalues():
				j.flush()

class _Network(_sc3wrap.base_network):
	def __init__(self, so):
		_sc3wrap.base_network.__init__(self, so)
		self.station = {}

	def _link_station(self, obj):
		if obj.code not in self.station:
			self.station[obj.code] = {}

		self.station[obj.code][obj.start] = obj
		obj.myNetwork = self
	
	def insert_station(self, code, start, **args):
		obj = _Station(self._new_station(code=code, start=start, **args))
		self._link_station(obj)
		return obj

	def remove_station(self, code, start):
		try:
			self.station[code][start]._delete()
			del self.station[code][start]
			if len(self.station[code]) == 0:
				del self.station[code]

		except KeyError:
			raise DBError, "station [%s][%s] not found" % (code, start)

	def flush(self):
		self._sync_update()
		for i in self.station.itervalues():
			for j in i.itervalues():
				j.flush()
		
class _StationReference(_sc3wrap.base_stationreference):
	def __init__(self, so):
		_sc3wrap.base_stationreference.__init__(self, so)

	def flush(self):
		self._sync_update()

class _StationGroup(_sc3wrap.base_stationgroup):
	def __init__(self, so):
		_sc3wrap.base_stationgroup.__init__(self, so)
		self.stationReference = {}

	def _link_stationReference(self, obj):
		self.stationReference[obj.stationID] = obj
		obj.myStationGroup = self
	
	def insert_stationReference(self, stationID, **args):
		obj = _StationReference(self._new_stationreference(stationID=stationID, **args))
		self._link_stationReference(obj)
		return obj

	def remove_stationReference(self, stationID):
		try:
			self.stationReference[stationID]._delete()
			del self.stationReference[stationID]

		except KeyError:
			raise DBError, "stationReference [%s] not found" % (stationID,)

	def flush(self):
		self._sync_update()
		for i in self.stationReference.itervalues():
			i.flush()
		
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#
# conditional functions
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
def _retr(val):
	if val is None:
		val = ''
	s = re.sub("^[?]$", "*", val)       # single ? --> *
	s = re.sub("[?]", ".", s)           # SN?? --> SN..
	s = re.sub("[+]", ".+", s)          # + --> .+
	s = re.sub("[\*]", ".*", "^"+s+"$") # S*AA --> S.*AA
	return s


def _modifiedAfter(obj, modified_after):
	if (obj.last_modified == None) or (modified_after == None):
		return True
		
	return (obj.last_modified >= modified_after)
	return True

def _inPattern(code, pat):
	return re.match(_retr(pat), code)
	

def _inTimeSpan(obj, start_time, end_time):
	start = True
	end = True
	
	if (obj.start is not None) and (end_time is not None):
		start = (obj.start <= end_time)
	
	if (obj.end is not None) and (start_time is not None):
		end = (obj.end >= start_time)
	
	return (start and end)
	

def _restricted(obj, restricted):
	if (obj.restricted == None) or (restricted == None):
		return True
	
	return (obj.restricted == restricted)
	

def _permanent(netClass, permanent):
	if (netClass == None) or (permanent == None):
		return True
	
	if permanent == True:
		return (netClass == "p")
	elif permanent == False:
		return (netClass == "t")
	

def _inRegion(obj, latmin, latmax, lonmin, lonmax):
	minlat = True
	maxlat = True
	minlon = True
	maxlon = True

	if (latmin is not None):
		minlat = (obj.latitude >= latmin)
	if (latmax is not None):
		maxlat = (obj.latitude <= latmax)
	if (lonmin is not None):
		minlon = (obj.longitude >= lonmin)
	if (lonmax is not None):
		maxlon = (obj.longitude <= lonmax)
	if (lonmin is not None and lonmax is not None and lonmin > lonmax):
		minlon = maxlon = minlon or maxlon
	
	return (minlat and maxlat and minlon and maxlon)
	

def _sensortype(obj, sensortype, sensor_dict):
	if (obj.sensor == None) or (sensortype == None):
		return True
	
	for st in sensortype.split("+"):
		try:
			if (sensor_dict[obj.sensor].type == st):
				return True

		except KeyError:
			pass

	return False

def _qccompliant(wfID, start, end, constraints):
	if not constraints:
		return True

	retval = True
	consdict = dict([ (key,value) for key,value in constraints.items() if value is not None ])
	parset = set([key[:-4].replace("_"," ") for key in consdict.keys()])

	for qcParameter in parset:
		count = 0
		sum = 0
		it = _sc3wrap.dbQuery.getWaveformQuality(wfID,qcParameter,Core.Time.FromString(str(start),"%Y-%m-%d %H:%M:%S"),Core.Time.FromString(str(end),"%Y-%m-%d %H:%M:%S"))
		
		while it.get():
			count += 1
			wfq = DataModel.WaveformQuality.Cast(it.get())
			sum += wfq.value()
			it.step()
		
		try:
			max = consdict.get(qcParameter.replace(" ","_") + "_max")
			retval = retval and ((sum/count) <= float(max))
		except:
			pass
		try:
			min = consdict.get(qcParameter.replace(" ","_") + "_min")
			retval = retval and ((sum/count) >= float(min))
		except:
			pass

		if count == 0:
			retval = False
		
	return retval

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#
# arclink's  i n v e n t o r y  implementation
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class Inventory(_sc3wrap.base_inventory):
	def __init__(self, sc3Inv):	
		_sc3wrap.base_inventory.__init__(self, sc3Inv)
		self.responseFIR = {}
		self.responsePAZ = {}
		self.responsePolynomial = {}
		self.datalogger = {}
		self.sensor = {}
		self.auxDevice = {}
		self.network = {}
		self.stationGroup = {}
		self.object = {}
		
	def _link_responseFIR(self, obj):
		self.responseFIR[obj.name] = obj
		self.object[obj.publicID] = obj

	def _link_responsePAZ(self, obj):
		self.responsePAZ[obj.name] = obj
		self.object[obj.publicID] = obj

	def _link_responsePolynomial(self, obj):
		self.responsePolynomial[obj.name] = obj
		self.object[obj.publicID] = obj

	def _link_datalogger(self, obj):
		self.datalogger[obj.name] = obj
		self.object[obj.publicID] = obj

	def _link_sensor(self, obj):
		self.sensor[obj.name] = obj
		self.object[obj.publicID] = obj

	def _link_auxDevice(self, obj):
		self.auxDevice[obj.name] = obj
		self.object[obj.publicID] = obj

	def _link_network(self, obj):
		if obj.code not in self.network:
			self.network[obj.code] = {}
		self.network[obj.code][obj.start] = obj
	
	def _link_stationGroup(self, obj):
		self.stationGroup[obj.code] = obj
		self.object[obj.publicID] = obj

	def insert_responseFIR(self, name, **args):
		obj = _ResponseFIR(self._new_responsefir(name=name, **args))
		self._link_responseFIR(obj)
		return obj
	
	def insert_responsePAZ(self, name, **args):
		obj = _ResponsePAZ(self._new_responsepaz(name=name, **args))
		self._link_responsePAZ(obj)
		return obj
	
	def insert_responsePolynomial(self, name, **args):
		obj = _ResponsePolynomial(self._new_responsepolynomial(name=name, **args))
		self._link_responsePolynomial(obj)
		return obj
	
	def insert_datalogger(self, name, **args):
		obj = _Datalogger(self._new_datalogger(name=name, **args))
		self._link_datalogger(obj)
		return obj
	
	def insert_sensor(self, name, **args):
		obj = _Seismometer(self._new_sensor(name=name, **args))
		self._link_sensor(obj)
		return obj
	
	def insert_auxDevice(self, name, **args):
		obj = _AuxilliaryDevice(self._new_auxdevice(name=name, **args))
		self._link_auxDevice(obj)
		return obj
	
	def insert_network(self, code, start, **args):
		obj = _Network(self._new_network(code=code, start=start, **args))
		self._link_network(obj)
		return obj

	def insert_stationGroup(self, code, **args):
		obj = _StationGroup(self._new_stationgroup(code=code, **args))
		self._link_stationGroup(obj)
		return obj

	def remove_responseFIR(self, name):
		try:
			self.responseFIR[name]._delete()
			del self.responseFIR[name]

		except KeyError:
			raise DBError, "FIR response %s not found" % (name,)
			
	def remove_responsePAZ(self, name):
		try:
			self.responsePAZ[name]._delete()
			del self.responsePAZ[name]

		except KeyError:
			raise DBError, "PAZ response %s not found" % (name,)
			
	def remove_responsePolynomial(self, name):
		try:
			self.responsePolynomial[name]._delete()
			del self.responsePolynomial[name]

		except KeyError:
			raise DBError, "Polynomial response %s not found" % (name,)
			
	def remove_datalogger(self, name):
		try:
			self.datalogger[name]._delete()
			del self.datalogger[name]

		except KeyError:
			raise DBError, "datalogger %s not found" % (name,)
			
	def remove_sensor(self, name):
		try:
			self.sensor[name]._delete()
			del self.sensor[name]

		except KeyError:
			raise DBError, "sensor %s not found" % (name,)
			
	def remove_auxDevice(self, name):
		try:
			self.auxDevice[name]._delete()
			del self.auxDevice[name]

		except KeyError:
			raise DBError, "auxilliary device %s not found" % (name,)
			
	def remove_network(self, code, start):
		try:
			self.network[code][start]._delete()
			del self.network[code][start]
			if len(self.network[code]) == 0:
				del self.network[code]

		except KeyError:
			raise DBError, "network [%s][%s] not found" % (code, start)

	def remove_stationGroup(self, code):
		try:
			self.stationGroup[code]._delete()
			del self.stationGroup[code]

		except KeyError:
			raise DBError, "station group %s not found" % (code,)
			
	def flush(self):
		for i in self.responseFIR.itervalues():
			i.flush()

		for i in self.responsePAZ.itervalues():
			i.flush()

		for i in self.datalogger.itervalues():
			i.flush()

		for i in self.sensor.itervalues():
			i.flush()

		for i in self.auxDevice.itervalues():
			i.flush()

		for i in self.network.itervalues():
			for j in i.itervalues():
				j.flush()

		for i in self.stationGroup.itervalues():
			i.flush()



	def load_instruments(self, modified_after = None):
		
		#print "inventory.load_instruments()"
		#sys.stdout.flush()
		
		for auxDevice in self._auxDevice:
			if not _modifiedAfter(auxDevice, modified_after):
				continue
			try:
				AD = self.auxDevice[auxDevice.name]
			except:
				AD = _AuxilliaryDevice(auxDevice.obj)
			self._link_auxDevice(AD)
			
			for source in auxDevice._auxSource:
				if not _modifiedAfter(source, modified_after):
					continue
				AD._link_source(_AuxilliarySource(source.obj))
		
		for sensor in self._sensor:
			if not _modifiedAfter(sensor, modified_after):
				continue
			try:
				SM = self.sensor[sensor.name]
			except:
				SM = _Seismometer(sensor.obj)
			self._link_sensor(SM)
			
			for calibration in sensor._sensorCalibration:
				if not _modifiedAfter(calibration, modified_after):
					continue
				try:
					SC = SM.calibration[calibration.serialNumber][calibration.channel][calibration.start]
				except:
					SC = _SeismometerCalibration(calibration.obj)	
				SM._link_calibration(SC)
								
		for respaz in self._responsePAZ:
			if not _modifiedAfter(respaz, modified_after):
				continue
			
			RP = _ResponsePAZ(respaz.obj)
			self._link_responsePAZ(RP)
		
		for resppoly in self._responsePolynomial:
			if not _modifiedAfter(resppoly, modified_after):
				continue
			
			RP = _ResponsePolynomial(resppoly.obj)
			self._link_responsePolynomial(RP)
		
		for respfir in self._responseFIR:
			if not _modifiedAfter(respfir, modified_after):
				continue
			RF = _ResponseFIR(respfir.obj)
			self._link_responseFIR(RF)
			
		for datalogger in self._datalogger:
			if not _modifiedAfter(datalogger, modified_after):
				continue
			try:
				DL = self.datalogger[datalogger.name]
			except:
				DL = _Datalogger(datalogger.obj)
			self._link_datalogger(DL)
			
			for calibration in datalogger._dataloggerCalibration:
				if not _modifiedAfter(calibration, modified_after):
					continue
				try:
					DC = DL.calibration[calibration.serialNumber][calibration.channel]
				except:
					DC = _DataloggerCalibration(calibration.obj)
					DL._link_calibration(DC)
			
			for decimation in datalogger._decimation:
				if not _modifiedAfter(decimation, modified_after):
					continue
				DL._link_decimation(_Decimation(decimation.obj))
		
	
	def clear_instruments(self):
		self.flush()
		self.responseFIR = {}
		self.responsePAZ = {}
		self.datalogger = {}
		self.sensor = {}
		self.auxDevice = {}




	def load_stations(self, net_pat = None, sta_pat = None, chan_pat = None,
		loc_pat = None, start_time = None, end_time = None, sensortype = None,
		latmin = None, latmax = None, lonmin = None, lonmax = None,
		permanent = None, restricted = None, modified_after = None,
		self_dcid = None, sync_dcid = None, qc_constraints=None):
		
		#print "inventory.load_stations()"
		#print net_pat, sta_pat, chan_pat, loc_pat
		#sys.stdout.flush()

		ret = False
		
		if loc_pat is None:
			loc_pat = ""

		vn_stations = {}
		vn_stations_nets = set()

		for stationGroup in self._stationGroup:
			if not _inPattern(stationGroup.code, net_pat):
				continue

			try:
				G = self.stationGroup[stationGroup.code]

			except:
				G = _StationGroup(stationGroup.obj)
				self._link_stationGroup(G)

			for stationReference in stationGroup._stationReference:
				R = _StationReference(stationReference.obj)
#				G._link_stationReference(R)
				try: vn_stations[stationReference.stationID].append((G, R))
				except KeyError: vn_stations[stationReference.stationID] = [(G, R)]

				station_obj = DataModel.Station.Find(stationReference.stationID)
				if station_obj:
					network_obj = station_obj.parent()
					if network_obj:
						vn_stations_nets.add(network_obj.publicID())
		
		for network in self._network:
			stationFound = False
			if not _modifiedAfter(network, modified_after):
				continue
			if not _inPattern(network.code, net_pat) and \
				network.publicID not in vn_stations_nets:
				continue
			if not _inTimeSpan(network, start_time, end_time):
				continue
			if not _restricted(network, restricted):
				continue
			if not _permanent(network.netClass, permanent):
				continue
			try:
				N = self.network[network.code][network.start]
			except:
				N = _Network(network.obj)
			if sta_pat is None and (qc_constraints is None or len(qc_constraints) == 0):
				self._link_network(N)
				continue
			
			for station in network._station:
				locationFound = False
				if not _modifiedAfter(station, modified_after):
					continue
				if not _inPattern(network.code, net_pat) and \
					station.publicID not in vn_stations:
					continue
				if not _inPattern(station.code, sta_pat):
					continue
				if not _inRegion(station, latmin, latmax, lonmin, lonmax):
					continue
				if not _inTimeSpan(station, start_time, end_time):
					continue
				if not _restricted(station, restricted):
					continue
				if self_dcid and (not station.shared or self_dcid == station.archive):
					continue
				if sync_dcid and (not station.shared or sync_dcid != station.archive):
					continue
				try:
					S = N.station[station.code][station.start]
				except:
					S = _Station(station.obj)
				if chan_pat is None and (qc_constraints is None or len(qc_constraints) == 0):
					N._link_station(S)
					stationFound = True

					try:
						for (G, R) in vn_stations[S.publicID]:
							G._link_stationReference(R)
					
					except KeyError:
						pass
					
					continue
				
				for location in station._sensorLocation:
					streamFound = False
					if not _inPattern(location.code, loc_pat):
						continue
					try:
						L = S.sensorLocation[location.code][location.start]
					except:
						L = _SensorLocation(location.obj)

					for stream in location._stream:
						link = (qc_constraints is None or len(qc_constraints) == 0)
						if not _modifiedAfter(stream, modified_after):
							continue
						if not _inPattern(stream.code, chan_pat):
							continue
						if not _inTimeSpan(stream, start_time, end_time):
							continue
						if not _restricted(stream, restricted):
							continue
						if not _sensortype(stream, sensortype, self.object):
							continue
						L._link_stream(_Stream(stream.obj))
						streamFound = True
						
					for auxStream in location._auxStream:
						if not _modifiedAfter(auxStream, modified_after):
							continue
						if not _inPattern(auxStream.code, chan_pat):
							continue
						if not _inTimeSpan(auxStream, start_time, end_time):
							continue
						if not _restricted(auxStream, restricted):
							continue
						if sensortype is not None:
							continue
						L._link_auxStream(_AuxilliaryStream(auxStream.obj))
						streamFound = True
							
					if streamFound:
						S._link_sensorLocation(L)
						locationFound = True
				
				if locationFound:
					N._link_station(S)
					stationFound = True

					try:
						for (G, R) in vn_stations[S.publicID]:
							G._link_stationReference(R)
					
					except KeyError:
						pass
					
			if stationFound:
				self._link_network(N)
				ret = True

		return ret


	# D E B U G
	def print_stations(self):
		print "printing networks, stations, ..."
		for net in sum([i.values() for i in self.network.itervalues()], []):
			print net.code
			for sta in sum([i.values() for i in net.station.itervalues()], []):
				print "---> " + sta.code
				for sl in sum([i.values() for i in sta.sensorLocation.itervalues()], []):
					print "--------> " + sl.code
					for stream in sl.stream.itervalues():
						for t,s in stream.iteritems():
							print "---- seis ---> " + str(t), s.code
					for stream in sl.auxStream.itervalues():
						for t,s in stream.iteritems():
							print "---- aux- ---> " + str(t), s.code
		sys.stdout.flush()

	def clear_stations(self):
		self.flush()
		self.network = {}
		self.stationGroup = {}

	def load_xml(self, src):       # override, prefix?
		_xmlio.xml_in(self, src)

	def save_xml(self, dest, instr=0, modified_after=None, stylesheet=None):
		_xmlio.xml_out(self, dest, instr, modified_after, stylesheet)

	def make_parser(self):
		return _xmlio.make_parser(self)
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
