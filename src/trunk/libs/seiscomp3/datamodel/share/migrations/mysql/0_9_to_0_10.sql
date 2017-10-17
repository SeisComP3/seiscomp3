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

UPDATE Meta SET value='0.10' WHERE name='Schema-Version';
