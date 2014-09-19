DROP TABLE EnvelopeValue;
DROP TABLE EnvelopeChannel;
DROP TABLE Envelope;

INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'VS');

CREATE TABLE EnvelopeValue (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	value DOUBLE NOT NULL,
	type VARCHAR NOT NULL,
	quality VARCHAR(64),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX EnvelopeValue_type ON EnvelopeValue(type);

CREATE TRIGGER EnvelopeValueUpdate UPDATE ON EnvelopeValue
BEGIN
  UPDATE EnvelopeValue SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE EnvelopeChannel (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	waveformID_networkCode CHAR NOT NULL,
	waveformID_stationCode CHAR NOT NULL,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX EnvelopeChannel_name ON EnvelopeChannel(name);

CREATE TRIGGER EnvelopeChannelUpdate UPDATE ON EnvelopeChannel
BEGIN
  UPDATE EnvelopeChannel SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Envelope (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	network VARCHAR NOT NULL,
	station VARCHAR NOT NULL,
	timestamp DATETIME NOT NULL,
	timestamp_ms INTEGER NOT NULL,
	creationInfo_agencyID VARCHAR,
	creationInfo_agencyURI VARCHAR,
	creationInfo_author VARCHAR,
	creationInfo_authorURI VARCHAR,
	creationInfo_creationTime DATETIME,
	creationInfo_creationTime_ms INTEGER,
	creationInfo_modificationTime DATETIME,
	creationInfo_modificationTime_ms INTEGER,
	creationInfo_version VARCHAR,
	creationInfo_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX Envelope_network ON Envelope(network);
CREATE INDEX Envelope_station ON Envelope(station);

CREATE TRIGGER EnvelopeUpdate UPDATE ON Envelope
BEGIN
  UPDATE Envelope SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;
