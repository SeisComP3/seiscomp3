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

ALTER TABLE StationGroup ADD m_start_ms INTEGER;
ALTER TABLE StationGroup ADD m_end_ms INTEGER;
ALTER TABLE DataloggerCalibration ADD m_start_ms INTEGER;
ALTER TABLE DataloggerCalibration ADD m_end_ms INTEGER;
ALTER TABLE DataloggerCalibration DROP CONSTRAINT dataloggercalibration__parent_oid_m_serialnumber_m_channel__key;
ALTER TABLE DataloggerCalibration ADD CONSTRAINT dataloggercalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms);
ALTER TABLE SensorCalibration ADD m_start_ms INTEGER;
ALTER TABLE SensorCalibration ADD m_end_ms INTEGER;
ALTER TABLE SensorCalibration DROP CONSTRAINT sensorcalibration__parent_oid_m_serialnumber_m_channel_m_st_key;
ALTER TABLE SensorCalibration ADD CONSTRAINT sensorcalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms);
ALTER TABLE AuxStream ADD m_start_ms INTEGER;
ALTER TABLE AuxStream ADD m_end_ms INTEGER;
ALTER TABLE AuxStream DROP CONSTRAINT auxstream__parent_oid_m_code_m_start_key;
ALTER TABLE AuxStream ADD CONSTRAINT auxstream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE Stream ADD m_start_ms INTEGER;
ALTER TABLE Stream ADD m_end_ms INTEGER;
ALTER TABLE Stream DROP CONSTRAINT stream__parent_oid_m_code_m_start_key;
ALTER TABLE Stream ADD CONSTRAINT stream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE SensorLocation ADD m_start_ms INTEGER;
ALTER TABLE SensorLocation ADD m_end_ms INTEGER;
ALTER TABLE SensorLocation DROP CONSTRAINT sensorlocation__parent_oid_m_code_m_start_key;
ALTER TABLE SensorLocation ADD CONSTRAINT sensorlocation_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE Station ADD m_start_ms INTEGER;
ALTER TABLE Station ADD m_end_ms INTEGER;
ALTER TABLE Station DROP CONSTRAINT station__parent_oid_m_code_m_start_key;
ALTER TABLE Station ADD CONSTRAINT station_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE Network ADD m_start_ms INTEGER;
ALTER TABLE Network ADD m_end_ms INTEGER;
ALTER TABLE Network DROP CONSTRAINT network__parent_oid_m_code_m_start_key;
ALTER TABLE Network ADD CONSTRAINT network_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms);
ALTER TABLE ResponseFIR ADD m_gainFrequency DOUBLE PRECISION;
ALTER TABLE ResponsePAZ ADD m_decimationFactor SMALLINT UNSIGNED;
ALTER TABLE ResponsePAZ ADD m_delay DOUBLE UNSIGNED;
ALTER TABLE ResponsePAZ ADD m_correction DOUBLE;
ALTER TABLE StationGroup MODIFY m_code VARCHAR(20);
ALTER TABLE StationGroup MODIFY m_description VARCHAR(255);
ALTER TABLE AuxSource MODIFY m_name VARCHAR(255);
ALTER TABLE AuxSource MODIFY m_description VARCHAR(255);
ALTER TABLE AuxDevice MODIFY m_name VARCHAR(255);
ALTER TABLE AuxDevice MODIFY m_description VARCHAR(255);
ALTER TABLE Sensor MODIFY m_name VARCHAR(255);
ALTER TABLE Datalogger MODIFY m_description VARCHAR(255);
ALTER TABLE Station MODIFY m_description VARCHAR(255);
ALTER TABLE Network MODIFY m_description VARCHAR(255);

UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
