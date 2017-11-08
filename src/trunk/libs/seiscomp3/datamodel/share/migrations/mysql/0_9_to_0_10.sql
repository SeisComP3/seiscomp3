SELECT 'Creating ResponseIIR' AS '';
CREATE TABLE ResponseIIR (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255),
	type CHAR(1),
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	decimationFactor SMALLINT UNSIGNED,
	delay DOUBLE UNSIGNED,
	correction DOUBLE,
	numberOfNumerators TINYINT UNSIGNED,
	numberOfDenominators TINYINT UNSIGNED,
	numerators_content BLOB,
	numerators_used TINYINT(1) NOT NULL DEFAULT '0',
	denominators_content BLOB,
	denominators_used TINYINT(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

SELECT 'Updating Comment' AS '';
ALTER TABLE Comment
  ADD start DATETIME AFTER id,
  ADD start_ms INTEGER AFTER start,
  ADD end DATETIME AFTER start_ms,
  ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON Comment;
ALTER TABLE Comment ADD CONSTRAINT composite_index UNIQUE(_parent_oid,id);

SELECT 'Updating StationGroup' AS '';
ALTER TABLE StationGroup
  ADD start_ms INTEGER AFTER start,
  ADD end_ms INTEGER AFTER end;
UPDATE StationGroup SET start_ms=0 WHERE start != NULL;
UPDATE StationGroup SET end_ms=0 WHERE end != NULL;

SELECT 'Updating DataloggerCalibration' AS '';
ALTER TABLE DataloggerCalibration
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end,
  MODIFY gain DOUBLE;
ALTER TABLE DataloggerCalibration ALTER start_ms DROP DEFAULT;
UPDATE DataloggerCalibration SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON DataloggerCalibration;
ALTER TABLE DataloggerCalibration ADD CONSTRAINT composite_index UNIQUE(_parent_oid,serialNumber,channel,start,start_ms);

SELECT 'Updating SensorCalibration' AS '';
ALTER TABLE SensorCalibration
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end,
  MODIFY gain DOUBLE;
ALTER TABLE SensorCalibration ALTER start_ms DROP DEFAULT;
UPDATE SensorCalibration SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON SensorCalibration;
ALTER TABLE SensorCalibration ADD CONSTRAINT composite_index UNIQUE(_parent_oid,serialNumber,channel,start,start_ms);

SELECT 'Updating AuxStream' AS '';
ALTER TABLE AuxStream
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end,
  MODIFY format VARCHAR(50);
ALTER TABLE AuxStream ALTER start_ms DROP DEFAULT;
UPDATE AuxStream SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON AuxStream;
ALTER TABLE AuxStream ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);

SELECT 'Updating Stream' AS '';
ALTER TABLE Stream
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end,
  MODIFY format VARCHAR(50);
ALTER TABLE Stream ALTER start_ms DROP DEFAULT;
UPDATE Stream SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON Stream;
ALTER TABLE Stream ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);

SELECT 'Updating SensorLocation' AS '';
ALTER TABLE SensorLocation
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end;
ALTER TABLE SensorLocation ALTER start_ms DROP DEFAULT;
UPDATE SensorLocation SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON SensorLocation;
ALTER TABLE SensorLocation ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);

SELECT 'Updating Station' AS '';
ALTER TABLE Station
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end,
  MODIFY description VARCHAR(255);
ALTER TABLE Station ALTER start_ms DROP DEFAULT;
UPDATE Station SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON Station;
ALTER TABLE Station ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);

SELECT 'Updating Network' AS '';
ALTER TABLE Network
  ADD start_ms INTEGER NOT NULL DEFAULT '0' AFTER start,
  ADD end_ms INTEGER AFTER end,
  MODIFY description VARCHAR(255);
ALTER TABLE Network ALTER start_ms DROP DEFAULT;
UPDATE Network SET end_ms=0 WHERE end != NULL;
DROP INDEX _parent_oid_2 ON Network;
ALTER TABLE Network ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);

SELECT 'Updating ResponseFIR' AS '';
ALTER TABLE ResponseFIR
  MODIFY gain double,
  ADD gainFrequency DOUBLE UNSIGNED AFTER gain;
DROP INDEX _parent_oid_2 ON ResponseFIR;
ALTER TABLE ResponseFIR ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating ResponsePAZ' AS '';
ALTER TABLE ResponsePAZ
  ADD decimationFactor SMALLINT UNSIGNED,
  ADD delay DOUBLE UNSIGNED,
  ADD correction DOUBLE,
  MODIFY gain DOUBLE;
DROP INDEX _parent_oid_2 ON ResponsePAZ;
ALTER TABLE ResponsePAZ ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating ResponsePolynomial' AS '';
ALTER TABLE ResponsePolynomial
  MODIFY gain DOUBLE,
  MODIFY approximationLowerBound DOUBLE,
  MODIFY approximationUpperBound DOUBLE;
DROP INDEX _parent_oid_2 ON ResponsePolynomial;
ALTER TABLE ResponsePolynomial ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating ResponseFAP' AS '';
ALTER TABLE ResponseFAP MODIFY gain DOUBLE;
DROP INDEX _parent_oid_2 ON ResponseFAP;
ALTER TABLE ResponseFAP ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating Datalogger' AS '';
ALTER TABLE Datalogger
  MODIFY gain DOUBLE,
  MODIFY description VARCHAR(255);
DROP INDEX _parent_oid_2 ON Datalogger;
ALTER TABLE Datalogger ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating StationGroup' AS '';
ALTER TABLE StationGroup
  MODIFY code CHAR(20),
  MODIFY description VARCHAR(255);
DROP INDEX _parent_oid_2 ON StationGroup;
ALTER TABLE StationGroup ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code);

SELECT 'Updating AuxSource' AS '';
ALTER TABLE AuxSource
  MODIFY name VARCHAR(255) NOT NULL,
  MODIFY description VARCHAR(255);
DROP INDEX _parent_oid_2 ON AuxSource;
ALTER TABLE AuxSource ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating AuxDevice' AS '';
ALTER TABLE AuxDevice
  MODIFY name VARCHAR(255) NOT NULL,
  MODIFY description VARCHAR(255);
DROP INDEX _parent_oid_2 ON AuxDevice;
ALTER TABLE AuxDevice ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating Sensor' AS '';
ALTER TABLE Sensor MODIFY name VARCHAR(255) NOT NULL;
DROP INDEX _parent_oid_2 ON Sensor;
ALTER TABLE Sensor ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating CompositeTime' AS '';
ALTER TABLE CompositeTime
  ADD second_pdf_variable_content BLOB AFTER second_confidenceLevel,
  ADD second_pdf_probability_content BLOB AFTER second_pdf_variable_content,
  ADD second_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER second_pdf_probability_content;

SELECT 'Updating MomentTensor' AS '';
ALTER TABLE MomentTensor
  ADD scalarMoment_pdf_variable_content BLOB AFTER scalarMoment_confidenceLevel,
  ADD scalarMoment_pdf_probability_content BLOB AFTER scalarMoment_pdf_variable_content,
  ADD scalarMoment_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER scalarMoment_pdf_probability_content,
  ADD tensor_Mrr_pdf_variable_content BLOB AFTER tensor_Mrr_confidenceLevel,
  ADD tensor_Mrr_pdf_probability_content BLOB AFTER tensor_Mrr_pdf_variable_content,
  ADD tensor_Mrr_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER tensor_Mrr_pdf_probability_content,
  ADD tensor_Mtt_pdf_variable_content BLOB AFTER tensor_Mtt_confidenceLevel,
  ADD tensor_Mtt_pdf_probability_content BLOB AFTER tensor_Mtt_pdf_variable_content,
  ADD tensor_Mtt_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER tensor_Mtt_pdf_probability_content,
  ADD tensor_Mpp_pdf_variable_content BLOB AFTER tensor_Mpp_confidenceLevel,
  ADD tensor_Mpp_pdf_probability_content BLOB AFTER tensor_Mpp_pdf_variable_content,
  ADD tensor_Mpp_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER tensor_Mpp_pdf_probability_content,
  ADD tensor_Mrt_pdf_variable_content BLOB AFTER tensor_Mrt_confidenceLevel,
  ADD tensor_Mrt_pdf_probability_content BLOB AFTER tensor_Mrt_pdf_variable_content,
  ADD tensor_Mrt_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER tensor_Mrt_pdf_probability_content,
  ADD tensor_Mrp_pdf_variable_content BLOB AFTER tensor_Mrp_confidenceLevel,
  ADD tensor_Mrp_pdf_probability_content BLOB AFTER tensor_Mrp_pdf_variable_content,
  ADD tensor_Mrp_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER tensor_Mrp_pdf_probability_content,
  ADD tensor_Mtp_pdf_variable_content BLOB AFTER tensor_Mtp_confidenceLevel,
  ADD tensor_Mtp_pdf_probability_content BLOB AFTER tensor_Mtp_pdf_variable_content,
  ADD tensor_Mtp_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER tensor_Mtp_pdf_probability_content;

SELECT 'Updating FocalMechanism' AS '';
ALTER TABLE FocalMechanism
  ADD nodalPlanes_nodalPlane1_strike_pdf_variable_content BLOB AFTER nodalPlanes_nodalPlane1_strike_confidenceLevel,
  ADD nodalPlanes_nodalPlane1_strike_pdf_probability_content BLOB AFTER nodalPlanes_nodalPlane1_strike_pdf_variable_content,
  ADD nodalPlanes_nodalPlane1_strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER nodalPlanes_nodalPlane1_strike_pdf_probability_content,
  ADD nodalPlanes_nodalPlane1_dip_pdf_variable_content BLOB AFTER nodalPlanes_nodalPlane1_dip_confidenceLevel,
  ADD nodalPlanes_nodalPlane1_dip_pdf_probability_content BLOB AFTER nodalPlanes_nodalPlane1_dip_pdf_variable_content,
  ADD nodalPlanes_nodalPlane1_dip_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER nodalPlanes_nodalPlane1_dip_pdf_probability_content,
  ADD nodalPlanes_nodalPlane1_rake_pdf_variable_content BLOB AFTER nodalPlanes_nodalPlane1_rake_confidenceLevel,
  ADD nodalPlanes_nodalPlane1_rake_pdf_probability_content BLOB AFTER nodalPlanes_nodalPlane1_rake_pdf_variable_content,
  ADD nodalPlanes_nodalPlane1_rake_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER nodalPlanes_nodalPlane1_rake_pdf_probability_content,
  ADD nodalPlanes_nodalPlane2_strike_pdf_variable_content BLOB AFTER nodalPlanes_nodalPlane2_strike_confidenceLevel,
  ADD nodalPlanes_nodalPlane2_strike_pdf_probability_content BLOB AFTER nodalPlanes_nodalPlane2_strike_pdf_variable_content,
  ADD nodalPlanes_nodalPlane2_strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER nodalPlanes_nodalPlane2_strike_pdf_probability_content,
  ADD nodalPlanes_nodalPlane2_dip_pdf_variable_content BLOB AFTER nodalPlanes_nodalPlane2_dip_confidenceLevel,
  ADD nodalPlanes_nodalPlane2_dip_pdf_probability_content BLOB AFTER nodalPlanes_nodalPlane2_dip_pdf_variable_content,
  ADD nodalPlanes_nodalPlane2_dip_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER nodalPlanes_nodalPlane2_dip_pdf_probability_content,
  ADD nodalPlanes_nodalPlane2_rake_pdf_variable_content BLOB AFTER nodalPlanes_nodalPlane2_rake_confidenceLevel,
  ADD nodalPlanes_nodalPlane2_rake_pdf_probability_content BLOB AFTER nodalPlanes_nodalPlane2_rake_pdf_variable_content,
  ADD nodalPlanes_nodalPlane2_rake_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER nodalPlanes_nodalPlane2_rake_pdf_probability_content,
  ADD principalAxes_tAxis_azimuth_pdf_variable_content BLOB AFTER principalAxes_tAxis_azimuth_confidenceLevel,
  ADD principalAxes_tAxis_azimuth_pdf_probability_content BLOB AFTER principalAxes_tAxis_azimuth_pdf_variable_content,
  ADD principalAxes_tAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_tAxis_azimuth_pdf_probability_content,
  ADD principalAxes_tAxis_plunge_pdf_variable_content BLOB AFTER principalAxes_tAxis_plunge_confidenceLevel,
  ADD principalAxes_tAxis_plunge_pdf_probability_content BLOB AFTER principalAxes_tAxis_plunge_pdf_variable_content,
  ADD principalAxes_tAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_tAxis_plunge_pdf_probability_content,
  ADD principalAxes_tAxis_length_pdf_variable_content BLOB AFTER principalAxes_tAxis_length_confidenceLevel,
  ADD principalAxes_tAxis_length_pdf_probability_content BLOB AFTER principalAxes_tAxis_length_pdf_variable_content,
  ADD principalAxes_tAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_tAxis_length_pdf_probability_content,
  ADD principalAxes_pAxis_azimuth_pdf_variable_content BLOB AFTER principalAxes_pAxis_azimuth_confidenceLevel,
  ADD principalAxes_pAxis_azimuth_pdf_probability_content BLOB AFTER principalAxes_pAxis_azimuth_pdf_variable_content,
  ADD principalAxes_pAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_pAxis_azimuth_pdf_probability_content,
  ADD principalAxes_pAxis_plunge_pdf_variable_content BLOB AFTER principalAxes_pAxis_plunge_confidenceLevel,
  ADD principalAxes_pAxis_plunge_pdf_probability_content BLOB AFTER principalAxes_pAxis_plunge_pdf_variable_content,
  ADD principalAxes_pAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_pAxis_plunge_pdf_probability_content,
  ADD principalAxes_pAxis_length_pdf_variable_content BLOB AFTER principalAxes_pAxis_length_confidenceLevel,
  ADD principalAxes_pAxis_length_pdf_probability_content BLOB AFTER principalAxes_pAxis_length_pdf_variable_content,
  ADD principalAxes_pAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_pAxis_length_pdf_probability_content,
  ADD principalAxes_nAxis_azimuth_pdf_variable_content BLOB AFTER principalAxes_nAxis_azimuth_confidenceLevel,
  ADD principalAxes_nAxis_azimuth_pdf_probability_content BLOB AFTER principalAxes_nAxis_azimuth_pdf_variable_content,
  ADD principalAxes_nAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_nAxis_azimuth_pdf_probability_content,
  ADD principalAxes_nAxis_plunge_pdf_variable_content BLOB AFTER principalAxes_nAxis_plunge_confidenceLevel,
  ADD principalAxes_nAxis_plunge_pdf_probability_content BLOB AFTER principalAxes_nAxis_plunge_pdf_variable_content,
  ADD principalAxes_nAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_nAxis_plunge_pdf_probability_content,
  ADD principalAxes_nAxis_length_pdf_variable_content BLOB AFTER principalAxes_nAxis_length_confidenceLevel,
  ADD principalAxes_nAxis_length_pdf_probability_content BLOB AFTER principalAxes_nAxis_length_pdf_variable_content,
  ADD principalAxes_nAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER principalAxes_nAxis_length_pdf_probability_content;

SELECT 'Updating Amplitude' AS '';
ALTER TABLE Amplitude
  ADD amplitude_pdf_variable_content BLOB AFTER amplitude_confidenceLevel,
  ADD amplitude_pdf_probability_content BLOB AFTER amplitude_pdf_variable_content,
  ADD amplitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER amplitude_pdf_probability_content,
  ADD period_pdf_variable_content BLOB AFTER period_confidenceLevel,
  ADD period_pdf_probability_content BLOB AFTER period_pdf_variable_content,
  ADD period_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER period_pdf_probability_content,
  ADD scalingTime_pdf_variable_content BLOB AFTER scalingTime_confidenceLevel,
  ADD scalingTime_pdf_probability_content BLOB AFTER scalingTime_pdf_variable_content,
  ADD scalingTime_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER scalingTime_pdf_probability_content;

SELECT 'Updating Magnitude' AS '';
ALTER TABLE Magnitude
  ADD magnitude_pdf_variable_content BLOB AFTER magnitude_confidenceLevel,
  ADD magnitude_pdf_probability_content BLOB AFTER magnitude_pdf_variable_content,
  ADD magnitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER magnitude_pdf_probability_content;

SELECT 'Updating StationMagnitude' AS '';
ALTER TABLE StationMagnitude
  ADD magnitude_pdf_variable_content BLOB AFTER magnitude_confidenceLevel,
  ADD magnitude_pdf_probability_content BLOB AFTER magnitude_pdf_variable_content,
  ADD magnitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER magnitude_pdf_probability_content;

SELECT 'Updating Pick' AS '';
ALTER TABLE Pick
  ADD time_pdf_variable_content BLOB AFTER time_confidenceLevel,
  ADD time_pdf_probability_content BLOB AFTER time_pdf_variable_content,
  ADD time_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER time_pdf_probability_content,
  ADD horizontalSlowness_pdf_variable_content BLOB AFTER horizontalSlowness_confidenceLevel,
  ADD horizontalSlowness_pdf_probability_content BLOB AFTER horizontalSlowness_pdf_variable_content,
  ADD horizontalSlowness_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER horizontalSlowness_pdf_probability_content,
  ADD backazimuth_pdf_variable_content BLOB AFTER backazimuth_confidenceLevel,
  ADD backazimuth_pdf_probability_content BLOB AFTER backazimuth_pdf_variable_content,
  ADD backazimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER backazimuth_pdf_probability_content;

SELECT 'Updating Origin' AS '';
ALTER TABLE Origin
  ADD time_pdf_variable_content BLOB AFTER time_confidenceLevel,
  ADD time_pdf_probability_content BLOB AFTER time_pdf_variable_content,
  ADD time_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER time_pdf_probability_content,
  ADD latitude_pdf_variable_content BLOB AFTER latitude_confidenceLevel,
  ADD latitude_pdf_probability_content BLOB AFTER latitude_pdf_variable_content,
  ADD latitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER latitude_pdf_probability_content,
  ADD longitude_pdf_variable_content BLOB AFTER longitude_confidenceLevel,
  ADD longitude_pdf_probability_content BLOB AFTER longitude_pdf_variable_content,
  ADD longitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER longitude_pdf_probability_content,
  ADD depth_pdf_variable_content BLOB AFTER depth_confidenceLevel,
  ADD depth_pdf_probability_content BLOB AFTER depth_pdf_variable_content,
  ADD depth_pdf_used TINYINT(1) NOT NULL DEFAULT '0' AFTER depth_pdf_probability_content;

SELECT 'Updating Access' AS '';
DROP INDEX _parent_oid_2 ON Access;
ALTER TABLE Access ADD CONSTRAINT composite_index UNIQUE(_parent_oid,networkCode,stationCode,locationCode,streamCode,user,start);

SELECT 'Updating AmplitudeReference' AS '';
DROP INDEX _parent_oid_2 ON AmplitudeReference;
ALTER TABLE AmplitudeReference ADD CONSTRAINT composite_index UNIQUE(_parent_oid,amplitudeID);

SELECT 'Updating ArclinkRequest' AS '';
DROP INDEX _parent_oid_2 ON ArclinkRequest;
ALTER TABLE ArclinkRequest ADD CONSTRAINT composite_index UNIQUE(_parent_oid,created,created_ms,requestID,userID);

SELECT 'Updating ArclinkRequestLine' AS '';
DROP INDEX _parent_oid_2 ON ArclinkRequestLine;
ALTER TABLE ArclinkRequestLine ADD CONSTRAINT composite_index UNIQUE(_parent_oid,start,start_ms,end,end_ms,streamID_networkCode,streamID_stationCode,streamID_locationCode,streamID_channelCode,streamID_resourceURI);

SELECT 'Updating ArclinkStatusLine' AS '';
DROP INDEX _parent_oid_2 ON ArclinkStatusLine;
ALTER TABLE ArclinkStatusLine ADD CONSTRAINT composite_index UNIQUE(_parent_oid,volumeID,type,status);

SELECT 'Updating ArclinkUser' AS '';
DROP INDEX _parent_oid_2 ON ArclinkUser;
ALTER TABLE ArclinkUser ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name,email);

SELECT 'Updating Arrival' AS '';
DROP INDEX _parent_oid_2 ON Arrival;
ALTER TABLE Arrival ADD CONSTRAINT composite_index UNIQUE(_parent_oid,pickID);

SELECT 'Updating ConfigStation' AS '';
DROP INDEX _parent_oid_2 ON ConfigStation;
ALTER TABLE ConfigStation ADD CONSTRAINT composite_index UNIQUE(_parent_oid,networkCode,stationCode);

SELECT 'Updating Decimation' AS '';
DROP INDEX _parent_oid_2 ON Decimation;
ALTER TABLE Decimation ADD CONSTRAINT composite_index UNIQUE(_parent_oid,sampleRateNumerator,sampleRateDenominator);

SELECT 'Updating EventDescription' AS '';
DROP INDEX _parent_oid_2 ON EventDescription;
ALTER TABLE EventDescription ADD CONSTRAINT composite_index UNIQUE(_parent_oid,type);

SELECT 'Updating FocalMechanismReference' AS '';
DROP INDEX _parent_oid_2 ON FocalMechanismReference;
ALTER TABLE FocalMechanismReference ADD CONSTRAINT composite_index UNIQUE(_parent_oid,focalMechanismID);

SELECT 'Updating MomentTensorComponentContribution' AS '';
DROP INDEX _parent_oid_2 ON MomentTensorComponentContribution;
ALTER TABLE MomentTensorComponentContribution ADD CONSTRAINT composite_index UNIQUE(_parent_oid,phaseCode,component);

SELECT 'Updating MomentTensorPhaseSetting' AS '';
DROP INDEX _parent_oid_2 ON MomentTensorPhaseSetting;
ALTER TABLE MomentTensorPhaseSetting ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code);

SELECT 'Updating OriginReference' AS '';
DROP INDEX _parent_oid_2 ON OriginReference;
ALTER TABLE OriginReference ADD CONSTRAINT composite_index UNIQUE(_parent_oid,originID);

SELECT 'Updating Outage' AS '';
DROP INDEX _parent_oid_2 ON Outage;
ALTER TABLE Outage ADD CONSTRAINT composite_index UNIQUE(_parent_oid,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI,start,start_ms);

SELECT 'Updating PickReference' AS '';
DROP INDEX _parent_oid_2 ON PickReference;
ALTER TABLE PickReference ADD CONSTRAINT composite_index UNIQUE(_parent_oid,pickID);

SELECT 'Updating QCLog' AS '';
DROP INDEX _parent_oid_2 ON QCLog;
ALTER TABLE QCLog ADD CONSTRAINT composite_index UNIQUE(_parent_oid,start,start_ms,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI);

SELECT 'Updating Route' AS '';
DROP INDEX _parent_oid_2 ON Route;
ALTER TABLE Route ADD CONSTRAINT composite_index UNIQUE(_parent_oid,networkCode,stationCode,locationCode,streamCode);

SELECT 'Updating RouteArclink' AS '';
DROP INDEX _parent_oid_2 ON RouteArclink;
ALTER TABLE RouteArclink ADD CONSTRAINT composite_index UNIQUE(_parent_oid,address,start);

SELECT 'Updating RouteSeedlink' AS '';
DROP INDEX _parent_oid_2 ON RouteSeedlink;
ALTER TABLE RouteSeedlink ADD CONSTRAINT composite_index UNIQUE(_parent_oid,address);

SELECT 'Updating Setup' AS '';
DROP INDEX _parent_oid_2 ON Setup;
ALTER TABLE Setup ADD CONSTRAINT composite_index UNIQUE(_parent_oid,name);

SELECT 'Updating StationMagnitudeContribution' AS '';
DROP INDEX _parent_oid_2 ON StationMagnitudeContribution;
ALTER TABLE StationMagnitudeContribution ADD CONSTRAINT composite_index UNIQUE(_parent_oid,stationMagnitudeID);

SELECT 'Updating StationReference' AS '';
DROP INDEX _parent_oid_2 ON StationReference;
ALTER TABLE StationReference ADD CONSTRAINT composite_index UNIQUE(_parent_oid,stationID);

SELECT 'Updating WaveformQuality' AS '';
DROP INDEX _parent_oid_2 ON WaveformQuality;
ALTER TABLE WaveformQuality ADD CONSTRAINT composite_index UNIQUE(_parent_oid,start,start_ms,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI,type,parameter);

# Convert Stream type to type that inherits from PublicObject
SELECT 'Updating PublicObject' AS '';
INSERT INTO PublicObject(_oid, publicID) SELECT _oid, concat("Stream/", DATE_FORMAT(_last_modified, '%Y%m%d%H%i%s'), ".", _oid) FROM Stream;

SELECT 'Updating Meta' AS '';
UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
