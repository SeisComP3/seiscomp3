DROP TABLE FilterParameter;
DROP TABLE SimpleFilter;
DROP TABLE SimpleFilterChainMember;
DROP TABLE PeakMotion;
DROP TABLE Record;
DROP TABLE EventRecordReference;
DROP TABLE Rupture;
DROP TABLE StrongOriginDescription;

INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'StrongMotionParameters');

CREATE TABLE FilterParameter (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	value_value DOUBLE NOT NULL,
	value_uncertainty DOUBLE UNSIGNED,
	value_lowerUncertainty DOUBLE UNSIGNED,
	value_upperUncertainty DOUBLE UNSIGNED,
	value_confidenceLevel DOUBLE UNSIGNED,
	value_pdf_variable_content BLOB,
	value_pdf_probability_content BLOB,
	value_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	name VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER FilterParameterUpdate UPDATE ON FilterParameter
BEGIN
  UPDATE FilterParameter SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE SimpleFilter (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	type VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER SimpleFilterUpdate UPDATE ON SimpleFilter
BEGIN
  UPDATE SimpleFilter SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE SimpleFilterChainMember (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	sequenceNo INT UNSIGNED NOT NULL,
	simpleFilterID VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,sequenceNo)
);

CREATE INDEX SimpleFilterChainMember_simpleFilterID ON SimpleFilterChainMember(simpleFilterID);

CREATE TRIGGER SimpleFilterChainMemberUpdate UPDATE ON SimpleFilterChainMember
BEGIN
  UPDATE SimpleFilterChainMember SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE PeakMotion (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	motion_value DOUBLE NOT NULL,
	motion_uncertainty DOUBLE UNSIGNED,
	motion_lowerUncertainty DOUBLE UNSIGNED,
	motion_upperUncertainty DOUBLE UNSIGNED,
	motion_confidenceLevel DOUBLE UNSIGNED,
	motion_pdf_variable_content BLOB,
	motion_pdf_probability_content BLOB,
	motion_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	type VARCHAR NOT NULL,
	period DOUBLE UNSIGNED,
	damping DOUBLE UNSIGNED,
	method VARCHAR,
	atTime_value DATETIME,
	atTime_value_ms INTEGER,
	atTime_uncertainty DOUBLE UNSIGNED,
	atTime_lowerUncertainty DOUBLE UNSIGNED,
	atTime_upperUncertainty DOUBLE UNSIGNED,
	atTime_confidenceLevel DOUBLE UNSIGNED,
	atTime_pdf_variable_content BLOB,
	atTime_pdf_probability_content BLOB,
	atTime_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	atTime_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER PeakMotionUpdate UPDATE ON PeakMotion
BEGIN
  UPDATE PeakMotion SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Record (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
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
	gainUnit CHAR,
	duration DOUBLE,
	startTime_value DATETIME NOT NULL,
	startTime_value_ms INTEGER NOT NULL,
	startTime_uncertainty DOUBLE UNSIGNED,
	startTime_lowerUncertainty DOUBLE UNSIGNED,
	startTime_upperUncertainty DOUBLE UNSIGNED,
	startTime_confidenceLevel DOUBLE UNSIGNED,
	startTime_pdf_variable_content BLOB,
	startTime_pdf_probability_content BLOB,
	startTime_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	owner_name VARCHAR,
	owner_forename VARCHAR,
	owner_agency VARCHAR,
	owner_department VARCHAR,
	owner_address VARCHAR,
	owner_phone VARCHAR,
	owner_email VARCHAR,
	owner_used INTEGER(1) NOT NULL DEFAULT '0',
	resampleRateNumerator INT UNSIGNED,
	resampleRateDenominator INT UNSIGNED,
	waveformID_networkCode CHAR NOT NULL,
	waveformID_stationCode CHAR NOT NULL,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	waveformFile_creationInfo_agencyID VARCHAR,
	waveformFile_creationInfo_agencyURI VARCHAR,
	waveformFile_creationInfo_author VARCHAR,
	waveformFile_creationInfo_authorURI VARCHAR,
	waveformFile_creationInfo_creationTime DATETIME,
	waveformFile_creationInfo_creationTime_ms INTEGER,
	waveformFile_creationInfo_modificationTime DATETIME,
	waveformFile_creationInfo_modificationTime_ms INTEGER,
	waveformFile_creationInfo_version VARCHAR,
	waveformFile_creationInfo_used INTEGER(1) NOT NULL DEFAULT '0',
	waveformFile_class VARCHAR,
	waveformFile_type VARCHAR,
	waveformFile_filename VARCHAR,
	waveformFile_url VARCHAR,
	waveformFile_description VARCHAR,
	waveformFile_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER RecordUpdate UPDATE ON Record
BEGIN
  UPDATE Record SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE EventRecordReference (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	recordID VARCHAR NOT NULL,
	campbellDistance_value DOUBLE,
	campbellDistance_uncertainty DOUBLE UNSIGNED,
	campbellDistance_lowerUncertainty DOUBLE UNSIGNED,
	campbellDistance_upperUncertainty DOUBLE UNSIGNED,
	campbellDistance_confidenceLevel DOUBLE UNSIGNED,
	campbellDistance_pdf_variable_content BLOB,
	campbellDistance_pdf_probability_content BLOB,
	campbellDistance_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	campbellDistance_used INTEGER(1) NOT NULL DEFAULT '0',
	ruptureToStationAzimuth_value DOUBLE,
	ruptureToStationAzimuth_uncertainty DOUBLE UNSIGNED,
	ruptureToStationAzimuth_lowerUncertainty DOUBLE UNSIGNED,
	ruptureToStationAzimuth_upperUncertainty DOUBLE UNSIGNED,
	ruptureToStationAzimuth_confidenceLevel DOUBLE UNSIGNED,
	ruptureToStationAzimuth_pdf_variable_content BLOB,
	ruptureToStationAzimuth_pdf_probability_content BLOB,
	ruptureToStationAzimuth_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	ruptureToStationAzimuth_used INTEGER(1) NOT NULL DEFAULT '0',
	ruptureAreaDistance_value DOUBLE,
	ruptureAreaDistance_uncertainty DOUBLE UNSIGNED,
	ruptureAreaDistance_lowerUncertainty DOUBLE UNSIGNED,
	ruptureAreaDistance_upperUncertainty DOUBLE UNSIGNED,
	ruptureAreaDistance_confidenceLevel DOUBLE UNSIGNED,
	ruptureAreaDistance_pdf_variable_content BLOB,
	ruptureAreaDistance_pdf_probability_content BLOB,
	ruptureAreaDistance_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	ruptureAreaDistance_used INTEGER(1) NOT NULL DEFAULT '0',
	JoynerBooreDistance_value DOUBLE,
	JoynerBooreDistance_uncertainty DOUBLE UNSIGNED,
	JoynerBooreDistance_lowerUncertainty DOUBLE UNSIGNED,
	JoynerBooreDistance_upperUncertainty DOUBLE UNSIGNED,
	JoynerBooreDistance_confidenceLevel DOUBLE UNSIGNED,
	JoynerBooreDistance_pdf_variable_content BLOB,
	JoynerBooreDistance_pdf_probability_content BLOB,
	JoynerBooreDistance_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	JoynerBooreDistance_used INTEGER(1) NOT NULL DEFAULT '0',
	closestFaultDistance_value DOUBLE,
	closestFaultDistance_uncertainty DOUBLE UNSIGNED,
	closestFaultDistance_lowerUncertainty DOUBLE UNSIGNED,
	closestFaultDistance_upperUncertainty DOUBLE UNSIGNED,
	closestFaultDistance_confidenceLevel DOUBLE UNSIGNED,
	closestFaultDistance_pdf_variable_content BLOB,
	closestFaultDistance_pdf_probability_content BLOB,
	closestFaultDistance_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	closestFaultDistance_used INTEGER(1) NOT NULL DEFAULT '0',
	preEventLength DOUBLE,
	postEventLength DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX EventRecordReference_recordID ON EventRecordReference(recordID);

CREATE TRIGGER EventRecordReferenceUpdate UPDATE ON EventRecordReference
BEGIN
  UPDATE EventRecordReference SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Rupture (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	width_value DOUBLE,
	width_uncertainty DOUBLE UNSIGNED,
	width_lowerUncertainty DOUBLE UNSIGNED,
	width_upperUncertainty DOUBLE UNSIGNED,
	width_confidenceLevel DOUBLE UNSIGNED,
	width_pdf_variable_content BLOB,
	width_pdf_probability_content BLOB,
	width_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	width_used INTEGER(1) NOT NULL DEFAULT '0',
	displacement_value DOUBLE,
	displacement_uncertainty DOUBLE UNSIGNED,
	displacement_lowerUncertainty DOUBLE UNSIGNED,
	displacement_upperUncertainty DOUBLE UNSIGNED,
	displacement_confidenceLevel DOUBLE UNSIGNED,
	displacement_pdf_variable_content BLOB,
	displacement_pdf_probability_content BLOB,
	displacement_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	displacement_used INTEGER(1) NOT NULL DEFAULT '0',
	riseTime_value DOUBLE,
	riseTime_uncertainty DOUBLE UNSIGNED,
	riseTime_lowerUncertainty DOUBLE UNSIGNED,
	riseTime_upperUncertainty DOUBLE UNSIGNED,
	riseTime_confidenceLevel DOUBLE UNSIGNED,
	riseTime_pdf_variable_content BLOB,
	riseTime_pdf_probability_content BLOB,
	riseTime_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	riseTime_used INTEGER(1) NOT NULL DEFAULT '0',
	vt_to_vs_value DOUBLE,
	vt_to_vs_uncertainty DOUBLE UNSIGNED,
	vt_to_vs_lowerUncertainty DOUBLE UNSIGNED,
	vt_to_vs_upperUncertainty DOUBLE UNSIGNED,
	vt_to_vs_confidenceLevel DOUBLE UNSIGNED,
	vt_to_vs_pdf_variable_content BLOB,
	vt_to_vs_pdf_probability_content BLOB,
	vt_to_vs_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	vt_to_vs_used INTEGER(1) NOT NULL DEFAULT '0',
	shallowAsperityDepth_value DOUBLE,
	shallowAsperityDepth_uncertainty DOUBLE UNSIGNED,
	shallowAsperityDepth_lowerUncertainty DOUBLE UNSIGNED,
	shallowAsperityDepth_upperUncertainty DOUBLE UNSIGNED,
	shallowAsperityDepth_confidenceLevel DOUBLE UNSIGNED,
	shallowAsperityDepth_pdf_variable_content BLOB,
	shallowAsperityDepth_pdf_probability_content BLOB,
	shallowAsperityDepth_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	shallowAsperityDepth_used INTEGER(1) NOT NULL DEFAULT '0',
	shallowAsperity INTEGER(1),
	literatureSource_title VARCHAR,
	literatureSource_firstAuthorName VARCHAR,
	literatureSource_firstAuthorForename VARCHAR,
	literatureSource_secondaryAuthors VARCHAR,
	literatureSource_doi VARCHAR,
	literatureSource_year INT UNSIGNED,
	literatureSource_in_title VARCHAR,
	literatureSource_editor VARCHAR,
	literatureSource_place VARCHAR,
	literatureSource_language VARCHAR,
	literatureSource_tome INT UNSIGNED,
	literatureSource_page_from INT UNSIGNED,
	literatureSource_page_to INT UNSIGNED,
	literatureSource_used INTEGER(1) NOT NULL DEFAULT '0',
	slipVelocity_value DOUBLE,
	slipVelocity_uncertainty DOUBLE UNSIGNED,
	slipVelocity_lowerUncertainty DOUBLE UNSIGNED,
	slipVelocity_upperUncertainty DOUBLE UNSIGNED,
	slipVelocity_confidenceLevel DOUBLE UNSIGNED,
	slipVelocity_pdf_variable_content BLOB,
	slipVelocity_pdf_probability_content BLOB,
	slipVelocity_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	slipVelocity_used INTEGER(1) NOT NULL DEFAULT '0',
	strike_value DOUBLE,
	strike_uncertainty DOUBLE UNSIGNED,
	strike_lowerUncertainty DOUBLE UNSIGNED,
	strike_upperUncertainty DOUBLE UNSIGNED,
	strike_confidenceLevel DOUBLE UNSIGNED,
	strike_pdf_variable_content BLOB,
	strike_pdf_probability_content BLOB,
	strike_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	strike_used INTEGER(1) NOT NULL DEFAULT '0',
	length_value DOUBLE,
	length_uncertainty DOUBLE UNSIGNED,
	length_lowerUncertainty DOUBLE UNSIGNED,
	length_upperUncertainty DOUBLE UNSIGNED,
	length_confidenceLevel DOUBLE UNSIGNED,
	length_pdf_variable_content BLOB,
	length_pdf_probability_content BLOB,
	length_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	length_used INTEGER(1) NOT NULL DEFAULT '0',
	area_value DOUBLE,
	area_uncertainty DOUBLE UNSIGNED,
	area_lowerUncertainty DOUBLE UNSIGNED,
	area_upperUncertainty DOUBLE UNSIGNED,
	area_confidenceLevel DOUBLE UNSIGNED,
	area_pdf_variable_content BLOB,
	area_pdf_probability_content BLOB,
	area_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	area_used INTEGER(1) NOT NULL DEFAULT '0',
	ruptureVelocity_value DOUBLE,
	ruptureVelocity_uncertainty DOUBLE UNSIGNED,
	ruptureVelocity_lowerUncertainty DOUBLE UNSIGNED,
	ruptureVelocity_upperUncertainty DOUBLE UNSIGNED,
	ruptureVelocity_confidenceLevel DOUBLE UNSIGNED,
	ruptureVelocity_pdf_variable_content BLOB,
	ruptureVelocity_pdf_probability_content BLOB,
	ruptureVelocity_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	ruptureVelocity_used INTEGER(1) NOT NULL DEFAULT '0',
	stressdrop_value DOUBLE,
	stressdrop_uncertainty DOUBLE UNSIGNED,
	stressdrop_lowerUncertainty DOUBLE UNSIGNED,
	stressdrop_upperUncertainty DOUBLE UNSIGNED,
	stressdrop_confidenceLevel DOUBLE UNSIGNED,
	stressdrop_pdf_variable_content BLOB,
	stressdrop_pdf_probability_content BLOB,
	stressdrop_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	stressdrop_used INTEGER(1) NOT NULL DEFAULT '0',
	momentReleaseTop5km_value DOUBLE,
	momentReleaseTop5km_uncertainty DOUBLE UNSIGNED,
	momentReleaseTop5km_lowerUncertainty DOUBLE UNSIGNED,
	momentReleaseTop5km_upperUncertainty DOUBLE UNSIGNED,
	momentReleaseTop5km_confidenceLevel DOUBLE UNSIGNED,
	momentReleaseTop5km_pdf_variable_content BLOB,
	momentReleaseTop5km_pdf_probability_content BLOB,
	momentReleaseTop5km_pdf_used INTEGER(1) NOT NULL DEFAULT '0',
	momentReleaseTop5km_used INTEGER(1) NOT NULL DEFAULT '0',
	fwHwIndicator VARCHAR(64),
	ruptureGeometryWKT VARCHAR,
	faultID VARCHAR NOT NULL,
	surfaceRupture_observed INTEGER(1),
	surfaceRupture_evidence VARCHAR,
	surfaceRupture_literatureSource_title VARCHAR,
	surfaceRupture_literatureSource_firstAuthorName VARCHAR,
	surfaceRupture_literatureSource_firstAuthorForename VARCHAR,
	surfaceRupture_literatureSource_secondaryAuthors VARCHAR,
	surfaceRupture_literatureSource_doi VARCHAR,
	surfaceRupture_literatureSource_year INT UNSIGNED,
	surfaceRupture_literatureSource_in_title VARCHAR,
	surfaceRupture_literatureSource_editor VARCHAR,
	surfaceRupture_literatureSource_place VARCHAR,
	surfaceRupture_literatureSource_language VARCHAR,
	surfaceRupture_literatureSource_tome INT UNSIGNED,
	surfaceRupture_literatureSource_page_from INT UNSIGNED,
	surfaceRupture_literatureSource_page_to INT UNSIGNED,
	surfaceRupture_literatureSource_used INTEGER(1) NOT NULL DEFAULT '0',
	surfaceRupture_used INTEGER(1) NOT NULL DEFAULT '0',
	centroidReference VARCHAR,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX Rupture_ruptureGeometryWKT ON Rupture(ruptureGeometryWKT);

CREATE TRIGGER RuptureUpdate UPDATE ON Rupture
BEGIN
  UPDATE Rupture SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE StrongOriginDescription (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	originID VARCHAR NOT NULL,
	waveformCount INT UNSIGNED,
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

CREATE INDEX StrongOriginDescription_originID ON StrongOriginDescription(originID);

CREATE TRIGGER StrongOriginDescriptionUpdate UPDATE ON StrongOriginDescription
BEGIN
  UPDATE StrongOriginDescription SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;
