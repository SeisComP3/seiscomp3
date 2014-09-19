DROP TABLE EnvelopeValue;
DROP TABLE EnvelopeChannel;
DROP TABLE Envelope;

INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'VS');

CREATE TABLE EnvelopeValue (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_value DOUBLE PRECISION NOT NULL,
	m_type VARCHAR(10) NOT NULL,
	m_quality VARCHAR(64),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX EnvelopeValue_m_type ON EnvelopeValue(m_type);

CREATE TRIGGER EnvelopeValue_update BEFORE UPDATE ON EnvelopeValue FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE EnvelopeChannel (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(8) NOT NULL,
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX EnvelopeChannel_m_name ON EnvelopeChannel(m_name);

CREATE TRIGGER EnvelopeChannel_update BEFORE UPDATE ON EnvelopeChannel FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Envelope (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_network VARCHAR(8) NOT NULL,
	m_station VARCHAR(8) NOT NULL,
	m_timestamp TIMESTAMP NOT NULL,
	m_timestamp_ms INTEGER NOT NULL,
	m_creationInfo_agencyID VARCHAR(64),
	m_creationInfo_agencyURI VARCHAR(255),
	m_creationInfo_author VARCHAR(128),
	m_creationInfo_authorURI VARCHAR(255),
	m_creationInfo_creationTime TIMESTAMP,
	m_creationInfo_creationTime_ms INTEGER,
	m_creationInfo_modificationTime TIMESTAMP,
	m_creationInfo_modificationTime_ms INTEGER,
	m_creationInfo_version VARCHAR(64),
	m_creationInfo_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX Envelope_m_network ON Envelope(m_network);
CREATE INDEX Envelope_m_station ON Envelope(m_station);

CREATE TRIGGER Envelope_update BEFORE UPDATE ON Envelope FOR EACH ROW EXECUTE PROCEDURE update_modified();

