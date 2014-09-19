DROP TABLE IF EXISTS EnvelopeValue;
DROP TABLE IF EXISTS EnvelopeChannel;
DROP TABLE IF EXISTS Envelope;

INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'VS');

CREATE TABLE EnvelopeValue (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	value DOUBLE NOT NULL,
	type VARCHAR(10) NOT NULL,
	quality VARCHAR(64),
	PRIMARY KEY(_oid),
	INDEX(type),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE EnvelopeChannel (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(8) NOT NULL,
	waveformID_networkCode CHAR(8) NOT NULL,
	waveformID_stationCode CHAR(8) NOT NULL,
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	PRIMARY KEY(_oid),
	INDEX(name),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE Envelope (
	_oid INTEGER(11) NOT NULL,
	_parent_oid INTEGER(11) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	network VARCHAR(8) NOT NULL,
	station VARCHAR(8) NOT NULL,
	timestamp DATETIME NOT NULL,
	timestamp_ms INTEGER NOT NULL,
	creationInfo_agencyID VARCHAR(64),
	creationInfo_agencyURI VARCHAR(255),
	creationInfo_author VARCHAR(128),
	creationInfo_authorURI VARCHAR(255),
	creationInfo_creationTime DATETIME,
	creationInfo_creationTime_ms INTEGER,
	creationInfo_modificationTime DATETIME,
	creationInfo_modificationTime_ms INTEGER,
	creationInfo_version VARCHAR(64),
	creationInfo_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(network),
	INDEX(station),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;
