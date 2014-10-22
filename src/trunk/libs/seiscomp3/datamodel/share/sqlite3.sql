DROP TABLE EventDescription;
DROP TABLE Comment;
DROP TABLE DataUsed;
DROP TABLE CompositeTime;
DROP TABLE PickReference;
DROP TABLE AmplitudeReference;
DROP TABLE Reading;
DROP TABLE MomentTensorComponentContribution;
DROP TABLE MomentTensorStationContribution;
DROP TABLE MomentTensorPhaseSetting;
DROP TABLE MomentTensor;
DROP TABLE FocalMechanism;
DROP TABLE Amplitude;
DROP TABLE StationMagnitudeContribution;
DROP TABLE Magnitude;
DROP TABLE StationMagnitude;
DROP TABLE Pick;
DROP TABLE OriginReference;
DROP TABLE FocalMechanismReference;
DROP TABLE Event;
DROP TABLE Arrival;
DROP TABLE Origin;
DROP TABLE Parameter;
DROP TABLE ParameterSet;
DROP TABLE Setup;
DROP TABLE ConfigStation;
DROP TABLE ConfigModule;
DROP TABLE QCLog;
DROP TABLE WaveformQuality;
DROP TABLE Outage;
DROP TABLE StationReference;
DROP TABLE StationGroup;
DROP TABLE AuxSource;
DROP TABLE AuxDevice;
DROP TABLE SensorCalibration;
DROP TABLE Sensor;
DROP TABLE ResponsePAZ;
DROP TABLE ResponsePolynomial;
DROP TABLE DataloggerCalibration;
DROP TABLE Decimation;
DROP TABLE Datalogger;
DROP TABLE ResponseFIR;
DROP TABLE AuxStream;
DROP TABLE Stream;
DROP TABLE SensorLocation;
DROP TABLE Station;
DROP TABLE Network;
DROP TABLE RouteArclink;
DROP TABLE RouteSeedlink;
DROP TABLE Route;
DROP TABLE Access;
DROP TABLE JournalEntry;
DROP TABLE ArclinkUser;
DROP TABLE ArclinkStatusLine;
DROP TABLE ArclinkRequestLine;
DROP TABLE ArclinkRequest;
DROP TABLE PublicObject;
DROP TABLE Object;
DROP TABLE Meta;

CREATE TABLE Meta (
	name CHAR NOT NULL,
	value VARCHAR NOT NULL,
	PRIMARY KEY(name)
);


CREATE TABLE Object (
	_oid INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE PublicObject (
	_oid INTEGER NOT NULL,
	publicID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	UNIQUE(publicID),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

INSERT INTO Meta(name,value) VALUES ('Schema-Version', '0.7');
INSERT INTO Meta(name,value) VALUES ('Creation-Time', CURRENT_TIMESTAMP);

INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'EventParameters');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'Config');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'QualityControl');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'Inventory');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'Routing');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'Journaling');
INSERT INTO Object(_oid) VALUES (NULL);
INSERT INTO PublicObject(_oid,publicID) VALUES ((SELECT MAX(_oid) FROM Object),'ArclinkLog');

CREATE TABLE EventDescription (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	text VARCHAR NOT NULL,
	type VARCHAR(64) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,type)
);


CREATE TRIGGER EventDescriptionUpdate UPDATE ON EventDescription
BEGIN
  UPDATE EventDescription SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Comment (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	text BLOB NOT NULL,
	id VARCHAR,
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
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,id)
);


CREATE TRIGGER CommentUpdate UPDATE ON Comment
BEGIN
  UPDATE Comment SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE DataUsed (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	waveType VARCHAR(64) NOT NULL,
	stationCount INT UNSIGNED NOT NULL,
	componentCount INT UNSIGNED NOT NULL,
	shortestPeriod DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER DataUsedUpdate UPDATE ON DataUsed
BEGIN
  UPDATE DataUsed SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE CompositeTime (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	year_value INT,
	year_uncertainty INT UNSIGNED,
	year_lowerUncertainty INT UNSIGNED,
	year_upperUncertainty INT UNSIGNED,
	year_confidenceLevel DOUBLE UNSIGNED,
	year_used INTEGER(1) NOT NULL DEFAULT '0',
	month_value INT,
	month_uncertainty INT UNSIGNED,
	month_lowerUncertainty INT UNSIGNED,
	month_upperUncertainty INT UNSIGNED,
	month_confidenceLevel DOUBLE UNSIGNED,
	month_used INTEGER(1) NOT NULL DEFAULT '0',
	day_value INT,
	day_uncertainty INT UNSIGNED,
	day_lowerUncertainty INT UNSIGNED,
	day_upperUncertainty INT UNSIGNED,
	day_confidenceLevel DOUBLE UNSIGNED,
	day_used INTEGER(1) NOT NULL DEFAULT '0',
	hour_value INT,
	hour_uncertainty INT UNSIGNED,
	hour_lowerUncertainty INT UNSIGNED,
	hour_upperUncertainty INT UNSIGNED,
	hour_confidenceLevel DOUBLE UNSIGNED,
	hour_used INTEGER(1) NOT NULL DEFAULT '0',
	minute_value INT,
	minute_uncertainty INT UNSIGNED,
	minute_lowerUncertainty INT UNSIGNED,
	minute_upperUncertainty INT UNSIGNED,
	minute_confidenceLevel DOUBLE UNSIGNED,
	minute_used INTEGER(1) NOT NULL DEFAULT '0',
	second_value DOUBLE,
	second_uncertainty DOUBLE UNSIGNED,
	second_lowerUncertainty DOUBLE UNSIGNED,
	second_upperUncertainty DOUBLE UNSIGNED,
	second_confidenceLevel DOUBLE UNSIGNED,
	second_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER CompositeTimeUpdate UPDATE ON CompositeTime
BEGIN
  UPDATE CompositeTime SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE PickReference (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	pickID VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,pickID)
);

CREATE INDEX PickReference_pickID ON PickReference(pickID);

CREATE TRIGGER PickReferenceUpdate UPDATE ON PickReference
BEGIN
  UPDATE PickReference SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE AmplitudeReference (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	amplitudeID VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,amplitudeID)
);

CREATE INDEX AmplitudeReference_amplitudeID ON AmplitudeReference(amplitudeID);

CREATE TRIGGER AmplitudeReferenceUpdate UPDATE ON AmplitudeReference
BEGIN
  UPDATE AmplitudeReference SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Reading (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER ReadingUpdate UPDATE ON Reading
BEGIN
  UPDATE Reading SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE MomentTensorComponentContribution (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	phaseCode CHAR NOT NULL,
	component INT NOT NULL,
	active INTEGER(1) NOT NULL,
	weight DOUBLE NOT NULL,
	timeShift DOUBLE NOT NULL,
	dataTimeWindow BLOB NOT NULL,
	misfit DOUBLE,
	snr DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,phaseCode,component)
);


CREATE TRIGGER MomentTensorComponentContributionUpdate UPDATE ON MomentTensorComponentContribution
BEGIN
  UPDATE MomentTensorComponentContribution SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE MomentTensorStationContribution (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	active INTEGER(1) NOT NULL,
	waveformID_networkCode CHAR,
	waveformID_stationCode CHAR,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	waveformID_used INTEGER(1) NOT NULL DEFAULT '0',
	weight DOUBLE,
	timeShift DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER MomentTensorStationContributionUpdate UPDATE ON MomentTensorStationContribution
BEGIN
  UPDATE MomentTensorStationContribution SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE MomentTensorPhaseSetting (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	code CHAR NOT NULL,
	lowerPeriod DOUBLE NOT NULL,
	upperPeriod DOUBLE NOT NULL,
	minimumSNR DOUBLE,
	maximumTimeShift DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code)
);


CREATE TRIGGER MomentTensorPhaseSettingUpdate UPDATE ON MomentTensorPhaseSetting
BEGIN
  UPDATE MomentTensorPhaseSetting SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE MomentTensor (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	derivedOriginID VARCHAR NOT NULL,
	momentMagnitudeID VARCHAR,
	scalarMoment_value DOUBLE,
	scalarMoment_uncertainty DOUBLE UNSIGNED,
	scalarMoment_lowerUncertainty DOUBLE UNSIGNED,
	scalarMoment_upperUncertainty DOUBLE UNSIGNED,
	scalarMoment_confidenceLevel DOUBLE UNSIGNED,
	scalarMoment_used INTEGER(1) NOT NULL DEFAULT '0',
	tensor_Mrr_value DOUBLE,
	tensor_Mrr_uncertainty DOUBLE UNSIGNED,
	tensor_Mrr_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mrr_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mrr_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mtt_value DOUBLE,
	tensor_Mtt_uncertainty DOUBLE UNSIGNED,
	tensor_Mtt_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mtt_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mtt_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mpp_value DOUBLE,
	tensor_Mpp_uncertainty DOUBLE UNSIGNED,
	tensor_Mpp_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mpp_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mpp_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mrt_value DOUBLE,
	tensor_Mrt_uncertainty DOUBLE UNSIGNED,
	tensor_Mrt_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mrt_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mrt_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mrp_value DOUBLE,
	tensor_Mrp_uncertainty DOUBLE UNSIGNED,
	tensor_Mrp_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mrp_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mrp_confidenceLevel DOUBLE UNSIGNED,
	tensor_Mtp_value DOUBLE,
	tensor_Mtp_uncertainty DOUBLE UNSIGNED,
	tensor_Mtp_lowerUncertainty DOUBLE UNSIGNED,
	tensor_Mtp_upperUncertainty DOUBLE UNSIGNED,
	tensor_Mtp_confidenceLevel DOUBLE UNSIGNED,
	tensor_used INTEGER(1) NOT NULL DEFAULT '0',
	variance DOUBLE,
	varianceReduction DOUBLE,
	doubleCouple DOUBLE,
	clvd DOUBLE,
	iso DOUBLE,
	greensFunctionID VARCHAR,
	filterID VARCHAR,
	sourceTimeFunction_type VARCHAR(64),
	sourceTimeFunction_duration DOUBLE,
	sourceTimeFunction_riseTime DOUBLE,
	sourceTimeFunction_decayTime DOUBLE,
	sourceTimeFunction_used INTEGER(1) NOT NULL DEFAULT '0',
	methodID VARCHAR,
	method VARCHAR(64),
	status VARCHAR(64),
	cmtName VARCHAR,
	cmtVersion VARCHAR,
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

CREATE INDEX MomentTensor_derivedOriginID ON MomentTensor(derivedOriginID);

CREATE TRIGGER MomentTensorUpdate UPDATE ON MomentTensor
BEGIN
  UPDATE MomentTensor SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE FocalMechanism (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	triggeringOriginID VARCHAR,
	nodalPlanes_nodalPlane1_strike_value DOUBLE,
	nodalPlanes_nodalPlane1_strike_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_strike_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_value DOUBLE,
	nodalPlanes_nodalPlane1_dip_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_dip_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_value DOUBLE,
	nodalPlanes_nodalPlane1_rake_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_rake_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane1_used INTEGER(1) NOT NULL DEFAULT '0',
	nodalPlanes_nodalPlane2_strike_value DOUBLE,
	nodalPlanes_nodalPlane2_strike_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_strike_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_value DOUBLE,
	nodalPlanes_nodalPlane2_dip_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_dip_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_value DOUBLE,
	nodalPlanes_nodalPlane2_rake_uncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_lowerUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_upperUncertainty DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_rake_confidenceLevel DOUBLE UNSIGNED,
	nodalPlanes_nodalPlane2_used INTEGER(1) NOT NULL DEFAULT '0',
	nodalPlanes_preferredPlane INT,
	nodalPlanes_used INTEGER(1) NOT NULL DEFAULT '0',
	principalAxes_tAxis_azimuth_value DOUBLE,
	principalAxes_tAxis_azimuth_uncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_azimuth_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_value DOUBLE,
	principalAxes_tAxis_plunge_uncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_plunge_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_tAxis_length_value DOUBLE,
	principalAxes_tAxis_length_uncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_length_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_length_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_tAxis_length_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_value DOUBLE,
	principalAxes_pAxis_azimuth_uncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_azimuth_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_value DOUBLE,
	principalAxes_pAxis_plunge_uncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_plunge_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_pAxis_length_value DOUBLE,
	principalAxes_pAxis_length_uncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_length_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_length_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_pAxis_length_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_value DOUBLE,
	principalAxes_nAxis_azimuth_uncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_azimuth_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_value DOUBLE,
	principalAxes_nAxis_plunge_uncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_plunge_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_length_value DOUBLE,
	principalAxes_nAxis_length_uncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_length_lowerUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_length_upperUncertainty DOUBLE UNSIGNED,
	principalAxes_nAxis_length_confidenceLevel DOUBLE UNSIGNED,
	principalAxes_nAxis_used INTEGER(1) NOT NULL DEFAULT '0',
	principalAxes_used INTEGER(1) NOT NULL DEFAULT '0',
	azimuthalGap DOUBLE UNSIGNED,
	stationPolarityCount INT UNSIGNED,
	misfit DOUBLE UNSIGNED,
	stationDistributionRatio DOUBLE UNSIGNED,
	methodID VARCHAR,
	evaluationMode VARCHAR(64),
	evaluationStatus VARCHAR(64),
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

CREATE INDEX FocalMechanism_triggeringOriginID ON FocalMechanism(triggeringOriginID);

CREATE TRIGGER FocalMechanismUpdate UPDATE ON FocalMechanism
BEGIN
  UPDATE FocalMechanism SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Amplitude (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	type CHAR NOT NULL,
	amplitude_value DOUBLE,
	amplitude_uncertainty DOUBLE UNSIGNED,
	amplitude_lowerUncertainty DOUBLE UNSIGNED,
	amplitude_upperUncertainty DOUBLE UNSIGNED,
	amplitude_confidenceLevel DOUBLE UNSIGNED,
	amplitude_used INTEGER(1) NOT NULL DEFAULT '0',
	timeWindow_reference DATETIME,
	timeWindow_reference_ms INTEGER,
	timeWindow_begin DOUBLE,
	timeWindow_end DOUBLE,
	timeWindow_used INTEGER(1) NOT NULL DEFAULT '0',
	period_value DOUBLE,
	period_uncertainty DOUBLE UNSIGNED,
	period_lowerUncertainty DOUBLE UNSIGNED,
	period_upperUncertainty DOUBLE UNSIGNED,
	period_confidenceLevel DOUBLE UNSIGNED,
	period_used INTEGER(1) NOT NULL DEFAULT '0',
	snr DOUBLE,
	unit VARCHAR,
	pickID VARCHAR,
	waveformID_networkCode CHAR,
	waveformID_stationCode CHAR,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	waveformID_used INTEGER(1) NOT NULL DEFAULT '0',
	filterID VARCHAR,
	methodID VARCHAR,
	scalingTime_value DATETIME,
	scalingTime_value_ms INTEGER,
	scalingTime_uncertainty DOUBLE UNSIGNED,
	scalingTime_lowerUncertainty DOUBLE UNSIGNED,
	scalingTime_upperUncertainty DOUBLE UNSIGNED,
	scalingTime_confidenceLevel DOUBLE UNSIGNED,
	scalingTime_used INTEGER(1) NOT NULL DEFAULT '0',
	magnitudeHint CHAR,
	evaluationMode VARCHAR(64),
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

CREATE INDEX Amplitude__parent_oid ON Amplitude(_parent_oid);
CREATE INDEX Amplitude_pickID ON Amplitude(pickID);

CREATE TRIGGER AmplitudeUpdate UPDATE ON Amplitude
BEGIN
  UPDATE Amplitude SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE StationMagnitudeContribution (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	stationMagnitudeID VARCHAR NOT NULL,
	residual DOUBLE,
	weight DOUBLE UNSIGNED,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,stationMagnitudeID)
);

CREATE INDEX StationMagnitudeContribution_stationMagnitudeID ON StationMagnitudeContribution(stationMagnitudeID);

CREATE TRIGGER StationMagnitudeContributionUpdate UPDATE ON StationMagnitudeContribution
BEGIN
  UPDATE StationMagnitudeContribution SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Magnitude (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	magnitude_value DOUBLE NOT NULL,
	magnitude_uncertainty DOUBLE UNSIGNED,
	magnitude_lowerUncertainty DOUBLE UNSIGNED,
	magnitude_upperUncertainty DOUBLE UNSIGNED,
	magnitude_confidenceLevel DOUBLE UNSIGNED,
	type CHAR COLLATE BINARY,
	originID VARCHAR,
	methodID VARCHAR,
	stationCount INT UNSIGNED,
	azimuthalGap DOUBLE UNSIGNED,
	evaluationStatus VARCHAR(64),
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

CREATE INDEX Magnitude__parent_oid ON Magnitude(_parent_oid);

CREATE TRIGGER MagnitudeUpdate UPDATE ON Magnitude
BEGIN
  UPDATE Magnitude SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE StationMagnitude (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	originID VARCHAR,
	magnitude_value DOUBLE NOT NULL,
	magnitude_uncertainty DOUBLE UNSIGNED,
	magnitude_lowerUncertainty DOUBLE UNSIGNED,
	magnitude_upperUncertainty DOUBLE UNSIGNED,
	magnitude_confidenceLevel DOUBLE UNSIGNED,
	type CHAR COLLATE BINARY,
	amplitudeID VARCHAR,
	methodID VARCHAR,
	waveformID_networkCode CHAR,
	waveformID_stationCode CHAR,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	waveformID_used INTEGER(1) NOT NULL DEFAULT '0',
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

CREATE INDEX StationMagnitude__parent_oid ON StationMagnitude(_parent_oid);
CREATE INDEX StationMagnitude_amplitudeID ON StationMagnitude(amplitudeID);

CREATE TRIGGER StationMagnitudeUpdate UPDATE ON StationMagnitude
BEGIN
  UPDATE StationMagnitude SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Pick (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	time_value DATETIME NOT NULL,
	time_value_ms INTEGER NOT NULL,
	time_uncertainty DOUBLE UNSIGNED,
	time_lowerUncertainty DOUBLE UNSIGNED,
	time_upperUncertainty DOUBLE UNSIGNED,
	time_confidenceLevel DOUBLE UNSIGNED,
	waveformID_networkCode CHAR NOT NULL,
	waveformID_stationCode CHAR NOT NULL,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	filterID VARCHAR,
	methodID VARCHAR,
	horizontalSlowness_value DOUBLE,
	horizontalSlowness_uncertainty DOUBLE UNSIGNED,
	horizontalSlowness_lowerUncertainty DOUBLE UNSIGNED,
	horizontalSlowness_upperUncertainty DOUBLE UNSIGNED,
	horizontalSlowness_confidenceLevel DOUBLE UNSIGNED,
	horizontalSlowness_used INTEGER(1) NOT NULL DEFAULT '0',
	backazimuth_value DOUBLE,
	backazimuth_uncertainty DOUBLE UNSIGNED,
	backazimuth_lowerUncertainty DOUBLE UNSIGNED,
	backazimuth_upperUncertainty DOUBLE UNSIGNED,
	backazimuth_confidenceLevel DOUBLE UNSIGNED,
	backazimuth_used INTEGER(1) NOT NULL DEFAULT '0',
	slownessMethodID VARCHAR,
	onset VARCHAR(64),
	phaseHint_code CHAR,
	phaseHint_used INTEGER(1) NOT NULL DEFAULT '0',
	polarity VARCHAR(64),
	evaluationMode VARCHAR(64),
	evaluationStatus VARCHAR(64),
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

CREATE INDEX Pick__parent_oid ON Pick(_parent_oid);
CREATE INDEX Pick_time_value ON Pick(time_value);
CREATE INDEX Pick_time_value_ms ON Pick(time_value_ms);

CREATE TRIGGER PickUpdate UPDATE ON Pick
BEGIN
  UPDATE Pick SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE OriginReference (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	originID VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,originID)
);

CREATE INDEX OriginReference_originID ON OriginReference(originID);

CREATE TRIGGER OriginReferenceUpdate UPDATE ON OriginReference
BEGIN
  UPDATE OriginReference SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE FocalMechanismReference (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	focalMechanismID VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,focalMechanismID)
);

CREATE INDEX FocalMechanismReference_focalMechanismID ON FocalMechanismReference(focalMechanismID);

CREATE TRIGGER FocalMechanismReferenceUpdate UPDATE ON FocalMechanismReference
BEGIN
  UPDATE FocalMechanismReference SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Event (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	preferredOriginID VARCHAR,
	preferredMagnitudeID VARCHAR,
	preferredFocalMechanismID VARCHAR,
	type VARCHAR(64),
	typeCertainty VARCHAR(64),
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

CREATE INDEX Event__parent_oid ON Event(_parent_oid);
CREATE INDEX Event_preferredOriginID ON Event(preferredOriginID);
CREATE INDEX Event_preferredMagnitudeID ON Event(preferredMagnitudeID);
CREATE INDEX Event_preferredFocalMechanismID ON Event(preferredFocalMechanismID);

CREATE TRIGGER EventUpdate UPDATE ON Event
BEGIN
  UPDATE Event SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Arrival (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	pickID VARCHAR NOT NULL,
	phase_code CHAR NOT NULL,
	timeCorrection DOUBLE,
	azimuth DOUBLE,
	distance DOUBLE UNSIGNED,
	takeOffAngle DOUBLE,
	timeResidual DOUBLE,
	horizontalSlownessResidual DOUBLE,
	backazimuthResidual DOUBLE,
	timeUsed INTEGER(1),
	horizontalSlownessUsed INTEGER(1),
	backazimuthUsed INTEGER(1),
	weight DOUBLE UNSIGNED,
	earthModelID VARCHAR,
	preliminary INTEGER(1),
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
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,pickID)
);

CREATE INDEX Arrival_pickID ON Arrival(pickID);

CREATE TRIGGER ArrivalUpdate UPDATE ON Arrival
BEGIN
  UPDATE Arrival SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Origin (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	time_value DATETIME NOT NULL,
	time_value_ms INTEGER NOT NULL,
	time_uncertainty DOUBLE UNSIGNED,
	time_lowerUncertainty DOUBLE UNSIGNED,
	time_upperUncertainty DOUBLE UNSIGNED,
	time_confidenceLevel DOUBLE UNSIGNED,
	latitude_value DOUBLE NOT NULL,
	latitude_uncertainty DOUBLE UNSIGNED,
	latitude_lowerUncertainty DOUBLE UNSIGNED,
	latitude_upperUncertainty DOUBLE UNSIGNED,
	latitude_confidenceLevel DOUBLE UNSIGNED,
	longitude_value DOUBLE NOT NULL,
	longitude_uncertainty DOUBLE UNSIGNED,
	longitude_lowerUncertainty DOUBLE UNSIGNED,
	longitude_upperUncertainty DOUBLE UNSIGNED,
	longitude_confidenceLevel DOUBLE UNSIGNED,
	depth_value DOUBLE,
	depth_uncertainty DOUBLE UNSIGNED,
	depth_lowerUncertainty DOUBLE UNSIGNED,
	depth_upperUncertainty DOUBLE UNSIGNED,
	depth_confidenceLevel DOUBLE UNSIGNED,
	depth_used INTEGER(1) NOT NULL DEFAULT '0',
	depthType VARCHAR(64),
	timeFixed INTEGER(1),
	epicenterFixed INTEGER(1),
	referenceSystemID VARCHAR,
	methodID VARCHAR,
	earthModelID VARCHAR,
	quality_associatedPhaseCount INT UNSIGNED,
	quality_usedPhaseCount INT UNSIGNED,
	quality_associatedStationCount INT UNSIGNED,
	quality_usedStationCount INT UNSIGNED,
	quality_depthPhaseCount INT UNSIGNED,
	quality_standardError DOUBLE UNSIGNED,
	quality_azimuthalGap DOUBLE UNSIGNED,
	quality_secondaryAzimuthalGap DOUBLE UNSIGNED,
	quality_groundTruthLevel VARCHAR,
	quality_maximumDistance DOUBLE UNSIGNED,
	quality_minimumDistance DOUBLE UNSIGNED,
	quality_medianDistance DOUBLE UNSIGNED,
	quality_used INTEGER(1) NOT NULL DEFAULT '0',
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
	uncertainty_confidenceEllipsoid_used INTEGER(1) NOT NULL DEFAULT '0',
	uncertainty_preferredDescription VARCHAR(64),
	uncertainty_used INTEGER(1) NOT NULL DEFAULT '0',
	type VARCHAR(64),
	evaluationMode VARCHAR(64),
	evaluationStatus VARCHAR(64),
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

CREATE INDEX Origin__parent_oid ON Origin(_parent_oid);
CREATE INDEX Origin_time_value ON Origin(time_value);
CREATE INDEX Origin_time_value_ms ON Origin(time_value_ms);

CREATE TRIGGER OriginUpdate UPDATE ON Origin
BEGIN
  UPDATE Origin SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Parameter (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	value BLOB,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);


CREATE TRIGGER ParameterUpdate UPDATE ON Parameter
BEGIN
  UPDATE Parameter SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ParameterSet (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	baseID VARCHAR,
	moduleID VARCHAR,
	created DATETIME,
	created_ms INTEGER,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX ParameterSet_baseID ON ParameterSet(baseID);

CREATE TRIGGER ParameterSetUpdate UPDATE ON ParameterSet
BEGIN
  UPDATE ParameterSet SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Setup (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR,
	parameterSetID VARCHAR,
	enabled INTEGER(1) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);

CREATE INDEX Setup_parameterSetID ON Setup(parameterSetID);

CREATE TRIGGER SetupUpdate UPDATE ON Setup
BEGIN
  UPDATE Setup SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ConfigStation (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	networkCode CHAR NOT NULL,
	stationCode CHAR NOT NULL,
	enabled INTEGER(1) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,networkCode,stationCode)
);


CREATE TRIGGER ConfigStationUpdate UPDATE ON ConfigStation
BEGIN
  UPDATE ConfigStation SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ConfigModule (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	parameterSetID VARCHAR,
	enabled INTEGER(1) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX ConfigModule_parameterSetID ON ConfigModule(parameterSetID);

CREATE TRIGGER ConfigModuleUpdate UPDATE ON ConfigModule
BEGIN
  UPDATE ConfigModule SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE QCLog (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR NOT NULL,
	waveformID_stationCode CHAR NOT NULL,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	creatorID VARCHAR NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	message BLOB NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,start,start_ms,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI)
);


CREATE TRIGGER QCLogUpdate UPDATE ON QCLog
BEGIN
  UPDATE QCLog SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE WaveformQuality (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR NOT NULL,
	waveformID_stationCode CHAR NOT NULL,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	creatorID VARCHAR NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	type CHAR COLLATE BINARY NOT NULL,
	parameter CHAR COLLATE BINARY NOT NULL,
	value DOUBLE NOT NULL,
	lowerUncertainty DOUBLE,
	upperUncertainty DOUBLE,
	windowLength DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,start,start_ms,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI,type,parameter)
);

CREATE INDEX WaveformQuality_start ON WaveformQuality(start);
CREATE INDEX WaveformQuality_start_ms ON WaveformQuality(start_ms);
CREATE INDEX WaveformQuality_end ON WaveformQuality(end);
CREATE INDEX WaveformQuality_end_ms ON WaveformQuality(end_ms);

CREATE TRIGGER WaveformQualityUpdate UPDATE ON WaveformQuality
BEGIN
  UPDATE WaveformQuality SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Outage (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	waveformID_networkCode CHAR NOT NULL,
	waveformID_stationCode CHAR NOT NULL,
	waveformID_locationCode CHAR,
	waveformID_channelCode CHAR,
	waveformID_resourceURI VARCHAR,
	creatorID VARCHAR NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME,
	end_ms INTEGER,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,waveformID_networkCode,waveformID_stationCode,waveformID_locationCode,waveformID_channelCode,waveformID_resourceURI,start,start_ms)
);


CREATE TRIGGER OutageUpdate UPDATE ON Outage
BEGIN
  UPDATE Outage SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE StationReference (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	stationID VARCHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,stationID)
);

CREATE INDEX StationReference_stationID ON StationReference(stationID);

CREATE TRIGGER StationReferenceUpdate UPDATE ON StationReference
BEGIN
  UPDATE StationReference SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE StationGroup (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	type VARCHAR(64),
	code CHAR,
	start DATETIME,
	end DATETIME,
	description VARCHAR,
	latitude DOUBLE,
	longitude DOUBLE,
	elevation DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code)
);


CREATE TRIGGER StationGroupUpdate UPDATE ON StationGroup
BEGIN
  UPDATE StationGroup SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE AuxSource (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	description VARCHAR,
	unit VARCHAR,
	conversion VARCHAR,
	sampleRateNumerator INT UNSIGNED,
	sampleRateDenominator INT UNSIGNED,
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER AuxSourceUpdate UPDATE ON AuxSource
BEGIN
  UPDATE AuxSource SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE AuxDevice (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	description VARCHAR,
	model VARCHAR,
	manufacturer VARCHAR,
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER AuxDeviceUpdate UPDATE ON AuxDevice
BEGIN
  UPDATE AuxDevice SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE SensorCalibration (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	serialNumber VARCHAR NOT NULL,
	channel INT UNSIGNED NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	gain DOUBLE UNSIGNED,
	gainFrequency DOUBLE UNSIGNED,
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,serialNumber,channel,start)
);


CREATE TRIGGER SensorCalibrationUpdate UPDATE ON SensorCalibration
BEGIN
  UPDATE SensorCalibration SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Sensor (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	description VARCHAR,
	model VARCHAR,
	manufacturer VARCHAR,
	type VARCHAR,
	unit VARCHAR,
	lowFrequency DOUBLE UNSIGNED,
	highFrequency DOUBLE UNSIGNED,
	response VARCHAR,
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER SensorUpdate UPDATE ON Sensor
BEGIN
  UPDATE Sensor SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ResponsePAZ (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR,
	type CHAR,
	gain DOUBLE UNSIGNED,
	gainFrequency DOUBLE UNSIGNED,
	normalizationFactor DOUBLE UNSIGNED,
	normalizationFrequency DOUBLE UNSIGNED,
	numberOfZeros TINYINT UNSIGNED,
	numberOfPoles TINYINT UNSIGNED,
	zeros_content BLOB,
	zeros_used INTEGER(1) NOT NULL DEFAULT '0',
	poles_content BLOB,
	poles_used INTEGER(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER ResponsePAZUpdate UPDATE ON ResponsePAZ
BEGIN
  UPDATE ResponsePAZ SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ResponsePolynomial (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR,
	gain DOUBLE UNSIGNED,
	gainFrequency DOUBLE UNSIGNED,
	frequencyUnit CHAR,
	approximationType CHAR,
	approximationLowerBound DOUBLE UNSIGNED,
	approximationUpperBound DOUBLE UNSIGNED,
	approximationError DOUBLE UNSIGNED,
	numberOfCoefficients SMALLINT UNSIGNED,
	coefficients_content BLOB,
	coefficients_used INTEGER(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER ResponsePolynomialUpdate UPDATE ON ResponsePolynomial
BEGIN
  UPDATE ResponsePolynomial SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE DataloggerCalibration (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	serialNumber VARCHAR NOT NULL,
	channel INT UNSIGNED NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	gain DOUBLE UNSIGNED,
	gainFrequency DOUBLE UNSIGNED,
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,serialNumber,channel,start)
);


CREATE TRIGGER DataloggerCalibrationUpdate UPDATE ON DataloggerCalibration
BEGIN
  UPDATE DataloggerCalibration SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Decimation (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	sampleRateNumerator INT UNSIGNED NOT NULL,
	sampleRateDenominator INT UNSIGNED NOT NULL,
	analogueFilterChain_content BLOB,
	analogueFilterChain_used INTEGER(1) NOT NULL DEFAULT '0',
	digitalFilterChain_content BLOB,
	digitalFilterChain_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,sampleRateNumerator,sampleRateDenominator)
);


CREATE TRIGGER DecimationUpdate UPDATE ON Decimation
BEGIN
  UPDATE Decimation SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Datalogger (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR,
	description VARCHAR,
	digitizerModel VARCHAR,
	digitizerManufacturer VARCHAR,
	recorderModel VARCHAR,
	recorderManufacturer VARCHAR,
	clockModel VARCHAR,
	clockManufacturer VARCHAR,
	clockType VARCHAR,
	gain DOUBLE UNSIGNED,
	maxClockDrift DOUBLE UNSIGNED,
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER DataloggerUpdate UPDATE ON Datalogger
BEGIN
  UPDATE Datalogger SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ResponseFIR (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR,
	gain DOUBLE UNSIGNED,
	decimationFactor SMALLINT UNSIGNED,
	delay DOUBLE UNSIGNED,
	correction DOUBLE,
	numberOfCoefficients SMALLINT UNSIGNED,
	symmetry CHAR,
	coefficients_content BLOB,
	coefficients_used INTEGER(1) NOT NULL DEFAULT '0',
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name)
);


CREATE TRIGGER ResponseFIRUpdate UPDATE ON ResponseFIR
BEGIN
  UPDATE ResponseFIR SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE AuxStream (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	code CHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	device VARCHAR,
	deviceSerialNumber VARCHAR,
	source VARCHAR,
	format VARCHAR,
	flags VARCHAR,
	restricted INTEGER(1),
	shared INTEGER(1),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code,start)
);


CREATE TRIGGER AuxStreamUpdate UPDATE ON AuxStream
BEGIN
  UPDATE AuxStream SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Stream (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	code CHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	datalogger VARCHAR,
	dataloggerSerialNumber VARCHAR,
	dataloggerChannel INT UNSIGNED,
	sensor VARCHAR,
	sensorSerialNumber VARCHAR,
	sensorChannel INT UNSIGNED,
	clockSerialNumber VARCHAR,
	sampleRateNumerator INT UNSIGNED,
	sampleRateDenominator INT UNSIGNED,
	depth DOUBLE,
	azimuth DOUBLE,
	dip DOUBLE,
	gain DOUBLE,
	gainFrequency DOUBLE UNSIGNED,
	gainUnit CHAR,
	format VARCHAR,
	flags VARCHAR,
	restricted INTEGER(1),
	shared INTEGER(1),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code,start)
);


CREATE TRIGGER StreamUpdate UPDATE ON Stream
BEGIN
  UPDATE Stream SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE SensorLocation (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	code CHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	latitude DOUBLE,
	longitude DOUBLE,
	elevation DOUBLE,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code,start)
);


CREATE TRIGGER SensorLocationUpdate UPDATE ON SensorLocation
BEGIN
  UPDATE SensorLocation SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Station (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	code CHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	description VARCHAR,
	latitude DOUBLE,
	longitude DOUBLE,
	elevation DOUBLE,
	place VARCHAR,
	country VARCHAR,
	affiliation VARCHAR,
	type VARCHAR,
	archive VARCHAR,
	archiveNetworkCode CHAR,
	restricted INTEGER(1),
	shared INTEGER(1),
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code,start)
);


CREATE TRIGGER StationUpdate UPDATE ON Station
BEGIN
  UPDATE Station SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Network (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	code CHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	description VARCHAR,
	institutions VARCHAR,
	region VARCHAR,
	type VARCHAR,
	netClass CHAR,
	archive VARCHAR,
	restricted INTEGER(1),
	shared INTEGER(1),
	remark_content BLOB,
	remark_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,code,start)
);


CREATE TRIGGER NetworkUpdate UPDATE ON Network
BEGIN
  UPDATE Network SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE RouteArclink (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	address VARCHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	priority TINYINT UNSIGNED,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,address,start)
);


CREATE TRIGGER RouteArclinkUpdate UPDATE ON RouteArclink
BEGIN
  UPDATE RouteArclink SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE RouteSeedlink (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	address VARCHAR NOT NULL,
	priority TINYINT UNSIGNED,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,address)
);


CREATE TRIGGER RouteSeedlinkUpdate UPDATE ON RouteSeedlink
BEGIN
  UPDATE RouteSeedlink SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Route (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	networkCode CHAR NOT NULL,
	stationCode CHAR NOT NULL,
	locationCode CHAR NOT NULL,
	streamCode CHAR NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,networkCode,stationCode,locationCode,streamCode)
);


CREATE TRIGGER RouteUpdate UPDATE ON Route
BEGIN
  UPDATE Route SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE Access (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	networkCode CHAR NOT NULL,
	stationCode CHAR NOT NULL,
	locationCode CHAR NOT NULL,
	streamCode CHAR NOT NULL,
	user VARCHAR NOT NULL,
	start DATETIME NOT NULL,
	end DATETIME,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,networkCode,stationCode,locationCode,streamCode,user,start)
);


CREATE TRIGGER AccessUpdate UPDATE ON Access
BEGIN
  UPDATE Access SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE JournalEntry (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	created DATETIME,
	created_ms INTEGER,
	objectID VARCHAR NOT NULL,
	sender VARCHAR NOT NULL,
	action VARCHAR NOT NULL,
	parameters VARCHAR,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE
);

CREATE INDEX JournalEntry_objectID ON JournalEntry(objectID);

CREATE TRIGGER JournalEntryUpdate UPDATE ON JournalEntry
BEGIN
  UPDATE JournalEntry SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ArclinkUser (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	name VARCHAR NOT NULL,
	email VARCHAR,
	password VARCHAR,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,name,email)
);


CREATE TRIGGER ArclinkUserUpdate UPDATE ON ArclinkUser
BEGIN
  UPDATE ArclinkUser SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ArclinkStatusLine (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	type VARCHAR NOT NULL,
	status VARCHAR NOT NULL,
	size INT UNSIGNED,
	message VARCHAR,
	volumeID VARCHAR,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,volumeID,type,status)
);


CREATE TRIGGER ArclinkStatusLineUpdate UPDATE ON ArclinkStatusLine
BEGIN
  UPDATE ArclinkStatusLine SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ArclinkRequestLine (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	start DATETIME NOT NULL,
	start_ms INTEGER NOT NULL,
	end DATETIME NOT NULL,
	end_ms INTEGER NOT NULL,
	streamID_networkCode CHAR NOT NULL,
	streamID_stationCode CHAR NOT NULL,
	streamID_locationCode CHAR,
	streamID_channelCode CHAR,
	streamID_resourceURI VARCHAR,
	restricted INTEGER(1),
	shared INTEGER(1),
	netClass CHAR,
	constraints VARCHAR,
	status_type VARCHAR NOT NULL,
	status_status VARCHAR NOT NULL,
	status_size INT UNSIGNED,
	status_message VARCHAR,
	status_volumeID VARCHAR,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,start,start_ms,end,end_ms,streamID_networkCode,streamID_stationCode,streamID_locationCode,streamID_channelCode,streamID_resourceURI)
);


CREATE TRIGGER ArclinkRequestLineUpdate UPDATE ON ArclinkRequestLine
BEGIN
  UPDATE ArclinkRequestLine SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;

CREATE TABLE ArclinkRequest (
	_oid INTEGER NOT NULL,
	_parent_oid INTEGER NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	requestID VARCHAR NOT NULL,
	userID VARCHAR NOT NULL,
	userIP VARCHAR,
	clientID VARCHAR,
	clientIP VARCHAR,
	type VARCHAR NOT NULL,
	created DATETIME NOT NULL,
	created_ms INTEGER NOT NULL,
	status VARCHAR NOT NULL,
	message VARCHAR,
	label VARCHAR,
	header VARCHAR,
	summary_okLineCount INT UNSIGNED,
	summary_totalLineCount INT UNSIGNED,
	summary_averageTimeWindow INT UNSIGNED,
	summary_used INTEGER(1) NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	FOREIGN KEY(_parent_oid)
	  REFERENCES Object(_oid)
	  ON DELETE CASCADE,
	UNIQUE(_parent_oid,created,created_ms,requestID,userID)
);


CREATE TRIGGER ArclinkRequestUpdate UPDATE ON ArclinkRequest
BEGIN
  UPDATE ArclinkRequest SET _last_modified=CURRENT_TIMESTAMP WHERE _oid=old._oid;
END;
