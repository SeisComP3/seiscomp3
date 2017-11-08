\echo Creating ResponseIIR
CREATE TABLE ResponseIIR (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_type VARCHAR(1),
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_decimationFactor SMALLINT,
	m_delay DOUBLE PRECISION,
	m_correction DOUBLE PRECISION,
	m_numberOfNumerators SMALLINT,
	m_numberOfDenominators SMALLINT,
	m_numerators_content BYTEA,
	m_numerators_used BOOLEAN NOT NULL DEFAULT '0',
	m_denominators_content BYTEA,
	m_denominators_used BOOLEAN NOT NULL DEFAULT '0',
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT responseiir_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX ResponseIIR__parent_oid ON ResponseIIR(_parent_oid);
CREATE TRIGGER ResponseIIR_update BEFORE UPDATE ON ResponseIIR FOR EACH ROW EXECUTE PROCEDURE update_modified();

\echo Updating Comment
ALTER TABLE Comment
  ADD m_start TIMESTAMP,
  ADD m_start_ms INTEGER,
  ADD m_end TIMESTAMP,
  ADD m_end_ms INTEGER;
ALTER TABLE ONLY Comment ADD CONSTRAINT comment_composite_index UNIQUE (_parent_oid, m_id);
ALTER TABLE ONLY Comment DROP CONSTRAINT comment__parent_oid_m_id_key;

\echo Updating AuxStream
ALTER TABLE AuxStream
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER,
  ALTER m_format TYPE VARCHAR(50);
ALTER TABLE AuxStream ALTER m_start_ms DROP DEFAULT;
UPDATE AuxStream SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY AuxStream ADD CONSTRAINT auxstream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE ONLY AuxStream DROP CONSTRAINT auxstream__parent_oid_m_code_m_start_key;

\echo Updating Stream
ALTER TABLE Stream
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER,
  ALTER m_format TYPE VARCHAR(50);
ALTER TABLE Stream ALTER m_start_ms DROP DEFAULT;
UPDATE Stream SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY Stream ADD CONSTRAINT stream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE ONLY Stream DROP CONSTRAINT stream__parent_oid_m_code_m_start_key;

\echo Updating SensorLocation
ALTER TABLE SensorLocation
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER;
ALTER TABLE SensorLocation ALTER m_start_ms DROP DEFAULT;
UPDATE SensorLocation SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY SensorLocation ADD CONSTRAINT sensorlocation_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE ONLY SensorLocation DROP CONSTRAINT sensorlocation__parent_oid_m_code_m_start_key;

\echo Updating Station
ALTER TABLE Station
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER,
  ALTER m_description TYPE VARCHAR(255);
ALTER TABLE Station ALTER m_start_ms DROP DEFAULT;
ALTER TABLE ONLY Station ADD CONSTRAINT station_composite_index UNIQUE (_parent_oid, m_code, m_start, m_start_ms);
ALTER TABLE ONLY Station DROP CONSTRAINT station__parent_oid_m_code_m_start_key;

\echo Updating DataloggerCalibration
ALTER TABLE DataloggerCalibration
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER;
ALTER TABLE DataloggerCalibration ALTER m_start_ms DROP DEFAULT;
UPDATE DataloggerCalibration SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY DataloggerCalibration ADD CONSTRAINT dataloggercalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms);
ALTER TABLE ONLY DataloggerCalibration DROP CONSTRAINT dataloggercalibration__parent_oid_m_serialnumber_m_channel__key;

\echo Updating SensorCalibration
ALTER TABLE SensorCalibration
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER;
ALTER TABLE SensorCalibration ALTER m_start_ms DROP DEFAULT;
UPDATE SensorCalibration SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY SensorCalibration ADD CONSTRAINT sensorcalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms);
ALTER TABLE ONLY SensorCalibration DROP CONSTRAINT sensorcalibration__parent_oid_m_serialnumber_m_channel_m_st_key;

\echo Updating Network
ALTER TABLE Network
  ADD m_start_ms INTEGER NOT NULL DEFAULT '0',
  ADD m_end_ms INTEGER,
  ALTER m_description TYPE VARCHAR(255);
ALTER TABLE Network ALTER m_start_ms DROP DEFAULT;
UPDATE Network SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY Network ADD CONSTRAINT network_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE ONLY Network DROP CONSTRAINT network__parent_oid_m_code_m_start_key;

\echo Updating ResponseFIR
ALTER TABLE ResponseFIR ADD m_gainFrequency DOUBLE PRECISION;
ALTER TABLE ONLY ResponseFIR ADD CONSTRAINT responsefir_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY ResponseFIR DROP CONSTRAINT responsefir__parent_oid_m_name_key;

\echo Updating ResponsePAZ
ALTER TABLE ResponsePAZ
  ADD m_decimationFactor SMALLINT,
  ADD m_delay DOUBLE PRECISION,
  ADD m_correction DOUBLE PRECISION;
ALTER TABLE ONLY ResponsePAZ ADD CONSTRAINT responsepaz_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY ResponsePAZ DROP CONSTRAINT responsepaz__parent_oid_m_name_key;

\echo Updating StationGroup
ALTER TABLE StationGroup
  ADD m_start_ms INTEGER,
  ADD m_end_ms INTEGER,
  ALTER m_code TYPE VARCHAR(20),
  ALTER m_description TYPE VARCHAR(255);
UPDATE StationGroup SET m_start_ms=0 WHERE m_start != NULL;
UPDATE StationGroup SET m_end_ms=0 WHERE m_end != NULL;
ALTER TABLE ONLY StationGroup ADD CONSTRAINT stationgroup_composite_index UNIQUE (_parent_oid, m_code);
ALTER TABLE ONLY StationGroup DROP CONSTRAINT stationgroup__parent_oid_m_code_key;

\echo Updating AuxSource
ALTER TABLE AuxSource
  ALTER m_name TYPE VARCHAR(255),
  ALTER m_description TYPE VARCHAR(255);
ALTER TABLE ONLY AuxSource ADD CONSTRAINT auxsource_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY AuxSource DROP CONSTRAINT auxsource__parent_oid_m_name_key;

\echo Updating AuxDevice
ALTER TABLE AuxDevice
  ALTER m_name TYPE VARCHAR(255),
  ALTER m_description TYPE VARCHAR(255);
ALTER TABLE ONLY AuxDevice ADD CONSTRAINT auxdevice_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY AuxDevice DROP CONSTRAINT auxdevice__parent_oid_m_name_key;

\echo Updating Sensor
ALTER TABLE Sensor ALTER m_name TYPE VARCHAR(255);
ALTER TABLE ONLY Sensor ADD CONSTRAINT sensor_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY Sensor DROP CONSTRAINT sensor__parent_oid_m_name_key;

\echo Updating Datalogger
ALTER TABLE Datalogger ALTER m_description TYPE VARCHAR(255);
ALTER TABLE ONLY Datalogger ADD CONSTRAINT datalogger_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY Datalogger DROP CONSTRAINT datalogger__parent_oid_m_name_key;

\echo Updating CompositeTime
ALTER TABLE CompositeTime
  ADD m_second_pdf_variable_content BYTEA,
  ADD m_second_pdf_probability_content BYTEA,
  ADD m_second_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating MomentTensor
ALTER TABLE MomentTensor
  ADD m_scalarMoment_pdf_variable_content BYTEA,
  ADD m_scalarMoment_pdf_probability_content BYTEA,
  ADD m_scalarMoment_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_tensor_Mrr_pdf_variable_content BYTEA,
  ADD m_tensor_Mrr_pdf_probability_content BYTEA,
  ADD m_tensor_Mrr_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_tensor_Mtt_pdf_variable_content BYTEA,
  ADD m_tensor_Mtt_pdf_probability_content BYTEA,
  ADD m_tensor_Mtt_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_tensor_Mpp_pdf_variable_content BYTEA,
  ADD m_tensor_Mpp_pdf_probability_content BYTEA,
  ADD m_tensor_Mpp_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_tensor_Mrt_pdf_variable_content BYTEA,
  ADD m_tensor_Mrt_pdf_probability_content BYTEA,
  ADD m_tensor_Mrt_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_tensor_Mrp_pdf_variable_content BYTEA,
  ADD m_tensor_Mrp_pdf_probability_content BYTEA,
  ADD m_tensor_Mrp_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_tensor_Mtp_pdf_variable_content BYTEA,
  ADD m_tensor_Mtp_pdf_probability_content BYTEA,
  ADD m_tensor_Mtp_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating FocalMechanism
ALTER TABLE FocalMechanism
  ADD m_nodalPlanes_nodalPlane1_strike_pdf_variable_content BYTEA,
  ADD m_nodalPlanes_nodalPlane1_strike_pdf_probability_content BYTEA,
  ADD m_nodalPlanes_nodalPlane1_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_nodalPlanes_nodalPlane1_dip_pdf_variable_content BYTEA,
  ADD m_nodalPlanes_nodalPlane1_dip_pdf_probability_content BYTEA,
  ADD m_nodalPlanes_nodalPlane1_dip_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_nodalPlanes_nodalPlane1_rake_pdf_variable_content BYTEA,
  ADD m_nodalPlanes_nodalPlane1_rake_pdf_probability_content BYTEA,
  ADD m_nodalPlanes_nodalPlane1_rake_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_nodalPlanes_nodalPlane2_strike_pdf_variable_content BYTEA,
  ADD m_nodalPlanes_nodalPlane2_strike_pdf_probability_content BYTEA,
  ADD m_nodalPlanes_nodalPlane2_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_nodalPlanes_nodalPlane2_dip_pdf_variable_content BYTEA,
  ADD m_nodalPlanes_nodalPlane2_dip_pdf_probability_content BYTEA,
  ADD m_nodalPlanes_nodalPlane2_dip_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_nodalPlanes_nodalPlane2_rake_pdf_variable_content BYTEA,
  ADD m_nodalPlanes_nodalPlane2_rake_pdf_probability_content BYTEA,
  ADD m_nodalPlanes_nodalPlane2_rake_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_tAxis_azimuth_pdf_variable_content BYTEA,
  ADD m_principalAxes_tAxis_azimuth_pdf_probability_content BYTEA,
  ADD m_principalAxes_tAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_tAxis_plunge_pdf_variable_content BYTEA,
  ADD m_principalAxes_tAxis_plunge_pdf_probability_content BYTEA,
  ADD m_principalAxes_tAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_tAxis_length_pdf_variable_content BYTEA,
  ADD m_principalAxes_tAxis_length_pdf_probability_content BYTEA,
  ADD m_principalAxes_tAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_pAxis_azimuth_pdf_variable_content BYTEA,
  ADD m_principalAxes_pAxis_azimuth_pdf_probability_content BYTEA,
  ADD m_principalAxes_pAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_pAxis_plunge_pdf_variable_content BYTEA,
  ADD m_principalAxes_pAxis_plunge_pdf_probability_content BYTEA,
  ADD m_principalAxes_pAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_pAxis_length_pdf_variable_content BYTEA,
  ADD m_principalAxes_pAxis_length_pdf_probability_content BYTEA,
  ADD m_principalAxes_pAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_nAxis_azimuth_pdf_variable_content BYTEA,
  ADD m_principalAxes_nAxis_azimuth_pdf_probability_content BYTEA,
  ADD m_principalAxes_nAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_nAxis_plunge_pdf_variable_content BYTEA,
  ADD m_principalAxes_nAxis_plunge_pdf_probability_content BYTEA,
  ADD m_principalAxes_nAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_principalAxes_nAxis_length_pdf_variable_content BYTEA,
  ADD m_principalAxes_nAxis_length_pdf_probability_content BYTEA,
  ADD m_principalAxes_nAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Amplitude
ALTER TABLE Amplitude
  ADD m_amplitude_pdf_variable_content BYTEA,
  ADD m_amplitude_pdf_probability_content BYTEA,
  ADD m_amplitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_period_pdf_variable_content BYTEA,
  ADD m_period_pdf_probability_content BYTEA,
  ADD m_period_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_scalingTime_pdf_variable_content BYTEA,
  ADD m_scalingTime_pdf_probability_content BYTEA,
  ADD m_scalingTime_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Magnitude
ALTER TABLE Magnitude
  ADD m_magnitude_pdf_variable_content BYTEA,
  ADD m_magnitude_pdf_probability_content BYTEA,
  ADD m_magnitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating StationMagnitude
ALTER TABLE StationMagnitude
  ADD m_magnitude_pdf_variable_content BYTEA,
  ADD m_magnitude_pdf_probability_content BYTEA,
  ADD m_magnitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Pick
ALTER TABLE Pick
  ADD m_time_pdf_variable_content BYTEA,
  ADD m_time_pdf_probability_content BYTEA,
  ADD m_time_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_horizontalSlowness_pdf_variable_content BYTEA,
  ADD m_horizontalSlowness_pdf_probability_content BYTEA,
  ADD m_horizontalSlowness_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_backazimuth_pdf_variable_content BYTEA,
  ADD m_backazimuth_pdf_probability_content BYTEA,
  ADD m_backazimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Origin
ALTER TABLE Origin
  ADD m_time_pdf_variable_content BYTEA,
  ADD m_time_pdf_probability_content BYTEA,
  ADD m_time_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_latitude_pdf_variable_content BYTEA,
  ADD m_latitude_pdf_probability_content BYTEA,
  ADD m_latitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_longitude_pdf_variable_content BYTEA,
  ADD m_longitude_pdf_probability_content BYTEA,
  ADD m_longitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_depth_pdf_variable_content BYTEA,
  ADD m_depth_pdf_probability_content BYTEA,
  ADD m_depth_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Access
ALTER TABLE ONLY Access ADD CONSTRAINT access_composite_index UNIQUE (_parent_oid, m_networkcode, m_stationcode, m_locationcode, m_streamcode, m_user, m_start);
ALTER TABLE ONLY Access DROP CONSTRAINT access__parent_oid_m_networkcode_m_stationcode_m_locationco_key;

\echo Updating AmplitudeReference
ALTER TABLE ONLY AmplitudeReference ADD CONSTRAINT amplitudereference_composite_index UNIQUE (_parent_oid, m_amplitudeid);
ALTER TABLE ONLY AmplitudeReference DROP CONSTRAINT amplitudereference__parent_oid_m_amplitudeid_key;

\echo Updating ArclinkRequest
ALTER TABLE ONLY ArclinkRequest ADD CONSTRAINT arclinkrequest_composite_index UNIQUE (_parent_oid, m_created, m_created_ms, m_requestid, m_userid);
ALTER TABLE ONLY ArclinkRequest DROP CONSTRAINT arclinkrequest__parent_oid_m_created_m_created_ms_m_request_key;

\echo Updating ArclinkRequestLine
ALTER TABLE ONLY ArclinkRequestLine ADD CONSTRAINT arclinkrequestline_composite_index UNIQUE (_parent_oid, m_start, m_start_ms, m_end, m_end_ms, m_streamid_networkcode, m_streamid_stationcode, m_streamid_locationcode, m_streamid_channelcode, m_streamid_resourceuri);
ALTER TABLE ONLY ArclinkRequestLine DROP CONSTRAINT arclinkrequestline__parent_oid_m_start_m_start_ms_m_end_m_e_key;

\echo Updating ArclinkStatusLine
ALTER TABLE ONLY ArclinkStatusLine ADD CONSTRAINT arclinkstatusline_composite_index UNIQUE (_parent_oid, m_volumeid, m_type, m_status);
ALTER TABLE ONLY ArclinkStatusLine DROP CONSTRAINT arclinkstatusline__parent_oid_m_volumeid_m_type_m_status_key;

\echo Updating ArclinkUser
ALTER TABLE ONLY ArclinkUser ADD CONSTRAINT arclinkuser_composite_index UNIQUE (_parent_oid, m_name, m_email);
ALTER TABLE ONLY ArclinkUser DROP CONSTRAINT arclinkuser__parent_oid_m_name_m_email_key;

\echo Updating Arrival
ALTER TABLE ONLY Arrival ADD CONSTRAINT arrival_composite_index UNIQUE (_parent_oid, m_pickid);
ALTER TABLE ONLY Arrival DROP CONSTRAINT arrival__parent_oid_m_pickid_key;

\echo Updating ConfigStation
ALTER TABLE ONLY ConfigStation ADD CONSTRAINT configstation_composite_index UNIQUE (_parent_oid, m_networkcode, m_stationcode);
ALTER TABLE ONLY ConfigStation DROP CONSTRAINT configstation__parent_oid_m_networkcode_m_stationcode_key;

\echo Updating Decimation
ALTER TABLE ONLY Decimation ADD CONSTRAINT decimation_composite_index UNIQUE (_parent_oid, m_sampleratenumerator, m_sampleratedenominator);
ALTER TABLE ONLY Decimation DROP CONSTRAINT decimation__parent_oid_m_sampleratenumerator_m_sampleratede_key;

\echo Updating EventDescription
ALTER TABLE ONLY EventDescription ADD CONSTRAINT eventdescription_composite_index UNIQUE (_parent_oid, m_type);
ALTER TABLE ONLY EventDescription DROP CONSTRAINT eventdescription__parent_oid_m_type_key;

\echo Updating FocalMechanismReference
ALTER TABLE ONLY FocalMechanismReference ADD CONSTRAINT focalmechanismreference_composite_index UNIQUE (_parent_oid, m_focalmechanismid);
ALTER TABLE ONLY FocalMechanismReference DROP CONSTRAINT focalmechanismreference__parent_oid_m_focalmechanismid_key;

\echo Updating MomentTensorComponentContribution
ALTER TABLE ONLY MomentTensorComponentContribution ADD CONSTRAINT momenttensorcomponentcontribution_composite_index UNIQUE (_parent_oid, m_phasecode, m_component);
ALTER TABLE ONLY MomentTensorComponentContribution DROP CONSTRAINT momenttensorcomponentcontribu__parent_oid_m_phasecode_m_com_key;

\echo Updating MomentTensorPhaseSetting
ALTER TABLE ONLY MomentTensorPhaseSetting ADD CONSTRAINT momenttensorphasesetting_composite_index UNIQUE (_parent_oid, m_code);
ALTER TABLE ONLY MomentTensorPhaseSetting DROP CONSTRAINT momenttensorphasesetting__parent_oid_m_code_key;

\echo Updating OriginReference
ALTER TABLE ONLY OriginReference ADD CONSTRAINT originreference_composite_index UNIQUE (_parent_oid, m_originid);
ALTER TABLE ONLY OriginReference DROP CONSTRAINT originreference__parent_oid_m_originid_key;

\echo Updating Outage
ALTER TABLE ONLY Outage ADD CONSTRAINT outage_composite_index UNIQUE (_parent_oid, m_waveformid_networkcode, m_waveformid_stationcode, m_waveformid_locationcode, m_waveformid_channelcode, m_waveformid_resourceuri, m_start, m_start_ms);
ALTER TABLE ONLY Outage DROP CONSTRAINT outage__parent_oid_m_waveformid_networkcode_m_waveformid_st_key;

\echo Updating PickReference
ALTER TABLE ONLY PickReference ADD CONSTRAINT pickreference_composite_index UNIQUE (_parent_oid, m_pickid);
ALTER TABLE ONLY PickReference DROP CONSTRAINT pickreference__parent_oid_m_pickid_key;

\echo Updating QCLog
ALTER TABLE ONLY QCLog ADD CONSTRAINT qclog_composite_index UNIQUE (_parent_oid, m_start, m_start_ms, m_waveformid_networkcode, m_waveformid_stationcode, m_waveformid_locationcode, m_waveformid_channelcode, m_waveformid_resourceuri);
ALTER TABLE ONLY QCLog DROP CONSTRAINT qclog__parent_oid_m_start_m_start_ms_m_waveformid_networkco_key;

\echo Updating ResponseFAP
ALTER TABLE ONLY ResponseFAP ADD CONSTRAINT responsefap_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY ResponseFAP DROP CONSTRAINT responsefap__parent_oid_m_name_key;

\echo Updating ResponsePolynomial
ALTER TABLE ONLY ResponsePolynomial ADD CONSTRAINT responsepolynomial_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY ResponsePolynomial DROP CONSTRAINT responsepolynomial__parent_oid_m_name_key;

\echo Updating Route
ALTER TABLE ONLY Route ADD CONSTRAINT route_composite_index UNIQUE (_parent_oid, m_networkcode, m_stationcode, m_locationcode, m_streamcode);
ALTER TABLE ONLY Route DROP CONSTRAINT route__parent_oid_m_networkcode_m_stationcode_m_locationcod_key;

\echo Updating RouteArclink
ALTER TABLE ONLY RouteArclink ADD CONSTRAINT routearclink_composite_index UNIQUE (_parent_oid, m_address, m_start);
ALTER TABLE ONLY RouteArclink DROP CONSTRAINT routearclink__parent_oid_m_address_m_start_key;

\echo Updating RouteSeedlink
ALTER TABLE ONLY RouteSeedlink ADD CONSTRAINT routeseedlink_composite_index UNIQUE (_parent_oid, m_address);
ALTER TABLE ONLY RouteSeedlink DROP CONSTRAINT routeseedlink__parent_oid_m_address_key;

\echo Updating Setup
ALTER TABLE ONLY Setup ADD CONSTRAINT setup_composite_index UNIQUE (_parent_oid, m_name);
ALTER TABLE ONLY Setup DROP CONSTRAINT setup__parent_oid_m_name_key;

\echo Updating StationMagnitudeContribution
ALTER TABLE ONLY StationMagnitudeContribution ADD CONSTRAINT stationmagnitudecontribution_composite_index UNIQUE (_parent_oid, m_stationmagnitudeid);
ALTER TABLE ONLY StationMagnitudeContribution DROP CONSTRAINT stationmagnitudecontribution__parent_oid_m_stationmagnitude_key;

\echo Updating StationReference
ALTER TABLE ONLY StationReference ADD CONSTRAINT stationreference_composite_index UNIQUE (_parent_oid, m_stationid);
ALTER TABLE ONLY StationReference DROP CONSTRAINT stationreference__parent_oid_m_stationid_key;

\echo Updating WaveformQuality
ALTER TABLE ONLY WaveformQuality ADD CONSTRAINT waveformquality_composite_index UNIQUE (_parent_oid, m_start, m_start_ms, m_waveformid_networkcode, m_waveformid_stationcode, m_waveformid_locationcode, m_waveformid_channelcode, m_waveformid_resourceuri, m_type, m_parameter);
ALTER TABLE ONLY WaveformQuality DROP CONSTRAINT waveformquality__parent_oid_m_start_m_start_ms_m_waveformid_key;

/* Convert Stream type to type that inherits from PublicObject */
\echo Updating PublicObject
INSERT INTO PublicObject(_oid, m_publicID) SELECT _oid, 'Stream/' || to_char(_last_modified, 'YYYYmmddHHMISS') || '.' || _oid FROM Stream;

\echo Updating Meta
UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
