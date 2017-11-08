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

ALTER TABLE Comment ADD m_start TIMESTAMP;
ALTER TABLE Comment ADD m_start_ms INTEGER;
ALTER TABLE Comment ADD m_end TIMESTAMP;
ALTER TABLE Comment ADD m_end_ms INTEGER;

ALTER TABLE StationGroup ADD m_start_ms INTEGER;
UPDATE StationGroup SET m_start_ms=0;
ALTER TABLE StationGroup ADD m_end_ms INTEGER;
UPDATE StationGroup SET m_end_ms=0;
ALTER TABLE DataloggerCalibration ADD m_start_ms INTEGER;
UPDATE DataloggerCalibration SET m_start_ms=0;
ALTER TABLE DataloggerCalibration ADD m_end_ms INTEGER;
UPDATE DataloggerCalibration SET m_end_ms=0;
ALTER TABLE DataloggerCalibration DROP CONSTRAINT dataloggercalibration__parent_oid_m_serialnumber_m_channel__key;
ALTER TABLE DataloggerCalibration ADD CONSTRAINT dataloggercalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms);
ALTER TABLE SensorCalibration ADD m_start_ms INTEGER;
UPDATE SensorCalibration SET m_start_ms=0;
ALTER TABLE SensorCalibration ADD m_end_ms INTEGER;
UPDATE SensorCalibration SET m_end_ms=0;
ALTER TABLE SensorCalibration DROP CONSTRAINT sensorcalibration__parent_oid_m_serialnumber_m_channel_m_st_key;
ALTER TABLE SensorCalibration ADD CONSTRAINT sensorcalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms);
ALTER TABLE AuxStream ADD m_start_ms INTEGER;
UPDATE AuxStream SET m_start_ms=0;
ALTER TABLE AuxStream ADD m_end_ms INTEGER;
UPDATE AuxStream SET m_end_ms=0;
ALTER TABLE AuxStream DROP CONSTRAINT auxstream__parent_oid_m_code_m_start_key;
ALTER TABLE AuxStream ADD CONSTRAINT auxstream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE Stream ADD m_start_ms INTEGER;
UPDATE Stream SET m_start_ms=0;
ALTER TABLE Stream ADD m_end_ms INTEGER;
UPDATE Stream SET m_end_ms=0;
ALTER TABLE Stream DROP CONSTRAINT stream__parent_oid_m_code_m_start_key;
ALTER TABLE Stream ADD CONSTRAINT stream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE SensorLocation ADD m_start_ms INTEGER;
UPDATE SensorLocation SET m_start_ms=0;
ALTER TABLE SensorLocation ADD m_end_ms INTEGER;
UPDATE SensorLocation SET m_end_ms=0;
ALTER TABLE SensorLocation DROP CONSTRAINT sensorlocation__parent_oid_m_code_m_start_key;
ALTER TABLE SensorLocation ADD CONSTRAINT sensorlocation_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE Station ADD m_start_ms INTEGER;
UPDATE Station SET m_start_ms=0;
ALTER TABLE Station ADD m_end_ms INTEGER;
UPDATE Station SET m_end_ms=0;
ALTER TABLE Station DROP CONSTRAINT station__parent_oid_m_code_m_start_key;
ALTER TABLE Station ADD CONSTRAINT station_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE Network ADD m_start_ms INTEGER;
UPDATE Network SET m_start_ms=0;
ALTER TABLE Network ADD m_end_ms INTEGER;
UPDATE Network SET m_end_ms=0;
ALTER TABLE Network DROP CONSTRAINT network__parent_oid_m_code_m_start_key;
ALTER TABLE Network ADD CONSTRAINT network_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE ResponseFIR ADD m_gainFrequency DOUBLE PRECISION;
ALTER TABLE ResponsePAZ ADD m_decimationFactor SMALLINT;
ALTER TABLE ResponsePAZ ADD m_delay DOUBLE PRECISION;
ALTER TABLE ResponsePAZ ADD m_correction DOUBLE PRECISION;
ALTER TABLE StationGroup ALTER COLUMN m_code TYPE VARCHAR(20);
ALTER TABLE StationGroup ALTER COLUMN m_description TYPE VARCHAR(255);
ALTER TABLE AuxSource ALTER COLUMN m_name TYPE VARCHAR(255);
ALTER TABLE AuxSource ALTER COLUMN m_description TYPE VARCHAR(255);
ALTER TABLE AuxDevice ALTER COLUMN m_name TYPE VARCHAR(255);
ALTER TABLE AuxDevice ALTER COLUMN m_description TYPE VARCHAR(255);
ALTER TABLE Sensor ALTER COLUMN m_name TYPE VARCHAR(255);
ALTER TABLE Datalogger ALTER COLUMN m_description TYPE VARCHAR(255);
ALTER TABLE Station ALTER COLUMN m_description TYPE VARCHAR(255);
ALTER TABLE Network ALTER COLUMN m_description TYPE VARCHAR(255);

ALTER TABLE CompositeTime ADD m_second_pdf_variable_content BYTEA;
ALTER TABLE CompositeTime ADD m_second_pdf_probability_content BYTEA;
ALTER TABLE CompositeTime ADD m_second_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE MomentTensor ADD m_scalarMoment_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_scalarMoment_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_scalarMoment_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD m_tensor_Mrr_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mrr_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mrr_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD m_tensor_Mtt_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mtt_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mtt_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD m_tensor_Mpp_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mpp_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mpp_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD m_tensor_Mrt_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mrt_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mrt_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD m_tensor_Mrp_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mrp_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mrp_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD m_tensor_Mtp_pdf_variable_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mtp_pdf_probability_content BYTEA;
ALTER TABLE MomentTensor ADD m_tensor_Mtp_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_strike_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_strike_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_dip_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_dip_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_dip_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_rake_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_rake_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane1_rake_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_strike_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_strike_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_dip_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_dip_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_dip_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_rake_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_rake_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_nodalPlanes_nodalPlane2_rake_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_azimuth_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_azimuth_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_plunge_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_plunge_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_length_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_length_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_tAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_azimuth_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_azimuth_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_plunge_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_plunge_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_length_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_length_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_pAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_azimuth_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_azimuth_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_plunge_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_plunge_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_length_pdf_variable_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_length_pdf_probability_content BYTEA;
ALTER TABLE FocalMechanism ADD m_principalAxes_nAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE Amplitude ADD m_amplitude_pdf_variable_content BYTEA;
ALTER TABLE Amplitude ADD m_amplitude_pdf_probability_content BYTEA;
ALTER TABLE Amplitude ADD m_amplitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Amplitude ADD m_period_pdf_variable_content BYTEA;
ALTER TABLE Amplitude ADD m_period_pdf_probability_content BYTEA;
ALTER TABLE Amplitude ADD m_period_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Amplitude ADD m_scalingTime_pdf_variable_content BYTEA;
ALTER TABLE Amplitude ADD m_scalingTime_pdf_probability_content BYTEA;
ALTER TABLE Amplitude ADD m_scalingTime_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE Magnitude ADD m_magnitude_pdf_variable_content BYTEA;
ALTER TABLE Magnitude ADD m_magnitude_pdf_probability_content BYTEA;
ALTER TABLE Magnitude ADD m_magnitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE StationMagnitude ADD m_magnitude_pdf_variable_content BYTEA;
ALTER TABLE StationMagnitude ADD m_magnitude_pdf_probability_content BYTEA;
ALTER TABLE StationMagnitude ADD m_magnitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE Pick ADD m_time_pdf_variable_content BYTEA;
ALTER TABLE Pick ADD m_time_pdf_probability_content BYTEA;
ALTER TABLE Pick ADD m_time_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Pick ADD m_horizontalSlowness_pdf_variable_content BYTEA;
ALTER TABLE Pick ADD m_horizontalSlowness_pdf_probability_content BYTEA;
ALTER TABLE Pick ADD m_horizontalSlowness_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Pick ADD m_backazimuth_pdf_variable_content BYTEA;
ALTER TABLE Pick ADD m_backazimuth_pdf_probability_content BYTEA;
ALTER TABLE Pick ADD m_backazimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0';

ALTER TABLE Origin ADD m_time_pdf_variable_content BYTEA;
ALTER TABLE Origin ADD m_time_pdf_probability_content BYTEA;
ALTER TABLE Origin ADD m_time_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Origin ADD m_latitude_pdf_variable_content BYTEA;
ALTER TABLE Origin ADD m_latitude_pdf_probability_content BYTEA;
ALTER TABLE Origin ADD m_latitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Origin ADD m_longitude_pdf_variable_content BYTEA;
ALTER TABLE Origin ADD m_longitude_pdf_probability_content BYTEA;
ALTER TABLE Origin ADD m_longitude_pdf_used BOOLEAN NOT NULL DEFAULT '0';
ALTER TABLE Origin ADD m_depth_pdf_variable_content BYTEA;
ALTER TABLE Origin ADD m_depth_pdf_probability_content BYTEA;
ALTER TABLE Origin ADD m_depth_pdf_used BOOLEAN NOT NULL DEFAULT '0';

/* Convert Stream type to type that inherits from PublicObject */
INSERT INTO PublicObject(_oid, m_publicID) SELECT _oid, 'Stream/' || to_char(_last_modified, 'YYYYmmddHHMISS') || '.' || _oid FROM Stream;

UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
