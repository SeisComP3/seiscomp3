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
