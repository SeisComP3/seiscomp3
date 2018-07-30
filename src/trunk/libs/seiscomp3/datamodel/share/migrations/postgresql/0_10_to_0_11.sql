\echo Creating DataAvailability PublicObject entry
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'DataAvailability');


\echo Creating DataSegment
CREATE TABLE DataSegment (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP NOT NULL,
	m_end_ms INTEGER NOT NULL,
	m_updated TIMESTAMP NOT NULL,
	m_updated_ms INTEGER NOT NULL,
	m_sampleRate DOUBLE PRECISION NOT NULL,
	m_quality VARCHAR(8) NOT NULL,
	m_outOfOrder BOOLEAN NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	CONSTRAINT datasegment_composite_index UNIQUE(_parent_oid,m_start,m_start_ms)
);

CREATE INDEX DataSegment__parent_oid ON DataSegment(_parent_oid);
CREATE INDEX DataSegment_m_start_m_start_ms ON DataSegment(m_start,m_start_ms);
CREATE INDEX DataSegment_m_end_m_end_ms ON DataSegment(m_end,m_end_ms);
CREATE INDEX DataSegment_m_updated_m_updated_ms ON DataSegment(m_updated,m_updated_ms);

CREATE TRIGGER DataSegment_update BEFORE UPDATE ON DataSegment FOR EACH ROW EXECUTE PROCEDURE update_modified();


\echo Creating DataAttributeExtent
CREATE TABLE DataAttributeExtent (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP NOT NULL,
	m_end_ms INTEGER NOT NULL,
	m_sampleRate DOUBLE PRECISION NOT NULL,
	m_quality VARCHAR(8) NOT NULL,
	m_updated TIMESTAMP NOT NULL,
	m_updated_ms INTEGER NOT NULL,
	m_segmentCount INT NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	CONSTRAINT dataattributeextent_composite_index UNIQUE(_parent_oid,m_sampleRate,m_quality)
);

CREATE INDEX DataAttributeExtent__parent_oid ON DataAttributeExtent(_parent_oid);
CREATE INDEX DataAttributeExtent_m_start_m_start_ms ON DataAttributeExtent(m_start,m_start_ms);
CREATE INDEX DataAttributeExtent_m_end_m_end_ms ON DataAttributeExtent(m_end,m_end_ms);
CREATE INDEX DataAttributeExtent_m_updated_m_updated_ms ON DataAttributeExtent(m_updated,m_updated_ms);

CREATE TRIGGER DataAttributeExtent_update BEFORE UPDATE ON DataAttributeExtent FOR EACH ROW EXECUTE PROCEDURE update_modified();


\echo Creating DataExtent
CREATE TABLE DataExtent (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP NOT NULL,
	m_end_ms INTEGER NOT NULL,
	m_updated TIMESTAMP NOT NULL,
	m_updated_ms INTEGER NOT NULL,
	m_lastScan TIMESTAMP NOT NULL,
	m_lastScan_ms INTEGER NOT NULL,
	m_segmentOverflow BOOLEAN NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	CONSTRAINT dataextent_composite_index UNIQUE(_parent_oid,m_waveformID_networkCode,m_waveformID_stationCode,m_waveformID_locationCode,m_waveformID_channelCode,m_waveformID_resourceURI)
);

CREATE INDEX DataExtent__parent_oid ON DataExtent(_parent_oid);
CREATE INDEX DataExtent_m_waveformID_resourceURI ON DataExtent(m_waveformID_resourceURI);
CREATE INDEX DataExtent_m_start_m_start_ms ON DataExtent(m_start,m_start_ms);
CREATE INDEX DataExtent_m_end_m_end_ms ON DataExtent(m_end,m_end_ms);
CREATE INDEX DataExtent_m_updated_m_updated_ms ON DataExtent(m_updated,m_updated_ms);
CREATE INDEX DataExtent_m_lastScan_m_lastScan_ms ON DataExtent(m_lastScan,m_lastScan_ms);

CREATE TRIGGER DataExtent_update BEFORE UPDATE ON DataExtent FOR EACH ROW EXECUTE PROCEDURE update_modified();


\echo Updating Meta
UPDATE Meta SET value='0.11' WHERE name='Schema-Version';
