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
