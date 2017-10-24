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
UPDATE StationGroup SET start_ms=0;
ALTER TABLE StationGroup ADD end_ms INTEGER AFTER end;
UPDATE StationGroup SET end_ms=0;
ALTER TABLE DataloggerCalibration ADD start_ms INTEGER AFTER start;
UPDATE DataloggerCalibration SET start_ms=0;
ALTER TABLE DataloggerCalibration ADD end_ms INTEGER AFTER end;
UPDATE DataloggerCalibration SET end_ms=0;
DROP INDEX _parent_oid_2 ON DataloggerCalibration;
ALTER TABLE DataloggerCalibration ADD CONSTRAINT composite_index UNIQUE(_parent_oid,serialNumber,channel,start,start_ms);
ALTER TABLE SensorCalibration ADD start_ms INTEGER AFTER start;
UPDATE SensorCalibration SET start_ms=0;
ALTER TABLE SensorCalibration ADD end_ms INTEGER AFTER end;
UPDATE SensorCalibration SET end_ms=0;
ALTER TABLE SensorCalibration MODIFY gain DOUBLE;
DROP INDEX _parent_oid_2 ON SensorCalibration;
ALTER TABLE SensorCalibration ADD CONSTRAINT composite_index UNIQUE(_parent_oid,serialNumber,channel,start,start_ms);
ALTER TABLE AuxStream ADD start_ms INTEGER AFTER start;
UPDATE AuxStream SET start_ms=0;
ALTER TABLE AuxStream ADD end_ms INTEGER AFTER end;
UPDATE AuxStream SET end_ms=0;
DROP INDEX _parent_oid_2 ON AuxStream;
ALTER TABLE AuxStream ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE Stream ADD start_ms INTEGER AFTER start;
UPDATE Stream SET start_ms=0;
ALTER TABLE Stream ADD end_ms INTEGER AFTER end;
UPDATE Stream SET end_ms=0;
DROP INDEX _parent_oid_2 ON Stream;
ALTER TABLE Stream ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE SensorLocation ADD start_ms INTEGER AFTER start;
UPDATE SensorLocation SET start_ms=0;
ALTER TABLE SensorLocation ADD end_ms INTEGER AFTER end;
UPDATE SensorLocation SET end_ms=0;
DROP INDEX _parent_oid_2 ON SensorLocation;
ALTER TABLE SensorLocation ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE Station ADD start_ms INTEGER AFTER start;
UPDATE Station SET start_ms=0;
ALTER TABLE Station ADD end_ms INTEGER AFTER end;
UPDATE Station SET end_ms=0;
DROP INDEX _parent_oid_2 ON Station;
ALTER TABLE Station ADD CONSTRAINT composite_index UNIQUE(_parent_oid,code,start,start_ms);
ALTER TABLE Network ADD start_ms INTEGER AFTER start;
UPDATE Network SET start_ms=0;
ALTER TABLE Network ADD end_ms INTEGER AFTER end;
UPDATE Network SET end_ms=0;
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

# Convert Stream type to type that inherits from PublicObject
INSERT INTO PublicObject(_oid, publicID) SELECT _oid, concat("Stream/", DATE_FORMAT(_last_modified, '%Y%m%d%H%i%s'), ".", _oid) FROM Stream;

UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
