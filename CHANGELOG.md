# Jakarta

## Release YYYY.DDD

The database schema has changed since the previous version. To upgrade your
database from version 0.9 to 0.10 to following SQL script can be used:


**MYSQL**

```sql
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

ALTER TABLE Comment ADD start DATETIME AFTER id;
ALTER TABLE Comment ADD start_ms INTEGER AFTER start;
ALTER TABLE Comment ADD end DATETIME AFTER start_ms;
ALTER TABLE Comment ADD end_ms INTEGER AFTER end;

ALTER TABLE StationGroup ADD start_ms INTEGER AFTER start;
ALTER TABLE StationGroup ADD end_ms INTEGER AFTER end;
ALTER TABLE DataloggerCalibration ADD start_ms INTEGER AFTER start;
ALTER TABLE DataloggerCalibration ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON DataloggerCalibration;
ALTER TABLE DataloggerCalibration ADD CONSTRAINT composite_index UNIQUE(_parent_oid,serialNumber,channel,start,start_ms);
ALTER TABLE SensorCalibration ADD start_ms INTEGER AFTER start;
ALTER TABLE SensorCalibration ADD end_ms INTEGER AFTER end;
ALTER TABLE SensorCalibration MODIFY gain DOUBLE;
DROP INDEX _parent_oid_2 ON SensorCalibration;
ALTER TABLE SensorCalibration ADD CONSTRAINT composite_index UNIQUE(_parent_oid,serialNumber,channel,start,start_ms);
ALTER TABLE AuxStream ADD start_ms INTEGER AFTER start;
ALTER TABLE AuxStream ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON AuxStream;
ALTER TABLE AuxStream ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE Stream ADD start_ms INTEGER AFTER start;
ALTER TABLE Stream ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON Stream;
ALTER TABLE Stream ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE SensorLocation ADD start_ms INTEGER AFTER start;
ALTER TABLE SensorLocation ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON SensorLocation;
ALTER TABLE SensorLocation ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE Station ADD start_ms INTEGER AFTER start;
ALTER TABLE Station ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON Station;
ALTER TABLE Station ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE Network ADD start_ms INTEGER AFTER start;
ALTER TABLE Network ADD end_ms INTEGER AFTER end;
DROP INDEX _parent_oid_2 ON Network;
ALTER TABLE Network ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE ResponseFIR ADD gainFrequency DOUBLE UNSIGNED AFTER gain;
ALTER TABLE ResponsePAZ ADD decimationFactor SMALLINT UNSIGNED;
ALTER TABLE ResponsePAZ ADD delay DOUBLE UNSIGNED;
ALTER TABLE ResponsePAZ ADD correction DOUBLE;
ALTER TABLE ResponsePAZ MODIFY gain DOUBLE;
ALTER TABLE ResponsePolynomial MODIFY gain DOUBLE;
ALTER TABLE ResponsePolynomial MODIFY approximationLowerBound DOUBLE;
ALTER TABLE ResponsePolynomial MODIFY approximationUpperBound DOUBLE;
ALTER TABLE ResponseFAP MODIFY gain DOUBLE;
ALTER TABLE DataloggerCalibration MODIFY gain DOUBLE;
ALTER TABLE Datalogger MODIFY gain DOUBLE;
ALTER TABLE StationGroup MODIFY code CHAR(20);
ALTER TABLE StationGroup MODIFY description VARCHAR(255);
ALTER TABLE AuxSource MODIFY name VARCHAR(255);
ALTER TABLE AuxSource MODIFY description VARCHAR(255);
ALTER TABLE AuxDevice MODIFY name VARCHAR(255);
ALTER TABLE AuxDevice MODIFY description VARCHAR(255);
ALTER TABLE Sensor MODIFY name VARCHAR(255);
ALTER TABLE Datalogger MODIFY description VARCHAR(255);
ALTER TABLE Station MODIFY description VARCHAR(255);
ALTER TABLE Network MODIFY description VARCHAR(255);

ALTER TABLE CompositeTime ADD second_pdf_variable_content BLOB;
ALTER TABLE CompositeTime ADD second_pdf_probability_content BLOB;
ALTER TABLE CompositeTime ADD second_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE MomentTensor ADD scalarMoment_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD scalarMoment_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD scalarMoment_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD tensor_Mrr_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mrr_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mrr_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD tensor_Mtt_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mtt_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mtt_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD tensor_Mpp_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mpp_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mpp_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD tensor_Mrt_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mrt_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mrt_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD tensor_Mrp_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mrp_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mrp_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE MomentTensor ADD tensor_Mtp_pdf_variable_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mtp_pdf_probability_content BLOB;
ALTER TABLE MomentTensor ADD tensor_Mtp_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_strike_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_strike_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_dip_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_dip_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_dip_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_rake_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_rake_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane1_rake_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_strike_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_strike_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_dip_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_dip_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_dip_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_rake_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_rake_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD nodalPlanes_nodalPlane2_rake_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_azimuth_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_azimuth_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_plunge_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_plunge_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_length_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_length_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_tAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_azimuth_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_azimuth_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_plunge_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_plunge_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_length_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_length_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_pAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_azimuth_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_azimuth_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_plunge_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_plunge_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_length_pdf_variable_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_length_pdf_probability_content BLOB;
ALTER TABLE FocalMechanism ADD principalAxes_nAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE Amplitude ADD amplitude_pdf_variable_content BLOB;
ALTER TABLE Amplitude ADD amplitude_pdf_probability_content BLOB;
ALTER TABLE Amplitude ADD amplitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Amplitude ADD period_pdf_variable_content BLOB;
ALTER TABLE Amplitude ADD period_pdf_probability_content BLOB;
ALTER TABLE Amplitude ADD period_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Amplitude ADD scalingTime_pdf_variable_content BLOB;
ALTER TABLE Amplitude ADD scalingTime_pdf_probability_content BLOB;
ALTER TABLE Amplitude ADD scalingTime_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE Magnitude ADD magnitude_pdf_variable_content BLOB;
ALTER TABLE Magnitude ADD magnitude_pdf_probability_content BLOB;
ALTER TABLE Magnitude ADD magnitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE StationMagnitude ADD magnitude_pdf_variable_content BLOB;
ALTER TABLE StationMagnitude ADD magnitude_pdf_probability_content BLOB;
ALTER TABLE StationMagnitude ADD magnitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE Pick ADD time_pdf_variable_content BLOB;
ALTER TABLE Pick ADD time_pdf_probability_content BLOB;
ALTER TABLE Pick ADD time_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Pick ADD horizontalSlowness_pdf_variable_content BLOB;
ALTER TABLE Pick ADD horizontalSlowness_pdf_probability_content BLOB;
ALTER TABLE Pick ADD horizontalSlowness_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Pick ADD backazimuth_pdf_variable_content BLOB;
ALTER TABLE Pick ADD backazimuth_pdf_probability_content BLOB;
ALTER TABLE Pick ADD backazimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

ALTER TABLE Origin ADD time_pdf_variable_content BLOB;
ALTER TABLE Origin ADD time_pdf_probability_content BLOB;
ALTER TABLE Origin ADD time_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Origin ADD latitude_pdf_variable_content BLOB;
ALTER TABLE Origin ADD latitude_pdf_probability_content BLOB;
ALTER TABLE Origin ADD latitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Origin ADD longitude_pdf_variable_content BLOB;
ALTER TABLE Origin ADD longitude_pdf_probability_content BLOB;
ALTER TABLE Origin ADD longitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0';
ALTER TABLE Origin ADD depth_pdf_variable_content BLOB;
ALTER TABLE Origin ADD depth_pdf_probability_content BLOB;
ALTER TABLE Origin ADD depth_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
```

**PostgreSQL**

```sql
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

UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
```

**Rationale**

Most of the inventory objects are valid for certain epochs defined with start
and end time. The database schema did not support microsecond storage of those
times although the structures in the source code do. This schema revision closes
the gap. Furthermore a ResponsePAZ filter could be part of the decimation filter
chain. Therefore the decimation attributes decimationFactor, delay and correction
have been added. Furthermore the ResponseIIR type has been added to correctly
store SEED response coefficients (blockette 54) without the need to convert IIR
filters to poles and zeros.

Furthermore a description of a pdf (probability density function) has been added
to the RealQuantity and TimeQuantity type.

**Important API changes**

The MagnitudeProcessor interface has changed to support regionalized
magnitude computations. The method ```computeMagnitude``` receives additionally
two parameters, the origin and the sensor location object.
All external magnitude plugins need to be adapted. Change from

```c++
Status computeMagnitude(double amplitude, double period,
                        double delta, double depth, double &value);
```

to

```c++
Status computeMagnitude(double amplitude, double period,
                        double delta, double depth,
#ifdef SC_API_VERSION >= SC_API_VERSION_CHECK(11,0,0)
                        const DataModel::Origin *hypocenter,
                        const DataModel::SensorLocation *receiver,
#endif
                        double &value);
```

Furthermore a new enumeration has been added to return the status of
the magnitude processing: ```EpicenterOutOfRegions```.

----

* trunk

  * Set seiscomp3 database bytea encoding to 'escape' for PostgreSQL database
    servers with version >= 9 in postgres.sql script.
  * Add InventorySyncMessage which is used to enclose an inventory synchronization
    process. An application can listen to that message and trigger processing of
    the updated inventory. The InventorySyncMessage is currently sent by
    scinv (seiscomp update inventory) to STATUS_GROUP.
  * Changed default publicID pattern from "@classname@#@time/%Y%m%d%H%M%S.%f@.@id@"
    to "@classname@/@time/%Y%m%d%H%M%S.%f@.@id@". The hash was removed due to
    possible conflicts with QuakeML publicID constraints.

* GUI

  * The event list shows status REVIEWED as V and FINAL as F
  * Added option to allow map layer visibilities and order
  * Allow to add custom map layers via plugins to the map
  * Refactored Map API (Canvas, Layer, Legend)
  * All GUI applications support an author and/or user blacklist to prevent sending
    messages to scmaster. This is not a proper secure access control implementation
    but helps to setup read-only applications to avoid accidental commits.
    ```
    blacklist.users = sysop1, sysop2
    blacklist.authors = sysop1@host, sysop2@host
    ```
  * Map layer drawing properties may be additionally defined in a "map.cfg" file
    located in the data set folder and subfolder, e.g. @~/.seiscomp3/fep/map.cfg@,
    @~/.seiscomp3/bna/map.cfg@, @~/.seiscomp3/bna/category/map.cfg@

* scmv

  * Added option ```expiredEventsInterval``` which controls the interval to check for expired events.
    The default value is 0 and does disable the interval check.
  * Added option to show the event table initially and to configure visible columns
    ```
    eventTable.visible = true
    eventTable.columns = Event, Depth
    ```
  * Add legend for event symbols

* scolv

  * ```locator.minimumDepth``` is now deprecated in favour of ```olv.locator.minimumDepth```
  * ```olv.locator``` is now deprecated in favour of ```olv.locator.interface```
  * Add option to configure the default checkstate of the event association button and
    fix origin button of the popup for committing with additional options: ```olv.commit.forceEventAssociation```
    and ```olv.commit.fixOrigin```. Either default value is true.
  * Add system tray icon which shows a notification if a new event
    has been detected. This can be disabled with
    ```
    olv.systemTray = false
    ```

* scqc

  * Added configuration option ```use3Components``` that allows to use all
    3 components of a configured station. This only applies if ```useConfiguredStreams```
    is active (default).

* scinv

  * Print file source of conflicting definitions

* seiscomp

  * An init script can not forward its configuration to another module. This is
    especially useful if e.g. ```seiscomp update-config scautopick``` is ran
    which did not do anything. Now it forwards its configuration to module
    *trunk* and will update the bindings database. The old behaviour has always
    confused users.

* dlsv2inv

  * Improve conversion to SC3. Many thanks to Arnaud Lemarchand from IPGP France
    for his exhaustive tests and valuable advises.

* fdsnxml2inv

  * Improve conversion to SC3. Many thanks to Arnaud Lemarchand from IPGP France
    for his exhaustive tests and valuable advises.
  * Declare NumeratorCoefficient.i as optional according to the official schema. Before
    that change, a lot of responses failed to convert.
  * Do not populate NumeratorCoefficient.i when converting to FDSNXML to avoid
    bloating the XML.

* scsohlog

  * Add description and documentation

* seedlink

  * Removed option -C from nmxptool plugin template. This should go into the
    additional options parameter.

## Release 2017.124

* seiscomp

  * Use symbolic links to module defaults and configurations instead of
    copying when creating module aliases

* doc

  * Added plugin section to module documentation with all documented
    plugins

* spread

  * Upgraded to version 4.4

* LocSAT

  * Started to clean up code and to remove f2c dependencies. Goal of this
    was to make compute_tt work in subsequent calls. Prior to the changes
    travel time computation with LocSAT gave unpredictable results which
    is now fixed.

* scconfig

  * Fixed issue with deleted structures when saving a configuration file.
    Prior to that fix the structure was not deleted.
  * Added documentation section which allows to browse changelogs and
    documentations of installed modules

* scautopick

  * Added the option ```killPendingSPickers``` to configure whether to
    terminate pending secondary processors if a new detection has been
    found or not. The downside of disabling that is that two picks will
    be possibly sent: a P and an S pick.

* GUI

  * Fixed bug in map tilestore that caused custom tilestore implementations
    to crash under certain circumstances
  * Add option to show times in localtime
    ```
    scheme.dateTime.useLocalTime = true
    ```
    This will show (hopefully) all times in the GUI with respect to the
    systems timezone instead of UTC.

* scesv

  * Fixed crash if no event was loaded and either "Show full tensor" or
    "Show waveform propagation" was toggled

* scrttv

  * If loading data from a file then all data is loaded and --buffer-size
    is being ignored. Furthermore XML event parameter files can be loaded
    (File->Open) and picks will be shown on top of loaded traces.

* scolv

  * Show symbols for unassociated station by default up to 360 degrees.
    This distance Can be changed with:
    ```
    # Show unassociated stations up to 20 degrees
    olv.map.stations.unassociatedMaxDist = 20
    ```
  * Apply show spectrogram values initially
  * Use separate trace color if spectrogram is shown: ```scheme.colors.records.spectrogram```
  * Read and apply scolv/global bindings to the repicker

* scmv

  * Add "Show Details" button to event details widget

* scevent

  * Added parameter to also compare picks that are associated with
    weight 0
    ```
    eventAssociation.allowLooseAssociatedArrivals = true
    ```
  * Added region check plugin that allows to configure a list of arbitrary
    regions and to set the event type to "outside of network interest" if
    its location is outside any region configured.

* scevtls

  * Added --modified-after option

* dlsv2inv, fdsnxml2inv

  * Make sample rate conversion from float to fraction more stable

* fdsnxml2inv

  * Fix epoch creation of sensor locations for some channel epoch
    combinations which caused the creation of two sensor locations
    with split epochs

* fdsnws

  * Make arclink-access bindings optional through configuration parameter
    useArclinkAccess
  * Add option ```recordBulkSize``` which defaults to 100kb and improves
    the dataselect performance significantly

* seedlink

  * Added ps2400_eth plugin configuration


## Release 2016.333

The database schema has changed since the previous version. To upgrade your
database from version 0.8 to 0.9 to following SQL script can be used:


**MYSQL**

```sql
ALTER TABLE ConfigStation ADD creationInfo_agencyID VARCHAR(64);
ALTER TABLE ConfigStation ADD creationInfo_agencyURI VARCHAR(255);
ALTER TABLE ConfigStation ADD creationInfo_author VARCHAR(128);
ALTER TABLE ConfigStation ADD creationInfo_authorURI VARCHAR(255);
ALTER TABLE ConfigStation ADD creationInfo_creationTime DATETIME;
ALTER TABLE ConfigStation ADD creationInfo_creationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD creationInfo_modificationTime DATETIME;
ALTER TABLE ConfigStation ADD creationInfo_modificationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD creationInfo_version VARCHAR(64);
ALTER TABLE ConfigStation ADD creationInfo_used TINYINT(1) NOT NULL DEFAULT '0';

UPDATE Meta SET value='0.9' WHERE name='Schema-Version';
```

**PostgreSQL**

```sql
ALTER TABLE ConfigStation ADD m_creationInfo_agencyID VARCHAR(64);
ALTER TABLE ConfigStation ADD m_creationInfo_agencyURI VARCHAR(255);
ALTER TABLE ConfigStation ADD m_creationInfo_author VARCHAR(128);
ALTER TABLE ConfigStation ADD m_creationInfo_authorURI VARCHAR(255);
ALTER TABLE ConfigStation ADD m_creationInfo_creationTime TIMESTAMP;
ALTER TABLE ConfigStation ADD m_creationInfo_creationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD m_creationInfo_modificationTime TIMESTAMP;
ALTER TABLE ConfigStation ADD m_creationInfo_modificationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD m_creationInfo_version VARCHAR(64);
ALTER TABLE ConfigStation ADD m_creationInfo_used BOOLEAN NOT NULL DEFAULT '0';

UPDATE Meta SET value='0.9' WHERE name='Schema-Version';
```

**Rationale**

The ConfigStation object will be updated by three instances in SeisComP3:
```seiscomp update-config```, ```scrttv``` and ```scqcv```. To track which
module has disabled or enabled a particular station the CreationInfo
structure has been added to ConfigStation.

----

* slmon

  * Ported package from SeisComP 2.5

* trunk

  * Set default author to appname@hostname instead of user@hostname
  * Upgraded rapidjson library to 1.1.0
  * Apply processing stage0 gain correction if streams gain frequency
    does not match the sensors gain frequency

* scconfig

  * Fix parameter tooltip if description text contains special HTML characters
    such as < (less than) or > (greater than)

* fdsnws

  * Add URL builder page to each service

* ql2sc

  * Use socket timeout of 60s if keepAlive is activated

* scinv

  * Split Spread messages into smaller chunks if the payload size exceeds
    allowed limit

* scautopick

  * Fixed removal of expired secondary pickers that caused a segmentation
    fault

* fdsnxml2inv

  * Correct types of some attributes of FDSNXML::PolynomialResponse
  * Convert polynomial responses correctly
  * Made FDSNXML::Comment::id optional according to the standard
  * Output line numbers in case of errors

* inv2dlsv

  * Fix handling of blockette62

* dlsv2inv

  * Fix bug that caused wrong sensor calibration gain

* seedlink

  * Fix Q330 setup if multiple instances are configured per station
  * Fix scream_ring setup in combination with a configured map file

* scwfparam

  * Apply lost patch again to use the same path name as earthquake.id for input files
  * Add configuration of output XML encoding due to Shakemap issues with non ASCII
    characters
  * Add station bindings to configure saturation threshold

* scrttv

  * Add option `autoApplyFilter` to apply the configured filter initially

* scalert

  * Fix Python import issue

* fdsnws

  * Add support for logging Arclink-style request statistics
  * Add support for EIDA authentication scheme
  * Set CORS headers to allow cross-site Javascript use
  * Fix geo filter for POST queries
  * Allow access to non-restricted streams even if network or station is marked as restricted
  * Include fdsnws_fetch client

* scevent

  * Fix segfault

* GUIs

  * Connection setup dialog removes the fetch database parameters button
    and replaces it with "Switch to reported" action that connects to the
    database as reported by scmaster while handshaking

* NonLinLoc

  * Fix compilation and linking for gcc >= 5

## Release 2016.161

The database schema has changed since the previous version. To upgrade your
database from version 0.7 to 0.8 the following SQL script can be used:

**MYSQL**

```sql
CREATE TABLE ResponseFAP (
  _oid INTEGER(11) NOT NULL,
  _parent_oid INTEGER(11) NOT NULL,
  _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  name VARCHAR(255),
  gain DOUBLE UNSIGNED,
  gainFrequency DOUBLE UNSIGNED,
  numberOfTuples SMALLINT UNSIGNED,
  tuples_content BLOB,
  tuples_used TINYINT(1) NOT NULL DEFAULT '0',
  remark_content BLOB,
  remark_used TINYINT(1) NOT NULL DEFAULT '0',
  PRIMARY KEY(_oid),
  INDEX(_parent_oid),
  FOREIGN KEY(_oid)
    REFERENCES Object(_oid)
    ON DELETE CASCADE,
  UNIQUE(_parent_oid,name)
) ENGINE=INNODB;

UPDATE Meta SET value='0.8' WHERE name='Schema-Version';
```

**PostgreSQL**

```sql
CREATE TABLE ResponseFAP (
  _oid BIGINT NOT NULL,
  _parent_oid BIGINT NOT NULL,
  _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  m_name VARCHAR(255),
  m_gain DOUBLE PRECISION,
  m_gainFrequency DOUBLE PRECISION,
  m_numberOfTuples SMALLINT,
  m_tuples_content BYTEA,
  m_tuples_used BOOLEAN NOT NULL DEFAULT '0',
  m_remark_content BYTEA,
  m_remark_used BOOLEAN NOT NULL DEFAULT '0',
  PRIMARY KEY(_oid),
  FOREIGN KEY(_oid)
    REFERENCES Object(_oid)
    ON DELETE CASCADE,
  UNIQUE(_parent_oid,m_name)
);

UPDATE Meta SET value='0.8' WHERE name='Schema-Version';
```

----

* documentation

  * Enhance documentation of scqc and others

* scart

  * Fix bug that caused wrong or no data to be returned if a request spans
    multiple years

* NonLinLoc

  * Fixed bug that caused wrong confidence ellipsoid axis length measures.
    Values are expected in meters but the wrapper exported then as kilometers.

* trunk

  * Add support for HMB (http messaging bus) messaging protocol
  * Add recordstream implementation for HMB (http messaging bus) protocol
  * Remove parent_oid foreign key constraint in database tables
  * Add support for FAP responses (response list) to datamodel
  * Add support for FAP responses to processing
  * Make ML logA0 configurable in bindings and add documentation
  * Hide database passwords in logfiles

* dlsv2inv

  * Add support for response list (blockette 55) which are converted to
    ResponseFAP

* fdsnxml2inv

  * Add support for reponse list
  * Prefer SampleRateRatio over SampleRate when converting to SC3

* scinv

  * Synchronize ResponseFAP objects

* scimport

  * Allow to configure the sink protocol

* scimex

  * Allow to configure the sink protocol

* msrtsimul

  * Wait until the time of the last data point in record has been reached
  * Add option to reopen the pipe if closed

* GUI

  * Display also degrees when measuring distance in a map

* scolv

  * Fix timing quality rendering in picker
  * Fix magnitude table distance entries if distance is displayed in km
  * Fix spectrum display on 32bit systems
  * Fall back to detecStream for vertical component in picker if 3
    components are not available in inventory
  * Add spectra plotting in picker (hit space if picking is disabled) for
    the current trace
  * Fix sorting of take off column in Arrival table
  * Allow columns to be reorderd for the current session

* scmv

  * Add plotting of event focal mechanism if available

* fseed

  * Add conversion from ResponseFAP to blockette 55

* system

  * Upgrade libmseed to 2.17

* seiscomp

  * Fixed resolving the path to the seiscomp script when it is a symlink

## Release 2016.062

* FDSNWS

 * Fixed missing lines in text output of station and event service
 * Added sorted text output in station service
 * Add inventory filter option for station and dataselect service
 * Fixed missing responses if public ID does start with *Response*

* seiscomp

 * Fixed parsing of the release string with lsb_release and CentOS
 * Fixed resolving directory links during SEISCOMP_ROOT determination
 * Added shell completion support for bash and fish (optional)
 * Added support for remote database servers when setting up SeisComP
 * Added Debian8 support to install-deps
 * Added output of exceptions when stopping a module which can sometimes
   fail when garbarge is written to pid files (to be investigated)

* trunk

 * Removed KIWI Greens function archive plugin
 * Removed MultiComponentArray from source to be replaced by a more advanced version
 * Fixed (hopefully) a synchronization issue when an applications shuts down that caused a mutex lock assertation
 * Fixed exception with unset attributes in format autoloc1 and autoloc3 in scbulletin
 * Fixed crash in scqc when an invalid regexp was used
 * Fixed azimuth output in fdsnxml2inv to stay in range [0,360)
 * Fixed database configuration reader issue with SQLite3 databases
 * Added Instaseis Greens function archive
 * Added bindings2cfg app that reads an key dir and generates config XML output into a file or to stdout
 * Added E?? to default channel regexp in scart
 * Added offline XML processing to scevent
 * Added printing of analogue response for digitizer stages in scinv ls
 * Added libbson and rapidjson as 3rd-party library to support object (de-)serialization to JSON/BSON and back
 * Added offline XML processing to scautoloc and a command line option to use manual picks for association (--use-manual-picks)
 * Added active record filtering (time and code) to file record stream
 * Added support for amplitude calculation on displacement
 * Added possibility to return polarity information in PickProcessors
 * Added database index to Amplitude.timeWindow.reference
 * Added options to configure deconvolution frequency tapers
 * Added filtering of records to the file data source according to requested
   time windows and channels

* scconfig

 * Added guard to start scconfig only once per installation directory
 * Show popup window during save if an application configuration file
   has been changed outside of scconfig and allow to decide which version
   to take

* Seedlink

 * Fixed compilation of Q330 plugin with Debian8 and probably others

* Hypo71

 * Fixed crash when built as release
 * Fixed bug for stations with elevation <-999
 * Fixed documentation
 * Fixed plugin use of snrMin when changed in the waveform review window

* scautopick

 * Added option to bindings (sensitivityCorrection) to correct for sensitivity
   before applying the filter
 * Added --playback option to not issue a time window request to the underlying
   data source which is important in connection with playbacks and offline runs
   with file sources, see documentation
 * Fixed adding S pick comments also in --ep mode

* scevent

 * Fixed association bug with automatic origins which prevented origins from
   being set preferred if the phase count is larget but the creation time is
   lower

* fdsnxml2inv

 * Only allow positive azimuth values

* scmv

 * Added option to display station names on startup and to configure if channel names are shown or not

   ```
   annotations = true
   annotionsWithChannels = false
   ``` 

* scrttv

 * Fixed crash when a station configuration was received that was not configured

* scolv

 * Fixed bug that disabled the locator profile selection box
 * Added spectrogram rendering for active trace
 * Added button to sort traces by azimuth

* GUI

 * Added options to configure trace line width, antialiasing and optimization

   ```
   scheme.records.lineWidth = 2
   scheme.records.antiAliasing = true
   scheme.records.optimize = false
   ```

* doc

 * Added Python example on how filter records

## Release 2015.149

* doc

 * Added mB time window computation
 * Corrected M(JMA) depth range
 * Made generator to follow symlinks

* trunk

 * Application class issues a warning if a loaded plugin is built against a different API version
 * Fixed compilation with boost 1.54
 * Bumped API version to 1.15
 * Improved finding 3 oriented components of a stream which should work also for UVW streams
 * !RecordStream "combined" introduces new syntax to enable combined of combined streams, e.g. combined://combined/(arclink/host1:18001;arclink/host2:18001??rtMax=86400);slink/host2:18000??rtMax=3600
 * Fixed fft computation bug
 * Set default taper length for restitution to 5s
 * Added Python wrappers for geo lib
 * Added scgitinit.sh to initialize a Git repository for SeisComP3 configuration files

* GUI

 * Corrected default color description for gaps
 * Added description of overlap and alignment bar color
 * Shutdown GUI applications gracefully on ctrl+c
 * Do not try to fetch database parameters from master if database is not enabled
 * Increased default size of plot symbols
 * Increased defaultstation symbol size

* scrttv

 * Added option to define multiple filters to cycle through

* scolv

 * Added more functional row selection and activation in magnitude view
 * Added warning to trace if not metadata are available
 * Added option to cycle through filters with keyboard
 * Do not show acquisition error box if acquisition has been cancelled by user
 
* tabinvmodifier

 * Fixed Python return codes for event handlers

* screloc

 * Added option --use-weight to forward arrival weights to locator
 * Added option --evaluation-mode to override default evaluation mode of relocated origin
 * Added option --ep for XML processing

* scwfparam

 * Added configuration parameter "SC3EventID" to set the event publicID as earthquake.id
 * Added configuration parameter "regionName" to set the events region name in locstring
 * Fixed processing bug

* fdsnws

 * Hide author from creationInfo in format=text, too
 * Fixed order by time if magnitude filter is enabled

* scamp

 * Added option --ep for XML processing

* scautopick

 * Added option --ep for XML processing

* scmag

 * Added option --ep for XML processing

## Release 2015.040

**WARNING:** Due to a fixed bug in the amplitude calculation if deconvolution is enabled the processing of amplitudes introduces now a delay of 60s unless
configured otherwise. Background: the default cosine taper time window length has been left unchanged for now and is still 60s at either side. To not affect
the signal time window the taper must start at signal end and thus 60s more data are required. That is usually not necessary and too much. Upcoming releases
will set the default to 5s but for now it is 60s. The configuration of "amplitudes.[type].resp.taper" in global bindings can fix it. As an override the following
changes can be applied to global.cfg:

``` c++
module.trunk.global.amplitudes.ML.resp.taper = 5.0
module.trunk.global.amplitudes.MLv.resp.taper = 5.0
module.trunk.global.amplitudes.MLh.resp.taper = 5.0
module.trunk.global.amplitudes.mb.resp.taper = 5.0
module.trunk.global.amplitudes.mB.resp.taper = 5.0
module.trunk.global.amplitudes.Mwp.resp.taper = 5.0
```

* seiscomp

 * Refuse to run as root unless "--asroot" is passed

   ```
   $ sudo seiscomp help
   [sudo] password for sysop:
   Running seiscomp as root is dangerous. Use --asroot if you know what you are doing
   $ sudo seiscomp --asroot help
   Available commands:
     install-deps
     setup
     ...
   ```

* VS

 * Fixed installation of !VsMag Python libraries if wrappers are disabled

* scvsmaglog

 * ActiveMQ messages can now be sent using either the QuakeML, SeisComP3ML, or ShakeAlertML format. This allows to use both the !ShakeAlert !UserDisplay (developed at Caltech) and the EEWD to receive real-time messages from VS in SeisComP3
 * Event ID added to the email subject
 * Additional configuration options enable operating several instances of scvsmaglog

* scenvelope

 * The default message group for envelope messages is now AMPLITUDE. Adding the message group VS is therefore no longer needed.

* scvsmag

 * Fixing a bug in reading Vs30 files for amplitude correction (thanks Palmi Erlendsson for spotting it!)
 * Fixing a bug in magnitude computation: the proxy for the frequency content of the earthquake signal is now taken into account in MVS.
 * Fixing a bug in the computation of the pick rate
 * The computation of the likelihood quantifying the quality of the VS alert has been slightly modified (see documentation of scvsmag for details)
 * The azimuthal gap is used as an additional quality criterion
 * VS envelope messages are now received by default from the message group AMPLITUDE

* ud_interface

 * Use xslt files to format AMQ messages

* Trunk

 * Added default port 80 to FDSN Webservice RecordStream
 * Made Helmberger format with 10 component files work with Green's functions
 * Removed gain correction (if gain was defined outside the target frequency of interest) and rely on operator to define reasonable Gains in inventory files
 * Added BOOST link and include directories if installed in a non standard location to projects missing that entries
 * Fixed compilation of GUI tree if boost is installed in non-standard location
 * Added "no-batch" option to Seedlink recordstream to connect to Seedlink servers that do not support BATCH requests
 * Fixed Seedlink connection with respect to reconnect that caused restart of the entire request
 * Fixed mapping to event type 'other' in QuakeML converter
 * Removed StationXML support which is superseded by FDSNXML
 * Create more database indexes as suggested by Fabian Engels
 * LocSAT: take weight into account when locate with a picklist
 * Allow to configure usage of responses per amplitude type
 * Clip MLv and Ms(BB) time window to given signalEnd
 * Added amplitude check for valid range if computing magnitudes
 * Return correct status if distance is out of range for ML
 * AmplitudeProcessor moved deconvolution cosine taper start to signal end to not damp the amplitudes

* GUI

 * Added scheme.precision.uncertainties description

* scolv

 * Show acquisition errors in a message box
 * Added smart layout mode to focal mechanism map
 * Group by agency for focal mechanisms
 * Added option picker.ignoreUnconfiguredStations to add only stations that are configured with "detecStream" when adding stations in a certain range
 * Added option to define picker alignment position: picker.alignmentPosition
 * Set computeMissingTakeOffAngles to true by default
 * Fixed crash when computing magnitudes
 * Populate Pick.filterID with currently active filter
 * Crash if raw data is shown in amplitude view and the time window is changed

* hypo71

 * Fixed bug when a pick has been deactivated in scolv
 * Fixed "fixed depth" issue
 * Removed OPTIONAL flag for Fortran compiler check if HYPO build is enabled

* fdsnws

 * Added hideAuthor config parameter
 * Added evaluationMode config parameter
 * Added event type white and blacklist
 * Make stream codes case sensitive
 * Fixed service outage on HTTP error 204
 * Fixed combined analogue and digital filter chain, https://github.com/SeisComP3/seiscomp3/issues/10
 * Avoid unnecessary array lookup

* fdsnxml2inv

 * Fixed order of response stages if pre-amp stages are used

* scevtlog

 * Fixed issue with duplicate magnitudes in XML files

* scbulletin

 * Added event type to format "-3"
 * Added Ms(BB) amplitudes and periods to station magnitude list (contributed by Aleksey Emanov)
 * Fixed output for XML if only an origin(ID) is given

* dlsv2inv

 * Added support for TEMPERATURE and PRESSURE channels when converting PAZ responses

* screloc

 * Added commandline option "dump" and "profile" to dump results in XML to stdout and to set a locator profile

* scautoloc

 * Added parameter autoloc.stationLocations to description

* scdbstrip

 * Disable daemon mode

* scimport

 * Reconnect to sink until connection can be established during init

* scautopick

 * Populate Pick.filterID with currently active filter

* scevent

 * Config parameters are float rather than int (Marc Grunberg)

* scwfparam

 * Use significant duration as processing time window scaled by optional durationScale parameter, replaced sensitivity correction filter by post-deconvolution filter
 * Replaced "2nd filter" with "filter" and use t05+duration*x
 * Implemented requested XML changes
 * Fixed invalid description file

* scconfig

 * Log XML file which produced an error

* Seedlink

 * lib330 update
 * Fixed Antelope ORB plugin setup
 * Added global access parameter configuration

* NLL

 * Applied patches from Anthony Lomax and Romane Racine (Swiss Seismological Service)

* Known bugs

 * scwfparam produces wrong amplitudes if deconvolution or acausal filtering is enabled. There are [https://github.com/SeisComP3/seiscomp3/commit/1b9ea9a0f9417d72317343ae87fbc9e2ea42df43 two] [https://github.com/SeisComP3/seiscomp3/commit/e5bb17fe3492b62004526524b6bde23e5f3e1d99 patches] available.
