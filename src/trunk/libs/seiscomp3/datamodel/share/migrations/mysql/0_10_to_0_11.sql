SELECT 'Add StationMagnitude.passedQC' AS '';
ALTER TABLE StationMagnitude ADD passedQC TINYINT(1) AFTER waveformID_used;

SELECT 'Creating DataAvailability PublicObject entry' AS '';
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'DataAvailability');

SELECT 'Creating DataSegment' AS '';
CREATE TABLE DataSegment (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	updated DATETIME NOT NULL,
	updated_ms INTEGER NOT NULL,
	sampleRate DOUBLE NOT NULL,
	quality VARCHAR(8) NOT NULL,
	outOfOrder TINYINT(1) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(start,start_ms),
	INDEX(end,end_ms),
	INDEX(updated,updated_ms),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,start,start_ms)
) ENGINE=INNODB;

SELECT 'Creating DataAttributeExtent' AS '';
CREATE TABLE DataAttributeExtent (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	sampleRate DOUBLE NOT NULL,
	quality VARCHAR(8) NOT NULL,
	updated DATETIME NOT NULL,
	updated_ms INTEGER NOT NULL,
	segmentCount INT UNSIGNED NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(start,start_ms),
	INDEX(end,end_ms),
	INDEX(updated,updated_ms),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,sampleRate,quality)
) ENGINE=INNODB;

SELECT 'Creating DataExtent' AS '';
CREATE TABLE DataExtent (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR(8) NOT NULL,
	waveformID_stationCode CHAR(8) NOT NULL,
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	updated DATETIME NOT NULL,
	updated_ms INTEGER NOT NULL,
	lastScan DATETIME NOT NULL,
	lastScan_ms INTEGER NOT NULL,
	segmentOverflow TINYINT(1) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(waveformID_resourceURI),
	INDEX(start,start_ms),
	INDEX(end,end_ms),
	INDEX(updated,updated_ms),
	INDEX(lastScan,lastScan_ms),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI)
) ENGINE=INNODB;

SELECT 'Updating Meta' AS '';
UPDATE Meta SET value='0.11' WHERE name='Schema-Version';

