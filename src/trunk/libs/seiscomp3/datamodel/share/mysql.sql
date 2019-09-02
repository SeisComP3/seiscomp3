DROP TABLE IF EXISTS EventDescription;
DROP TABLE IF EXISTS Comment;
DROP TABLE IF EXISTS DataUsed;
DROP TABLE IF EXISTS CompositeTime;
DROP TABLE IF EXISTS PickReference;
DROP TABLE IF EXISTS AmplitudeReference;
DROP TABLE IF EXISTS Reading;
DROP TABLE IF EXISTS MomentTensorComponentContribution;
DROP TABLE IF EXISTS MomentTensorStationContribution;
DROP TABLE IF EXISTS MomentTensorPhaseSetting;
DROP TABLE IF EXISTS MomentTensor;
DROP TABLE IF EXISTS FocalMechanism;
DROP TABLE IF EXISTS Amplitude;
DROP TABLE IF EXISTS StationMagnitudeContribution;
DROP TABLE IF EXISTS Magnitude;
DROP TABLE IF EXISTS StationMagnitude;
DROP TABLE IF EXISTS Pick;
DROP TABLE IF EXISTS OriginReference;
DROP TABLE IF EXISTS FocalMechanismReference;
DROP TABLE IF EXISTS Event;
DROP TABLE IF EXISTS Arrival;
DROP TABLE IF EXISTS Origin;
DROP TABLE IF EXISTS Parameter;
DROP TABLE IF EXISTS ParameterSet;
DROP TABLE IF EXISTS Setup;
DROP TABLE IF EXISTS ConfigStation;
DROP TABLE IF EXISTS ConfigModule;
DROP TABLE IF EXISTS QCLog;
DROP TABLE IF EXISTS WaveformQuality;
DROP TABLE IF EXISTS Outage;
DROP TABLE IF EXISTS StationReference;
DROP TABLE IF EXISTS StationGroup;
DROP TABLE IF EXISTS AuxSource;
DROP TABLE IF EXISTS AuxDevice;
DROP TABLE IF EXISTS SensorCalibration;
DROP TABLE IF EXISTS Sensor;
DROP TABLE IF EXISTS ResponsePAZ;
DROP TABLE IF EXISTS ResponsePolynomial;
DROP TABLE IF EXISTS ResponseFAP;
DROP TABLE IF EXISTS ResponseFIR;
DROP TABLE IF EXISTS ResponseIIR;
DROP TABLE IF EXISTS DataloggerCalibration;
DROP TABLE IF EXISTS Decimation;
DROP TABLE IF EXISTS Datalogger;
DROP TABLE IF EXISTS AuxStream;
DROP TABLE IF EXISTS Stream;
DROP TABLE IF EXISTS SensorLocation;
DROP TABLE IF EXISTS Station;
DROP TABLE IF EXISTS Network;
DROP TABLE IF EXISTS RouteArclink;
DROP TABLE IF EXISTS RouteSeedlink;
DROP TABLE IF EXISTS Route;
DROP TABLE IF EXISTS Access;
DROP TABLE IF EXISTS JournalEntry;
DROP TABLE IF EXISTS ArclinkUser;
DROP TABLE IF EXISTS ArclinkStatusLine;
DROP TABLE IF EXISTS ArclinkRequestLine;
DROP TABLE IF EXISTS ArclinkRequest;
DROP TABLE IF EXISTS DataSegment;
DROP TABLE IF EXISTS DataAttributeExtent;
DROP TABLE IF EXISTS DataExtent;
DROP TABLE IF EXISTS PublicObject;
DROP TABLE IF EXISTS Object;
DROP TABLE IF EXISTS Meta;

CREATE TABLE Meta (
	name CHAR(80) NOT NULL,
	value VARCHAR(255) NOT NULL,
	PRIMARY KEY(name)
) ENGINE=INNODB;

CREATE TABLE Object (
	_oid BIGINT(20) AUTO_INCREMENT,
	_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY(_oid)
) ENGINE=INNODB;

CREATE TABLE PublicObject (
	_oid BIGINT(20) NOT NULL,
	publicID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	UNIQUE(publicID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

INSERT INTO Meta(name,value) VALUES ('Schema-Version', '0.11');
INSERT INTO Meta(name,value) VALUES ('Creation-Time', CURRENT_TIMESTAMP);

INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'EventParameters');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'Config');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'QualityControl');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'Inventory');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'Routing');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'Journaling');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'ArclinkLog');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES (LAST_INSERT_ID(),'DataAvailability');

CREATE TABLE EventDescription (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	text VARCHAR(128) NOT NULL,
	type VARCHAR(64) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,type)
) ENGINE=INNODB;

CREATE TABLE Comment (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	text BLOB NOT NULL,
	id VARCHAR(255),
	start DATETIME,
	start_ms INTEGER,
	end DATETIME,
	end_ms INTEGER,
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
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,id)
) ENGINE=INNODB;

CREATE TABLE DataUsed (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	waveType VARCHAR(64) NOT NULL,
	stationCount INT UNSIGNED NOT NULL,
	componentCount INT UNSIGNED NOT NULL,
	shortestPeriod DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE CompositeTime (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	year_value INT,
	year_uncertainty INT UNSIGNED,
	year_lowerUncertainty INT UNSIGNED,
	year_upperUncertainty INT UNSIGNED,
	year_confidenceLevel DOUBLE UNSIGNED,
	year_used TINYINT(1) NOT NULL DEFAULT '0',
	month_value INT,
	month_uncertainty INT UNSIGNED,
	month_lowerUncertainty INT UNSIGNED,
	month_upperUncertainty INT UNSIGNED,
	month_confidenceLevel DOUBLE UNSIGNED,
	month_used TINYINT(1) NOT NULL DEFAULT '0',
	day_value INT,
	day_uncertainty INT UNSIGNED,
	day_lowerUncertainty INT UNSIGNED,
	day_upperUncertainty INT UNSIGNED,
	day_confidenceLevel DOUBLE UNSIGNED,
	day_used TINYINT(1) NOT NULL DEFAULT '0',
	hour_value INT,
	hour_uncertainty INT UNSIGNED,
	hour_lowerUncertainty INT UNSIGNED,
	hour_upperUncertainty INT UNSIGNED,
	hour_confidenceLevel DOUBLE UNSIGNED,
	hour_used TINYINT(1) NOT NULL DEFAULT '0',
	minute_value INT,
	minute_uncertainty INT UNSIGNED,
	minute_lowerUncertainty INT UNSIGNED,
	minute_upperUncertainty INT UNSIGNED,
	minute_confidenceLevel DOUBLE UNSIGNED,
	minute_used TINYINT(1) NOT NULL DEFAULT '0',
	second_value DOUBLE,
	second_uncertainty DOUBLE UNSIGNED,
	second_lowerUncertainty DOUBLE UNSIGNED,
	second_upperUncertainty DOUBLE UNSIGNED,
	second_confidenceLevel DOUBLE UNSIGNED,
	second_pdf_variable_content BLOB,
	second_pdf_probability_content BLOB,
	second_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	second_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE PickReference (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	pickID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(pickID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,pickID)
) ENGINE=INNODB;

CREATE TABLE AmplitudeReference (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	amplitudeID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(amplitudeID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,amplitudeID)
) ENGINE=INNODB;

CREATE TABLE Reading (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE MomentTensorComponentContribution (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	phaseCode CHAR(4) NOT NULL,
	component INT NOT NULL,
	active TINYINT(1) NOT NULL,
	weight DOUBLE NOT NULL,
	timeShift DOUBLE NOT NULL,
	dataTimeWindow BLOB NOT NULL,
	misfit DOUBLE,
	snr DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,phaseCode,component)
) ENGINE=INNODB;

CREATE TABLE MomentTensorStationContribution (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	active TINYINT(1) NOT NULL,
	waveformID_networkCode CHAR(8),
	waveformID_stationCode CHAR(8),
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	waveformID_used TINYINT(1) NOT NULL DEFAULT '0',
	weight DOUBLE,
	timeShift DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE MomentTensorPhaseSetting (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	code CHAR(4) NOT NULL,
	lowerPeriod DOUBLE NOT NULL,
	upperPeriod DOUBLE NOT NULL,
	minimumSNR DOUBLE,
	maximumTimeShift DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code)
) ENGINE=INNODB;

CREATE TABLE MomentTensor (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	derivedOriginID VARCHAR(255) NOT NULL,
	momentMagnitudeID VARCHAR(255),
	scalarMoment_value DOUBLE,
	scalarMoment_uncertainty DOUBLE UNSIGNED,
	scalarMoment_lowerUncertainty DOUBLE UNSIGNED,
	scalarMoment_upperUncertainty DOUBLE UNSIGNED,
	scalarMoment_confidenceLevel DOUBLE UNSIGNED,
	scalarMoment_pdf_variable_content BLOB,
	scalarMoment_pdf_probability_content BLOB,
	scalarMoment_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	scalarMoment_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_Mrr_value DOUBLE,
	tensor_Mrr_uncertainty DOUBLE UNSIGNED,
	tensor_Mrr_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mrr_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mrr_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mrr_pdf_variable_content BLOB,
	tensor_Mrr_pdf_probability_content BLOB,
	tensor_Mrr_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_Mtt_value DOUBLE,
	tensor_Mtt_uncertainty DOUBLE UNSIGNED,
	tensor_Mtt_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mtt_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mtt_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mtt_pdf_variable_content BLOB,
	tensor_Mtt_pdf_probability_content BLOB,
	tensor_Mtt_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_Mpp_value DOUBLE,
	tensor_Mpp_uncertainty DOUBLE UNSIGNED,
	tensor_Mpp_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mpp_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mpp_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mpp_pdf_variable_content BLOB,
	tensor_Mpp_pdf_probability_content BLOB,
	tensor_Mpp_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_Mrt_value DOUBLE,
	tensor_Mrt_uncertainty DOUBLE UNSIGNED,
	tensor_Mrt_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mrt_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mrt_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mrt_pdf_variable_content BLOB,
	tensor_Mrt_pdf_probability_content BLOB,
	tensor_Mrt_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_Mrp_value DOUBLE,
	tensor_Mrp_uncertainty DOUBLE UNSIGNED,
	tensor_Mrp_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mrp_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mrp_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mrp_pdf_variable_content BLOB,
	tensor_Mrp_pdf_probability_content BLOB,
	tensor_Mrp_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_Mtp_value DOUBLE,
	tensor_Mtp_uncertainty DOUBLE UNSIGNED,
	tensor_Mtp_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mtp_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mtp_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mtp_pdf_variable_content BLOB,
	tensor_Mtp_pdf_probability_content BLOB,
	tensor_Mtp_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	tensor_used TINYINT(1) NOT NULL DEFAULT '0',
	variance DOUBLE,
	varianceReduction DOUBLE,
	doubleCouple DOUBLE,
	clvd DOUBLE,
	iso DOUBLE,
	greensFunctionID VARCHAR(255),
	filterID VARCHAR(255),
	sourceTimeFunction_type VARCHAR(64),
	sourceTimeFunction_duration DOUBLE,
	sourceTimeFunction_riseTime DOUBLE,
	sourceTimeFunction_decayTime DOUBLE,
	sourceTimeFunction_used TINYINT(1) NOT NULL DEFAULT '0',
	methodID VARCHAR(255),
	method VARCHAR(64),
	status VARCHAR(64),
	cmtName VARCHAR(80),
	cmtVersion VARCHAR(64),
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
	INDEX(_parent_oid),
	INDEX(derivedOriginID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE FocalMechanism (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	triggeringOriginID VARCHAR(255),
	nodalPlanes_nodalPlane1_strike_value DOUBLE,
	nodalPlanes_nodalPlane1_strike_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_pdf_variable_content BLOB,
	nodalPlanes_nodalPlane1_strike_pdf_probability_content BLOB,
	nodalPlanes_nodalPlane1_strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane1_dip_value DOUBLE,
	nodalPlanes_nodalPlane1_dip_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_pdf_variable_content BLOB,
	nodalPlanes_nodalPlane1_dip_pdf_probability_content BLOB,
	nodalPlanes_nodalPlane1_dip_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane1_rake_value DOUBLE,
	nodalPlanes_nodalPlane1_rake_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_pdf_variable_content BLOB,
	nodalPlanes_nodalPlane1_rake_pdf_probability_content BLOB,
	nodalPlanes_nodalPlane1_rake_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane1_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane2_strike_value DOUBLE,
	nodalPlanes_nodalPlane2_strike_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_pdf_variable_content BLOB,
	nodalPlanes_nodalPlane2_strike_pdf_probability_content BLOB,
	nodalPlanes_nodalPlane2_strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane2_dip_value DOUBLE,
	nodalPlanes_nodalPlane2_dip_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_pdf_variable_content BLOB,
	nodalPlanes_nodalPlane2_dip_pdf_probability_content BLOB,
	nodalPlanes_nodalPlane2_dip_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane2_rake_value DOUBLE,
	nodalPlanes_nodalPlane2_rake_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_pdf_variable_content BLOB,
	nodalPlanes_nodalPlane2_rake_pdf_probability_content BLOB,
	nodalPlanes_nodalPlane2_rake_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane2_used TINYINT(1) NOT NULL DEFAULT '0',
	nodalPlanes_preferredPlane INT,
	nodalPlanes_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_tAxis_azimuth_value DOUBLE,
	principalAxes_tAxis_azimuth_uncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_pdf_variable_content BLOB,
	principalAxes_tAxis_azimuth_pdf_probability_content BLOB,
	principalAxes_tAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_tAxis_plunge_value DOUBLE,
	principalAxes_tAxis_plunge_uncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_pdf_variable_content BLOB,
	principalAxes_tAxis_plunge_pdf_probability_content BLOB,
	principalAxes_tAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_tAxis_length_value DOUBLE,
	principalAxes_tAxis_length_uncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_length_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_length_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_length_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_tAxis_length_pdf_variable_content BLOB,
	principalAxes_tAxis_length_pdf_probability_content BLOB,
	principalAxes_tAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_pAxis_azimuth_value DOUBLE,
	principalAxes_pAxis_azimuth_uncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_pdf_variable_content BLOB,
	principalAxes_pAxis_azimuth_pdf_probability_content BLOB,
	principalAxes_pAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_pAxis_plunge_value DOUBLE,
	principalAxes_pAxis_plunge_uncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_pdf_variable_content BLOB,
	principalAxes_pAxis_plunge_pdf_probability_content BLOB,
	principalAxes_pAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_pAxis_length_value DOUBLE,
	principalAxes_pAxis_length_uncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_length_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_length_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_length_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_pAxis_length_pdf_variable_content BLOB,
	principalAxes_pAxis_length_pdf_probability_content BLOB,
	principalAxes_pAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_nAxis_azimuth_value DOUBLE,
	principalAxes_nAxis_azimuth_uncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_pdf_variable_content BLOB,
	principalAxes_nAxis_azimuth_pdf_probability_content BLOB,
	principalAxes_nAxis_azimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_nAxis_plunge_value DOUBLE,
	principalAxes_nAxis_plunge_uncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_pdf_variable_content BLOB,
	principalAxes_nAxis_plunge_pdf_probability_content BLOB,
	principalAxes_nAxis_plunge_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_nAxis_length_value DOUBLE,
	principalAxes_nAxis_length_uncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_length_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_length_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_length_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_length_pdf_variable_content BLOB,
	principalAxes_nAxis_length_pdf_probability_content BLOB,
	principalAxes_nAxis_length_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_nAxis_used TINYINT(1) NOT NULL DEFAULT '0',
	principalAxes_used TINYINT(1) NOT NULL DEFAULT '0',
	azimuthalGap DOUBLE UNSIGNED,
	stationPolarityCount INT UNSIGNED,
	misfit DOUBLE UNSIGNED,
	stationDistributionRatio DOUBLE UNSIGNED,
	methodID VARCHAR(255),
	evaluationMode VARCHAR(64),
	evaluationStatus VARCHAR(64),
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
	INDEX(_parent_oid),
	INDEX(triggeringOriginID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE Amplitude (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	type CHAR(16) NOT NULL,
	amplitude_value DOUBLE,
	amplitude_uncertainty DOUBLE UNSIGNED,
	amplitude_lowerUncertainty DOUBLE UNSIGNED,
	amplitude_upperUncertainty DOUBLE UNSIGNED,
	amplitude_confidenceLevel DOUBLE UNSIGNED,
	amplitude_pdf_variable_content BLOB,
	amplitude_pdf_probability_content BLOB,
	amplitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	amplitude_used TINYINT(1) NOT NULL DEFAULT '0',
	timeWindow_reference DATETIME,
	timeWindow_reference_ms INTEGER,
	timeWindow_begin DOUBLE,
	timeWindow_end DOUBLE,
	timeWindow_used TINYINT(1) NOT NULL DEFAULT '0',
	period_value DOUBLE,
	period_uncertainty DOUBLE UNSIGNED,
	period_lowerUncertainty DOUBLE UNSIGNED,
	period_upperUncertainty DOUBLE UNSIGNED,
	period_confidenceLevel DOUBLE UNSIGNED,
	period_pdf_variable_content BLOB,
	period_pdf_probability_content BLOB,
	period_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	period_used TINYINT(1) NOT NULL DEFAULT '0',
	snr DOUBLE,
	unit VARCHAR(255),
	pickID VARCHAR(255),
	waveformID_networkCode CHAR(8),
	waveformID_stationCode CHAR(8),
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	waveformID_used TINYINT(1) NOT NULL DEFAULT '0',
	filterID VARCHAR(255),
	methodID VARCHAR(255),
	scalingTime_value DATETIME,
	scalingTime_value_ms INTEGER,
	scalingTime_uncertainty DOUBLE UNSIGNED,
	scalingTime_lowerUncertainty DOUBLE UNSIGNED,
	scalingTime_upperUncertainty DOUBLE UNSIGNED,
	scalingTime_confidenceLevel DOUBLE UNSIGNED,
	scalingTime_pdf_variable_content BLOB,
	scalingTime_pdf_probability_content BLOB,
	scalingTime_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	scalingTime_used TINYINT(1) NOT NULL DEFAULT '0',
	magnitudeHint CHAR(16),
	evaluationMode VARCHAR(64),
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
	INDEX(_parent_oid),
	INDEX(pickID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE INDEX Amplitude_timeWindow_reference ON Amplitude(timeWindow_reference,timeWindow_reference_ms);


CREATE TABLE StationMagnitudeContribution (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	stationMagnitudeID VARCHAR(255) NOT NULL,
	residual DOUBLE,
	weight DOUBLE UNSIGNED,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(stationMagnitudeID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,stationMagnitudeID)
) ENGINE=INNODB;

CREATE TABLE Magnitude (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	magnitude_value DOUBLE NOT NULL,
	magnitude_uncertainty DOUBLE UNSIGNED,
	magnitude_lowerUncertainty DOUBLE UNSIGNED,
	magnitude_upperUncertainty DOUBLE UNSIGNED,
	magnitude_confidenceLevel DOUBLE UNSIGNED,
	magnitude_pdf_variable_content BLOB,
	magnitude_pdf_probability_content BLOB,
	magnitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	type CHAR(16) BINARY,
	originID VARCHAR(255),
	methodID VARCHAR(255),
	stationCount INT UNSIGNED,
	azimuthalGap DOUBLE UNSIGNED,
	evaluationStatus VARCHAR(64),
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
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE StationMagnitude (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	originID VARCHAR(255),
	magnitude_value DOUBLE NOT NULL,
	magnitude_uncertainty DOUBLE UNSIGNED,
	magnitude_lowerUncertainty DOUBLE UNSIGNED,
	magnitude_upperUncertainty DOUBLE UNSIGNED,
	magnitude_confidenceLevel DOUBLE UNSIGNED,
	magnitude_pdf_variable_content BLOB,
	magnitude_pdf_probability_content BLOB,
	magnitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	type CHAR(16) BINARY,
	amplitudeID VARCHAR(255),
	methodID VARCHAR(255),
	waveformID_networkCode CHAR(8),
	waveformID_stationCode CHAR(8),
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	waveformID_used TINYINT(1) NOT NULL DEFAULT '0',
	passedQC TINYINT(1),
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
	INDEX(_parent_oid),
	INDEX(amplitudeID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE Pick (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	time_value DATETIME NOT NULL,
	time_value_ms INTEGER NOT NULL,
	time_uncertainty DOUBLE UNSIGNED,
	time_lowerUncertainty DOUBLE UNSIGNED,
	time_upperUncertainty DOUBLE UNSIGNED,
	time_confidenceLevel DOUBLE UNSIGNED,
	time_pdf_variable_content BLOB,
	time_pdf_probability_content BLOB,
	time_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	waveformID_networkCode CHAR(8) NOT NULL,
	waveformID_stationCode CHAR(8) NOT NULL,
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	filterID VARCHAR(255),
	methodID VARCHAR(255),
	horizontalSlowness_value DOUBLE,
	horizontalSlowness_uncertainty DOUBLE UNSIGNED,
	horizontalSlowness_lowerUncertainty DOUBLE UNSIGNED,
	horizontalSlowness_upperUncertainty DOUBLE UNSIGNED,
	horizontalSlowness_confidenceLevel DOUBLE UNSIGNED,
	horizontalSlowness_pdf_variable_content BLOB,
	horizontalSlowness_pdf_probability_content BLOB,
	horizontalSlowness_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	horizontalSlowness_used TINYINT(1) NOT NULL DEFAULT '0',
	backazimuth_value DOUBLE,
	backazimuth_uncertainty DOUBLE UNSIGNED,
	backazimuth_lowerUncertainty DOUBLE UNSIGNED,
	backazimuth_upperUncertainty DOUBLE UNSIGNED,
	backazimuth_confidenceLevel DOUBLE UNSIGNED,
	backazimuth_pdf_variable_content BLOB,
	backazimuth_pdf_probability_content BLOB,
	backazimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	backazimuth_used TINYINT(1) NOT NULL DEFAULT '0',
	slownessMethodID VARCHAR(255),
	onset VARCHAR(64),
	phaseHint_code CHAR(32),
	phaseHint_used TINYINT(1) NOT NULL DEFAULT '0',
	polarity VARCHAR(64),
	evaluationMode VARCHAR(64),
	evaluationStatus VARCHAR(64),
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
	INDEX(_parent_oid),
	INDEX(time_value,time_value_ms),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE OriginReference (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	originID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(originID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,originID)
) ENGINE=INNODB;

CREATE TABLE FocalMechanismReference (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	focalMechanismID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(focalMechanismID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,focalMechanismID)
) ENGINE=INNODB;

CREATE TABLE Event (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	preferredOriginID VARCHAR(255),
	preferredMagnitudeID VARCHAR(255),
	preferredFocalMechanismID VARCHAR(255),
	type VARCHAR(64),
	typeCertainty VARCHAR(64),
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
	INDEX(_parent_oid),
	INDEX(preferredOriginID),
	INDEX(preferredMagnitudeID),
	INDEX(preferredFocalMechanismID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE Arrival (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	pickID VARCHAR(255) NOT NULL,
	phase_code CHAR(32) NOT NULL,
	timeCorrection DOUBLE,
	azimuth DOUBLE,
	distance DOUBLE UNSIGNED,
	takeOffAngle DOUBLE,
	timeResidual DOUBLE,
	horizontalSlownessResidual DOUBLE,
	backazimuthResidual DOUBLE,
	timeUsed TINYINT(1),
	horizontalSlownessUsed TINYINT(1),
	backazimuthUsed TINYINT(1),
	weight DOUBLE UNSIGNED,
	earthModelID VARCHAR(255),
	preliminary TINYINT(1),
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
	INDEX(_parent_oid),
	INDEX(pickID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,pickID)
) ENGINE=INNODB;

CREATE TABLE Origin (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	time_value DATETIME NOT NULL,
	time_value_ms INTEGER NOT NULL,
	time_uncertainty DOUBLE UNSIGNED,
	time_lowerUncertainty DOUBLE UNSIGNED,
	time_upperUncertainty DOUBLE UNSIGNED,
	time_confidenceLevel DOUBLE UNSIGNED,
	time_pdf_variable_content BLOB,
	time_pdf_probability_content BLOB,
	time_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	latitude_value DOUBLE NOT NULL,
	latitude_uncertainty DOUBLE UNSIGNED,
	latitude_lowerUncertainty DOUBLE UNSIGNED,
	latitude_upperUncertainty DOUBLE UNSIGNED,
	latitude_confidenceLevel DOUBLE UNSIGNED,
	latitude_pdf_variable_content BLOB,
	latitude_pdf_probability_content BLOB,
	latitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	longitude_value DOUBLE NOT NULL,
	longitude_uncertainty DOUBLE UNSIGNED,
	longitude_lowerUncertainty DOUBLE UNSIGNED,
	longitude_upperUncertainty DOUBLE UNSIGNED,
	longitude_confidenceLevel DOUBLE UNSIGNED,
	longitude_pdf_variable_content BLOB,
	longitude_pdf_probability_content BLOB,
	longitude_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	depth_value DOUBLE,
	depth_uncertainty DOUBLE UNSIGNED,
	depth_lowerUncertainty DOUBLE UNSIGNED,
	depth_upperUncertainty DOUBLE UNSIGNED,
	depth_confidenceLevel DOUBLE UNSIGNED,
	depth_pdf_variable_content BLOB,
	depth_pdf_probability_content BLOB,
	depth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
	depth_used TINYINT(1) NOT NULL DEFAULT '0',
	depthType VARCHAR(64),
	timeFixed TINYINT(1),
	epicenterFixed TINYINT(1),
	referenceSystemID VARCHAR(255),
	methodID VARCHAR(255),
	earthModelID VARCHAR(255),
	quality_associatedPhaseCount INT UNSIGNED,
	quality_usedPhaseCount INT UNSIGNED,
	quality_associatedStationCount INT UNSIGNED,
	quality_usedStationCount INT UNSIGNED,
	quality_depthPhaseCount INT UNSIGNED,
	quality_standardError DOUBLE UNSIGNED,
	quality_azimuthalGap DOUBLE UNSIGNED,
	quality_secondaryAzimuthalGap DOUBLE UNSIGNED,
	quality_groundTruthLevel VARCHAR(16),
	quality_maximumDistance DOUBLE UNSIGNED,
	quality_minimumDistance DOUBLE UNSIGNED,
	quality_medianDistance DOUBLE UNSIGNED,
	quality_used TINYINT(1) NOT NULL DEFAULT '0',
	uncertainty_horizontalUncertainty DOUBLE UNSIGNED,
	uncertainty_minHorizontalUncertainty DOUBLE UNSIGNED,
	uncertainty_maxHorizontalUncertainty DOUBLE UNSIGNED,
	uncertainty_azimuthMaxHorizontalUncertainty DOUBLE UNSIGNED,
	uncertainty_confidenceEllipsoid_semiMajorAxisLength DOUBLE,
	uncertainty_confidenceEllipsoid_semiMinorAxisLength DOUBLE,
	uncertainty_confidenceEllipsoid_semiIntermediateAxisLength DOUBLE,
	uncertainty_confidenceEllipsoid_majorAxisPlunge DOUBLE,
	uncertainty_confidenceEllipsoid_majorAxisAzimuth DOUBLE,
	uncertainty_confidenceEllipsoid_majorAxisRotation DOUBLE,
	uncertainty_confidenceEllipsoid_used TINYINT(1) NOT NULL DEFAULT '0',
	uncertainty_preferredDescription VARCHAR(64),
	uncertainty_used TINYINT(1) NOT NULL DEFAULT '0',
	type VARCHAR(64),
	evaluationMode VARCHAR(64),
	evaluationStatus VARCHAR(64),
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
	INDEX(_parent_oid),
	INDEX(time_value,time_value_ms),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE Parameter (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255) NOT NULL,
	value BLOB,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE ParameterSet (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	baseID VARCHAR(255),
	moduleID VARCHAR(255),
	created DATETIME,
	created_ms INTEGER,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(baseID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE Setup (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(20),
	parameterSetID VARCHAR(255),
	enabled TINYINT(1) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(parameterSetID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE ConfigStation (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	networkCode CHAR(8) NOT NULL,
	stationCode CHAR(8) NOT NULL,
	enabled TINYINT(1) NOT NULL,
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
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,networkCode,stationCode)
) ENGINE=INNODB;

CREATE TABLE ConfigModule (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(20) NOT NULL,
	parameterSetID VARCHAR(255),
	enabled TINYINT(1) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(parameterSetID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE QCLog (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR(8) NOT NULL,
	waveformID_stationCode CHAR(8) NOT NULL,
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	creatorID VARCHAR(255) NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	message BLOB NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,start,start_ms,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI)
) ENGINE=INNODB;

CREATE TABLE WaveformQuality (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR(8) NOT NULL,
	waveformID_stationCode CHAR(8) NOT NULL,
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	creatorID VARCHAR(255) NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	type CHAR(80) BINARY NOT NULL,
	parameter CHAR(80) BINARY NOT NULL,
	value DOUBLE NOT NULL,
	lowerUncertainty DOUBLE,
	upperUncertainty DOUBLE,
	windowLength DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(start,start_ms),
	INDEX(end,end_ms),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,start,start_ms,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI,type,parameter)
) ENGINE=INNODB;

CREATE TABLE Outage (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR(8) NOT NULL,
	waveformID_stationCode CHAR(8) NOT NULL,
	waveformID_locationCode CHAR(8),
	waveformID_channelCode CHAR(8),
	waveformID_resourceURI VARCHAR(255),
	creatorID VARCHAR(255) NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE StationReference (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	stationID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(stationID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,stationID)
) ENGINE=INNODB;

CREATE TABLE StationGroup (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	type VARCHAR(64),
	code CHAR(20),
	start DATETIME,
	start_ms INTEGER,
	end DATETIME,
	end_ms INTEGER,
	description VARCHAR(255),
	latitude DOUBLE,
	longitude DOUBLE,
	elevation DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code)
) ENGINE=INNODB;

CREATE TABLE AuxSource (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255) NOT NULL,
	description VARCHAR(255),
	unit VARCHAR(20),
	conversion VARCHAR(80),
	sampleRateNumerator INT UNSIGNED,
	sampleRateDenominator INT UNSIGNED,
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE AuxDevice (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255) NOT NULL,
	description VARCHAR(255),
	model VARCHAR(80),
	manufacturer VARCHAR(50),
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE SensorCalibration (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	serialNumber VARCHAR(80) NOT NULL,
	channel INT UNSIGNED NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,serialNumber,channel,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE Sensor (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255) NOT NULL,
	description VARCHAR(255),
	model VARCHAR(80),
	manufacturer VARCHAR(50),
	type VARCHAR(10),
	unit VARCHAR(20),
	lowFrequency DOUBLE UNSIGNED,
	highFrequency DOUBLE UNSIGNED,
	response VARCHAR(255),
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE ResponsePAZ (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255),
	type CHAR(1),
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	normalizationFactor DOUBLE UNSIGNED,
	normalizationFrequency DOUBLE UNSIGNED,
	numberOfZeros TINYINT UNSIGNED,
	numberOfPoles TINYINT UNSIGNED,
	zeros_content BLOB,
	zeros_used TINYINT(1) NOT NULL DEFAULT '0',
	poles_content BLOB,
	poles_used TINYINT(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	decimationFactor SMALLINT UNSIGNED,
	delay DOUBLE UNSIGNED,
	correction DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE ResponsePolynomial (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255),
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	frequencyUnit CHAR(1),
	approximationType CHAR(1),
	approximationLowerBound DOUBLE,
	approximationUpperBound DOUBLE,
	approximationError DOUBLE UNSIGNED,
	numberOfCoefficients SMALLINT UNSIGNED,
	coefficients_content BLOB,
	coefficients_used TINYINT(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE ResponseFAP (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255),
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	numberOfTuples SMALLINT UNSIGNED,
	tuples_content BLOB,
	tuples_used TINYINT(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE ResponseFIR (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255),
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	decimationFactor SMALLINT UNSIGNED,
	delay DOUBLE UNSIGNED,
	correction DOUBLE,
	numberOfCoefficients SMALLINT UNSIGNED,
	symmetry CHAR(1),
	coefficients_content BLOB,
	coefficients_used TINYINT(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE ResponseIIR (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
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

CREATE TABLE DataloggerCalibration (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	serialNumber VARCHAR(80) NOT NULL,
	channel INT UNSIGNED NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,serialNumber,channel,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE Decimation (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	sampleRateNumerator INT UNSIGNED NOT NULL,
	sampleRateDenominator INT UNSIGNED NOT NULL,
	analogueFilterChain_content BLOB,
	analogueFilterChain_used TINYINT(1) NOT NULL DEFAULT '0',
	digitalFilterChain_content BLOB,
	digitalFilterChain_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,sampleRateNumerator,sampleRateDenominator)
) ENGINE=INNODB;

CREATE TABLE Datalogger (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(255),
	description VARCHAR(255),
	digitizerModel VARCHAR(80),
	digitizerManufacturer VARCHAR(50),
	recorderModel VARCHAR(80),
	recorderManufacturer VARCHAR(50),
	clockModel VARCHAR(80),
	clockManufacturer VARCHAR(50),
	clockType VARCHAR(10),
	gain DOUBLE,
	maxClockDrift DOUBLE UNSIGNED,
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name)
) ENGINE=INNODB;

CREATE TABLE AuxStream (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	code CHAR(3) NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	device VARCHAR(255),
	deviceSerialNumber VARCHAR(80),
	source VARCHAR(80),
	format VARCHAR(50),
	flags VARCHAR(20),
	restricted TINYINT(1),
	shared TINYINT(1),
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE Stream (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	code CHAR(3) NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	datalogger VARCHAR(255),
	dataloggerSerialNumber VARCHAR(80),
	dataloggerChannel INT UNSIGNED,
	sensor VARCHAR(255),
	sensorSerialNumber VARCHAR(80),
	sensorChannel INT UNSIGNED,
	clockSerialNumber VARCHAR(80),
	sampleRateNumerator INT UNSIGNED,
	sampleRateDenominator INT UNSIGNED,
	depth DOUBLE,
	azimuth DOUBLE,
	dip DOUBLE,
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	gainUnit CHAR(20),
	format VARCHAR(50),
	flags VARCHAR(20),
	restricted TINYINT(1),
	shared TINYINT(1),
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE SensorLocation (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	code CHAR(8) NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	latitude DOUBLE,
	longitude DOUBLE,
	elevation DOUBLE,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE Station (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	code CHAR(8) NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	description VARCHAR(255),
	latitude DOUBLE,
	longitude DOUBLE,
	elevation DOUBLE,
	place VARCHAR(80),
	country VARCHAR(50),
	affiliation VARCHAR(50),
	type VARCHAR(50),
	archive VARCHAR(20),
	archiveNetworkCode CHAR(8),
	restricted TINYINT(1),
	shared TINYINT(1),
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE Network (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	code CHAR(8) NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	description VARCHAR(255),
	institutions VARCHAR(100),
	region VARCHAR(100),
	type VARCHAR(50),
	netClass CHAR(1),
	archive VARCHAR(20),
	restricted TINYINT(1),
	shared TINYINT(1),
	remark_content BLOB,
	remark_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,code,start,start_ms)
) ENGINE=INNODB;

CREATE TABLE RouteArclink (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	address VARCHAR(50) NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	priority TINYINT UNSIGNED,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,address,start)
) ENGINE=INNODB;

CREATE TABLE RouteSeedlink (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	address VARCHAR(50) NOT NULL,
	priority TINYINT UNSIGNED,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,address)
) ENGINE=INNODB;

CREATE TABLE Route (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	networkCode CHAR(8) NOT NULL,
	stationCode CHAR(8) NOT NULL,
	locationCode CHAR(8) NOT NULL,
	streamCode CHAR(8) NOT NULL,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,networkCode,stationCode,locationCode,streamCode)
) ENGINE=INNODB;

CREATE TABLE Access (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	networkCode CHAR(8) NOT NULL,
	stationCode CHAR(8) NOT NULL,
	locationCode CHAR(8) NOT NULL,
	streamCode CHAR(8) NOT NULL,
	user VARCHAR(50) NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,networkCode,stationCode,locationCode,streamCode,user,start)
) ENGINE=INNODB;

CREATE TABLE JournalEntry (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	created DATETIME,
	created_ms INTEGER,
	objectID VARCHAR(255) NOT NULL,
	sender VARCHAR(80) NOT NULL,
	action VARCHAR(160) NOT NULL,
	parameters VARCHAR(160),
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	INDEX(objectID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
) ENGINE=INNODB;

CREATE TABLE ArclinkUser (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	name VARCHAR(80) NOT NULL,
	email VARCHAR(80),
	password VARCHAR(80),
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,name,email)
) ENGINE=INNODB;

CREATE TABLE ArclinkStatusLine (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	type VARCHAR(80) NOT NULL,
	status VARCHAR(160) NOT NULL,
	size INT UNSIGNED,
	message VARCHAR(160),
	volumeID VARCHAR(80),
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,volumeID,type,status)
) ENGINE=INNODB;

CREATE TABLE ArclinkRequestLine (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	streamID_networkCode CHAR(8) NOT NULL,
	streamID_stationCode CHAR(8) NOT NULL,
	streamID_locationCode CHAR(8),
	streamID_channelCode CHAR(8),
	streamID_resourceURI VARCHAR(255),
	restricted TINYINT(1),
	shared TINYINT(1),
	netClass CHAR(1),
	constraints VARCHAR(160),
	status_type VARCHAR(80) NOT NULL,
	status_status VARCHAR(160) NOT NULL,
	status_size INT UNSIGNED,
	status_message VARCHAR(160),
	status_volumeID VARCHAR(80),
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,start,start_ms,end,end_ms,streamID_networkCode,streamID_stationCode,streamID_locationCode,streamID_channelCode,streamID_resourceURI)
) ENGINE=INNODB;

CREATE TABLE ArclinkRequest (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	requestID VARCHAR(255) NOT NULL,
	userID VARCHAR(80) NOT NULL,
	userIP VARCHAR(16),
	clientID VARCHAR(80),
	clientIP VARCHAR(16),
	type VARCHAR(80) NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	status VARCHAR(80) NOT NULL,
	message VARCHAR(160),
	label VARCHAR(80),
	header VARCHAR(160),
	summary_okLineCount INT UNSIGNED,
	summary_totalLineCount INT UNSIGNED,
	summary_averageTimeWindow INT UNSIGNED,
	summary_used TINYINT(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	INDEX(_parent_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE KEY composite_index (_parent_oid,created,created_ms,requestID,userID)
) ENGINE=INNODB;

CREATE TABLE DataSegment (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
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

CREATE TABLE DataAttributeExtent (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
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

CREATE TABLE DataExtent (
	_oid BIGINT(20) NOT NULL,
	_parent_oid BIGINT(20) NOT NULL,
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
