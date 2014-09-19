DROP TABLE FilterParameter;
DROP TABLE SimpleFilter;
DROP TABLE SimpleFilterChainMember;
DROP TABLE PeakMotion;
DROP TABLE Record;
DROP TABLE EventRecordReference;
DROP TABLE Rupture;
DROP TABLE StrongOriginDescription;

INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'StrongMotionParameters');

CREATE TABLE FilterParameter (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_value_value DOUBLE PRECISION NOT NULL,
	m_value_uncertainty DOUBLE PRECISION,
	m_value_lowerUncertainty DOUBLE PRECISION,
	m_value_upperUncertainty DOUBLE PRECISION,
	m_value_confidenceLevel DOUBLE PRECISION,
	m_name VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER FilterParameter_update BEFORE UPDATE ON FilterParameter FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE SimpleFilter (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_type VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER SimpleFilter_update BEFORE UPDATE ON SimpleFilter FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE SimpleFilterChainMember (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_sequenceNo INT NOT NULL,
	m_simpleFilterID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,m_sequenceNo)
);

CREATE INDEX SimpleFilterChainMember_m_simpleFilterID ON SimpleFilterChainMember(m_simpleFilterID);

CREATE TRIGGER SimpleFilterChainMember_update BEFORE UPDATE ON SimpleFilterChainMember FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE PeakMotion (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_motion_value DOUBLE PRECISION NOT NULL,
	m_motion_uncertainty DOUBLE PRECISION,
	m_motion_lowerUncertainty DOUBLE PRECISION,
	m_motion_upperUncertainty DOUBLE PRECISION,
	m_motion_confidenceLevel DOUBLE PRECISION,
	m_type VARCHAR(255) NOT NULL,
	m_period DOUBLE PRECISION,
	m_damping DOUBLE PRECISION,
	m_method VARCHAR(255),
	m_atTime_value TIMESTAMP,
	m_atTime_value_ms INTEGER,
	m_atTime_uncertainty DOUBLE PRECISION,
	m_atTime_lowerUncertainty DOUBLE PRECISION,
	m_atTime_upperUncertainty DOUBLE PRECISION,
	m_atTime_confidenceLevel DOUBLE PRECISION,
	m_atTime_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER PeakMotion_update BEFORE UPDATE ON PeakMotion FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Record (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
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
	m_gainUnit VARCHAR(20),
	m_duration DOUBLE PRECISION,
	m_startTime_value TIMESTAMP NOT NULL,
	m_startTime_value_ms INTEGER NOT NULL,
	m_startTime_uncertainty DOUBLE PRECISION,
	m_startTime_lowerUncertainty DOUBLE PRECISION,
	m_startTime_upperUncertainty DOUBLE PRECISION,
	m_startTime_confidenceLevel DOUBLE PRECISION,
	m_owner_name VARCHAR(255),
	m_owner_forename VARCHAR(255),
	m_owner_agency VARCHAR(255),
	m_owner_department VARCHAR(255),
	m_owner_address VARCHAR(255),
	m_owner_phone VARCHAR(255),
	m_owner_email VARCHAR(255),
	m_owner_used BOOLEAN NOT NULL DEFAULT '0',
	m_resampleRateNumerator INT,
	m_resampleRateDenominator INT,
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_waveformFile_creationInfo_agencyID VARCHAR(64),
	m_waveformFile_creationInfo_agencyURI VARCHAR(255),
	m_waveformFile_creationInfo_author VARCHAR(128),
	m_waveformFile_creationInfo_authorURI VARCHAR(255),
	m_waveformFile_creationInfo_creationTime TIMESTAMP,
	m_waveformFile_creationInfo_creationTime_ms INTEGER,
	m_waveformFile_creationInfo_modificationTime TIMESTAMP,
	m_waveformFile_creationInfo_modificationTime_ms INTEGER,
	m_waveformFile_creationInfo_version VARCHAR(64),
	m_waveformFile_creationInfo_used BOOLEAN NOT NULL DEFAULT '0',
	m_waveformFile_class VARCHAR(255),
	m_waveformFile_type VARCHAR(255),
	m_waveformFile_filename VARCHAR(255),
	m_waveformFile_url VARCHAR(255),
	m_waveformFile_description VARCHAR(255),
	m_waveformFile_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER Record_update BEFORE UPDATE ON Record FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE EventRecordReference (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_recordID VARCHAR(255) NOT NULL,
	m_campbellDistance_value DOUBLE PRECISION,
	m_campbellDistance_uncertainty DOUBLE PRECISION,
	m_campbellDistance_lowerUncertainty DOUBLE PRECISION,
	m_campbellDistance_upperUncertainty DOUBLE PRECISION,
	m_campbellDistance_confidenceLevel DOUBLE PRECISION,
	m_campbellDistance_used BOOLEAN NOT NULL DEFAULT '0',
	m_ruptureToStationAzimuth_value DOUBLE PRECISION,
	m_ruptureToStationAzimuth_uncertainty DOUBLE PRECISION,
	m_ruptureToStationAzimuth_lowerUncertainty DOUBLE PRECISION,
	m_ruptureToStationAzimuth_upperUncertainty DOUBLE PRECISION,
	m_ruptureToStationAzimuth_confidenceLevel DOUBLE PRECISION,
	m_ruptureToStationAzimuth_used BOOLEAN NOT NULL DEFAULT '0',
	m_ruptureAreaDistance_value DOUBLE PRECISION,
	m_ruptureAreaDistance_uncertainty DOUBLE PRECISION,
	m_ruptureAreaDistance_lowerUncertainty DOUBLE PRECISION,
	m_ruptureAreaDistance_upperUncertainty DOUBLE PRECISION,
	m_ruptureAreaDistance_confidenceLevel DOUBLE PRECISION,
	m_ruptureAreaDistance_used BOOLEAN NOT NULL DEFAULT '0',
	m_JoynerBooreDistance_value DOUBLE PRECISION,
	m_JoynerBooreDistance_uncertainty DOUBLE PRECISION,
	m_JoynerBooreDistance_lowerUncertainty DOUBLE PRECISION,
	m_JoynerBooreDistance_upperUncertainty DOUBLE PRECISION,
	m_JoynerBooreDistance_confidenceLevel DOUBLE PRECISION,
	m_JoynerBooreDistance_used BOOLEAN NOT NULL DEFAULT '0',
	m_closestFaultDistance_value DOUBLE PRECISION,
	m_closestFaultDistance_uncertainty DOUBLE PRECISION,
	m_closestFaultDistance_lowerUncertainty DOUBLE PRECISION,
	m_closestFaultDistance_upperUncertainty DOUBLE PRECISION,
	m_closestFaultDistance_confidenceLevel DOUBLE PRECISION,
	m_closestFaultDistance_used BOOLEAN NOT NULL DEFAULT '0',
	m_preEventLength DOUBLE PRECISION,
	m_postEventLength DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX EventRecordReference_m_recordID ON EventRecordReference(m_recordID);

CREATE TRIGGER EventRecordReference_update BEFORE UPDATE ON EventRecordReference FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Rupture (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_width_value DOUBLE PRECISION,
	m_width_uncertainty DOUBLE PRECISION,
	m_width_lowerUncertainty DOUBLE PRECISION,
	m_width_upperUncertainty DOUBLE PRECISION,
	m_width_confidenceLevel DOUBLE PRECISION,
	m_width_used BOOLEAN NOT NULL DEFAULT '0',
	m_displacement_value DOUBLE PRECISION,
	m_displacement_uncertainty DOUBLE PRECISION,
	m_displacement_lowerUncertainty DOUBLE PRECISION,
	m_displacement_upperUncertainty DOUBLE PRECISION,
	m_displacement_confidenceLevel DOUBLE PRECISION,
	m_displacement_used BOOLEAN NOT NULL DEFAULT '0',
	m_riseTime_value DOUBLE PRECISION,
	m_riseTime_uncertainty DOUBLE PRECISION,
	m_riseTime_lowerUncertainty DOUBLE PRECISION,
	m_riseTime_upperUncertainty DOUBLE PRECISION,
	m_riseTime_confidenceLevel DOUBLE PRECISION,
	m_riseTime_used BOOLEAN NOT NULL DEFAULT '0',
	m_vt_to_vs_value DOUBLE PRECISION,
	m_vt_to_vs_uncertainty DOUBLE PRECISION,
	m_vt_to_vs_lowerUncertainty DOUBLE PRECISION,
	m_vt_to_vs_upperUncertainty DOUBLE PRECISION,
	m_vt_to_vs_confidenceLevel DOUBLE PRECISION,
	m_vt_to_vs_used BOOLEAN NOT NULL DEFAULT '0',
	m_shallowAsperityDepth_value DOUBLE PRECISION,
	m_shallowAsperityDepth_uncertainty DOUBLE PRECISION,
	m_shallowAsperityDepth_lowerUncertainty DOUBLE PRECISION,
	m_shallowAsperityDepth_upperUncertainty DOUBLE PRECISION,
	m_shallowAsperityDepth_confidenceLevel DOUBLE PRECISION,
	m_shallowAsperityDepth_used BOOLEAN NOT NULL DEFAULT '0',
	m_shallowAsperity BOOLEAN,
	m_literatureSource_title VARCHAR(255),
	m_literatureSource_firstAuthorName VARCHAR(255),
	m_literatureSource_firstAuthorForename VARCHAR(255),
	m_literatureSource_secondaryAuthors VARCHAR(255),
	m_literatureSource_doi VARCHAR(255),
	m_literatureSource_year INT,
	m_literatureSource_in_title VARCHAR(255),
	m_literatureSource_editor VARCHAR(255),
	m_literatureSource_place VARCHAR(255),
	m_literatureSource_language VARCHAR(255),
	m_literatureSource_tome INT,
	m_literatureSource_page_from INT,
	m_literatureSource_page_to INT,
	m_literatureSource_used BOOLEAN NOT NULL DEFAULT '0',
	m_slipVelocity_value DOUBLE PRECISION,
	m_slipVelocity_uncertainty DOUBLE PRECISION,
	m_slipVelocity_lowerUncertainty DOUBLE PRECISION,
	m_slipVelocity_upperUncertainty DOUBLE PRECISION,
	m_slipVelocity_confidenceLevel DOUBLE PRECISION,
	m_slipVelocity_used BOOLEAN NOT NULL DEFAULT '0',
	m_length_value DOUBLE PRECISION,
	m_length_uncertainty DOUBLE PRECISION,
	m_length_lowerUncertainty DOUBLE PRECISION,
	m_length_upperUncertainty DOUBLE PRECISION,
	m_length_confidenceLevel DOUBLE PRECISION,
	m_length_used BOOLEAN NOT NULL DEFAULT '0',
	m_area_value DOUBLE PRECISION,
	m_area_uncertainty DOUBLE PRECISION,
	m_area_lowerUncertainty DOUBLE PRECISION,
	m_area_upperUncertainty DOUBLE PRECISION,
	m_area_confidenceLevel DOUBLE PRECISION,
	m_area_used BOOLEAN NOT NULL DEFAULT '0',
	m_ruptureVelocity_value DOUBLE PRECISION,
	m_ruptureVelocity_uncertainty DOUBLE PRECISION,
	m_ruptureVelocity_lowerUncertainty DOUBLE PRECISION,
	m_ruptureVelocity_upperUncertainty DOUBLE PRECISION,
	m_ruptureVelocity_confidenceLevel DOUBLE PRECISION,
	m_ruptureVelocity_used BOOLEAN NOT NULL DEFAULT '0',
	m_stressdrop_value DOUBLE PRECISION,
	m_stressdrop_uncertainty DOUBLE PRECISION,
	m_stressdrop_lowerUncertainty DOUBLE PRECISION,
	m_stressdrop_upperUncertainty DOUBLE PRECISION,
	m_stressdrop_confidenceLevel DOUBLE PRECISION,
	m_stressdrop_used BOOLEAN NOT NULL DEFAULT '0',
	m_momentReleaseTop5km_value DOUBLE PRECISION,
	m_momentReleaseTop5km_uncertainty DOUBLE PRECISION,
	m_momentReleaseTop5km_lowerUncertainty DOUBLE PRECISION,
	m_momentReleaseTop5km_upperUncertainty DOUBLE PRECISION,
	m_momentReleaseTop5km_confidenceLevel DOUBLE PRECISION,
	m_momentReleaseTop5km_used BOOLEAN NOT NULL DEFAULT '0',
	m_fwHwIndicator VARCHAR(64),
	m_ruptureGeometryWKT VARCHAR(255),
	m_faultID VARCHAR(255) NOT NULL,
	m_surfaceRupture_observed BOOLEAN,
	m_surfaceRupture_evidence VARCHAR(255),
	m_surfaceRupture_literatureSource_title VARCHAR(255),
	m_surfaceRupture_literatureSource_firstAuthorName VARCHAR(255),
	m_surfaceRupture_literatureSource_firstAuthorForename VARCHAR(255),
	m_surfaceRupture_literatureSource_secondaryAuthors VARCHAR(255),
	m_surfaceRupture_literatureSource_doi VARCHAR(255),
	m_surfaceRupture_literatureSource_year INT,
	m_surfaceRupture_literatureSource_in_title VARCHAR(255),
	m_surfaceRupture_literatureSource_editor VARCHAR(255),
	m_surfaceRupture_literatureSource_place VARCHAR(255),
	m_surfaceRupture_literatureSource_language VARCHAR(255),
	m_surfaceRupture_literatureSource_tome INT,
	m_surfaceRupture_literatureSource_page_from INT,
	m_surfaceRupture_literatureSource_page_to INT,
	m_surfaceRupture_literatureSource_used BOOLEAN NOT NULL DEFAULT '0',
	m_surfaceRupture_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX Rupture_m_ruptureGeometryWKT ON Rupture(m_ruptureGeometryWKT);

CREATE TRIGGER Rupture_update BEFORE UPDATE ON Rupture FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE StrongOriginDescription (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_originID VARCHAR(255) NOT NULL,
	m_waveformCount INT,
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

CREATE INDEX StrongOriginDescription_m_originID ON StrongOriginDescription(m_originID);

CREATE TRIGGER StrongOriginDescription_update BEFORE UPDATE ON StrongOriginDescription FOR EACH ROW EXECUTE PROCEDURE update_modified();

