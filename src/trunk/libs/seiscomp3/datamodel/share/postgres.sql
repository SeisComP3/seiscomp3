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
DROP TABLE ResponseFAP;
DROP TABLE ResponseFIR;
DROP TABLE ResponseIIR;
DROP TABLE DataloggerCalibration;
DROP TABLE Decimation;
DROP TABLE Datalogger;
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
DROP TABLE DataSegment;
DROP TABLE DataAttributeExtent;
DROP TABLE DataExtent;
DROP TABLE PublicObject;
DROP TABLE Object;
DROP TABLE Meta;
DROP SEQUENCE Object_seq;
DROP FUNCTION update_modified();
DROP LANGUAGE plpgsql;

CREATE LANGUAGE plpgsql;
CREATE FUNCTION update_modified() returns trigger as $$begin new._last_modified := now(); return new; end;$$ LANGUAGE plpgsql;

CREATE TABLE Meta (
	name VARCHAR(80) NOT NULL,
	value VARCHAR(255) NOT NULL,
	PRIMARY KEY(name)
);

CREATE SEQUENCE Object_seq;
CREATE TABLE Object (
	_oid BIGINT DEFAULT NEXTVAL('Object_seq') NOT NULL,
	_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY(_oid)
);

CREATE TABLE PublicObject (
	_oid BIGINT NOT NULL,
	m_publicID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	UNIQUE(m_publicID),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

INSERT INTO Meta(name,value) VALUES ('Schema-Version', '0.11');
INSERT INTO Meta(name,value) VALUES ('Creation-Time', CURRENT_TIMESTAMP);

INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'EventParameters');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'Config');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'QualityControl');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'Inventory');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'Routing');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'Journaling');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'ArclinkLog');
INSERT INTO Object(_oid) VALUES (DEFAULT);
INSERT INTO PublicObject(_oid,m_publicID) VALUES (CURRVAL('Object_seq'),'DataAvailability');

CREATE TABLE EventDescription (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_text VARCHAR(128) NOT NULL,
	m_type VARCHAR(64) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT eventdescription_composite_index UNIQUE(_parent_oid,m_type)
);

CREATE INDEX EventDescription__parent_oid ON EventDescription(_parent_oid);

CREATE TRIGGER EventDescription_update BEFORE UPDATE ON EventDescription FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Comment (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_text BYTEA NOT NULL,
	m_id VARCHAR(255),
	m_start TIMESTAMP,
	m_start_ms INTEGER,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
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
	CONSTRAINT comment_composite_index UNIQUE(_parent_oid,m_id)
);

CREATE INDEX Comment__parent_oid ON Comment(_parent_oid);

CREATE TRIGGER Comment_update BEFORE UPDATE ON Comment FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE DataUsed (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_waveType VARCHAR(64) NOT NULL,
	m_stationCount INT NOT NULL,
	m_componentCount INT NOT NULL,
	m_shortestPeriod DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX DataUsed__parent_oid ON DataUsed(_parent_oid);

CREATE TRIGGER DataUsed_update BEFORE UPDATE ON DataUsed FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE CompositeTime (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_year_value INT,
	m_year_uncertainty INT,
	m_year_lowerUncertainty INT,
	m_year_upperUncertainty INT,
	m_year_confidenceLevel DOUBLE PRECISION,
	m_year_used BOOLEAN NOT NULL DEFAULT '0',
	m_month_value INT,
	m_month_uncertainty INT,
	m_month_lowerUncertainty INT,
	m_month_upperUncertainty INT,
	m_month_confidenceLevel DOUBLE PRECISION,
	m_month_used BOOLEAN NOT NULL DEFAULT '0',
	m_day_value INT,
	m_day_uncertainty INT,
	m_day_lowerUncertainty INT,
	m_day_upperUncertainty INT,
	m_day_confidenceLevel DOUBLE PRECISION,
	m_day_used BOOLEAN NOT NULL DEFAULT '0',
	m_hour_value INT,
	m_hour_uncertainty INT,
	m_hour_lowerUncertainty INT,
	m_hour_upperUncertainty INT,
	m_hour_confidenceLevel DOUBLE PRECISION,
	m_hour_used BOOLEAN NOT NULL DEFAULT '0',
	m_minute_value INT,
	m_minute_uncertainty INT,
	m_minute_lowerUncertainty INT,
	m_minute_upperUncertainty INT,
	m_minute_confidenceLevel DOUBLE PRECISION,
	m_minute_used BOOLEAN NOT NULL DEFAULT '0',
	m_second_value DOUBLE PRECISION,
	m_second_uncertainty DOUBLE PRECISION,
	m_second_lowerUncertainty DOUBLE PRECISION,
	m_second_upperUncertainty DOUBLE PRECISION,
	m_second_confidenceLevel DOUBLE PRECISION,
	m_second_pdf_variable_content BYTEA,
	m_second_pdf_probability_content BYTEA,
	m_second_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_second_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX CompositeTime__parent_oid ON CompositeTime(_parent_oid);

CREATE TRIGGER CompositeTime_update BEFORE UPDATE ON CompositeTime FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE PickReference (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_pickID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT pickreference_composite_index UNIQUE(_parent_oid,m_pickID)
);

CREATE INDEX PickReference__parent_oid ON PickReference(_parent_oid);
CREATE INDEX PickReference_m_pickID ON PickReference(m_pickID);

CREATE TRIGGER PickReference_update BEFORE UPDATE ON PickReference FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE AmplitudeReference (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_amplitudeID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT amplitudereference_composite_index UNIQUE(_parent_oid,m_amplitudeID)
);

CREATE INDEX AmplitudeReference__parent_oid ON AmplitudeReference(_parent_oid);
CREATE INDEX AmplitudeReference_m_amplitudeID ON AmplitudeReference(m_amplitudeID);

CREATE TRIGGER AmplitudeReference_update BEFORE UPDATE ON AmplitudeReference FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Reading (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX Reading__parent_oid ON Reading(_parent_oid);



CREATE TABLE MomentTensorComponentContribution (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_phaseCode VARCHAR(4) NOT NULL,
	m_component INT NOT NULL,
	m_active BOOLEAN NOT NULL,
	m_weight DOUBLE PRECISION NOT NULL,
	m_timeShift DOUBLE PRECISION NOT NULL,
	m_dataTimeWindow BYTEA NOT NULL,
	m_misfit DOUBLE PRECISION,
	m_snr DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT momenttensorcomponentcontribution_composite_index UNIQUE(_parent_oid,m_phaseCode,m_component)
);

CREATE INDEX MomentTensorComponentContribution__parent_oid ON MomentTensorComponentContribution(_parent_oid);

CREATE TRIGGER MomentTensorComponentContribution_update BEFORE UPDATE ON MomentTensorComponentContribution FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE MomentTensorStationContribution (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_active BOOLEAN NOT NULL,
	m_waveformID_networkCode VARCHAR(8),
	m_waveformID_stationCode VARCHAR(8),
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_waveformID_used BOOLEAN NOT NULL DEFAULT '0',
	m_weight DOUBLE PRECISION,
	m_timeShift DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX MomentTensorStationContribution__parent_oid ON MomentTensorStationContribution(_parent_oid);

CREATE TRIGGER MomentTensorStationContribution_update BEFORE UPDATE ON MomentTensorStationContribution FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE MomentTensorPhaseSetting (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_code VARCHAR(4) NOT NULL,
	m_lowerPeriod DOUBLE PRECISION NOT NULL,
	m_upperPeriod DOUBLE PRECISION NOT NULL,
	m_minimumSNR DOUBLE PRECISION,
	m_maximumTimeShift DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT momenttensorphasesetting_composite_index UNIQUE(_parent_oid,m_code)
);

CREATE INDEX MomentTensorPhaseSetting__parent_oid ON MomentTensorPhaseSetting(_parent_oid);

CREATE TRIGGER MomentTensorPhaseSetting_update BEFORE UPDATE ON MomentTensorPhaseSetting FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE MomentTensor (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_derivedOriginID VARCHAR(255) NOT NULL,
	m_momentMagnitudeID VARCHAR(255),
	m_scalarMoment_value DOUBLE PRECISION,
	m_scalarMoment_uncertainty DOUBLE PRECISION,
	m_scalarMoment_lowerUncertainty DOUBLE PRECISION,
	m_scalarMoment_upperUncertainty DOUBLE PRECISION,
	m_scalarMoment_confidenceLevel DOUBLE PRECISION,
	m_scalarMoment_pdf_variable_content BYTEA,
	m_scalarMoment_pdf_probability_content BYTEA,
	m_scalarMoment_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_scalarMoment_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_Mrr_value DOUBLE PRECISION,
	m_tensor_Mrr_uncertainty DOUBLE PRECISION,
	m_tensor_Mrr_lowerUncertainty DOUBLE PRECISION,
	m_tensor_Mrr_upperUncertainty DOUBLE PRECISION,
	m_tensor_Mrr_confidenceLevel DOUBLE PRECISION,
	m_tensor_Mrr_pdf_variable_content BYTEA,
	m_tensor_Mrr_pdf_probability_content BYTEA,
	m_tensor_Mrr_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_Mtt_value DOUBLE PRECISION,
	m_tensor_Mtt_uncertainty DOUBLE PRECISION,
	m_tensor_Mtt_lowerUncertainty DOUBLE PRECISION,
	m_tensor_Mtt_upperUncertainty DOUBLE PRECISION,
	m_tensor_Mtt_confidenceLevel DOUBLE PRECISION,
	m_tensor_Mtt_pdf_variable_content BYTEA,
	m_tensor_Mtt_pdf_probability_content BYTEA,
	m_tensor_Mtt_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_Mpp_value DOUBLE PRECISION,
	m_tensor_Mpp_uncertainty DOUBLE PRECISION,
	m_tensor_Mpp_lowerUncertainty DOUBLE PRECISION,
	m_tensor_Mpp_upperUncertainty DOUBLE PRECISION,
	m_tensor_Mpp_confidenceLevel DOUBLE PRECISION,
	m_tensor_Mpp_pdf_variable_content BYTEA,
	m_tensor_Mpp_pdf_probability_content BYTEA,
	m_tensor_Mpp_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_Mrt_value DOUBLE PRECISION,
	m_tensor_Mrt_uncertainty DOUBLE PRECISION,
	m_tensor_Mrt_lowerUncertainty DOUBLE PRECISION,
	m_tensor_Mrt_upperUncertainty DOUBLE PRECISION,
	m_tensor_Mrt_confidenceLevel DOUBLE PRECISION,
	m_tensor_Mrt_pdf_variable_content BYTEA,
	m_tensor_Mrt_pdf_probability_content BYTEA,
	m_tensor_Mrt_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_Mrp_value DOUBLE PRECISION,
	m_tensor_Mrp_uncertainty DOUBLE PRECISION,
	m_tensor_Mrp_lowerUncertainty DOUBLE PRECISION,
	m_tensor_Mrp_upperUncertainty DOUBLE PRECISION,
	m_tensor_Mrp_confidenceLevel DOUBLE PRECISION,
	m_tensor_Mrp_pdf_variable_content BYTEA,
	m_tensor_Mrp_pdf_probability_content BYTEA,
	m_tensor_Mrp_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_Mtp_value DOUBLE PRECISION,
	m_tensor_Mtp_uncertainty DOUBLE PRECISION,
	m_tensor_Mtp_lowerUncertainty DOUBLE PRECISION,
	m_tensor_Mtp_upperUncertainty DOUBLE PRECISION,
	m_tensor_Mtp_confidenceLevel DOUBLE PRECISION,
	m_tensor_Mtp_pdf_variable_content BYTEA,
	m_tensor_Mtp_pdf_probability_content BYTEA,
	m_tensor_Mtp_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_tensor_used BOOLEAN NOT NULL DEFAULT '0',
	m_variance DOUBLE PRECISION,
	m_varianceReduction DOUBLE PRECISION,
	m_doubleCouple DOUBLE PRECISION,
	m_clvd DOUBLE PRECISION,
	m_iso DOUBLE PRECISION,
	m_greensFunctionID VARCHAR(255),
	m_filterID VARCHAR(255),
	m_sourceTimeFunction_type VARCHAR(64),
	m_sourceTimeFunction_duration DOUBLE PRECISION,
	m_sourceTimeFunction_riseTime DOUBLE PRECISION,
	m_sourceTimeFunction_decayTime DOUBLE PRECISION,
	m_sourceTimeFunction_used BOOLEAN NOT NULL DEFAULT '0',
	m_methodID VARCHAR(255),
	m_method VARCHAR(64),
	m_status VARCHAR(64),
	m_cmtName VARCHAR(80),
	m_cmtVersion VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX MomentTensor__parent_oid ON MomentTensor(_parent_oid);
CREATE INDEX MomentTensor_m_derivedOriginID ON MomentTensor(m_derivedOriginID);

CREATE TRIGGER MomentTensor_update BEFORE UPDATE ON MomentTensor FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE FocalMechanism (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_triggeringOriginID VARCHAR(255),
	m_nodalPlanes_nodalPlane1_strike_value DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_strike_uncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_strike_lowerUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_strike_upperUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_strike_confidenceLevel DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_strike_pdf_variable_content BYTEA,
	m_nodalPlanes_nodalPlane1_strike_pdf_probability_content BYTEA,
	m_nodalPlanes_nodalPlane1_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane1_dip_value DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_dip_uncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_dip_lowerUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_dip_upperUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_dip_confidenceLevel DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_dip_pdf_variable_content BYTEA,
	m_nodalPlanes_nodalPlane1_dip_pdf_probability_content BYTEA,
	m_nodalPlanes_nodalPlane1_dip_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane1_rake_value DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_rake_uncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_rake_lowerUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_rake_upperUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_rake_confidenceLevel DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane1_rake_pdf_variable_content BYTEA,
	m_nodalPlanes_nodalPlane1_rake_pdf_probability_content BYTEA,
	m_nodalPlanes_nodalPlane1_rake_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane1_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane2_strike_value DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_strike_uncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_strike_lowerUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_strike_upperUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_strike_confidenceLevel DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_strike_pdf_variable_content BYTEA,
	m_nodalPlanes_nodalPlane2_strike_pdf_probability_content BYTEA,
	m_nodalPlanes_nodalPlane2_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane2_dip_value DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_dip_uncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_dip_lowerUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_dip_upperUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_dip_confidenceLevel DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_dip_pdf_variable_content BYTEA,
	m_nodalPlanes_nodalPlane2_dip_pdf_probability_content BYTEA,
	m_nodalPlanes_nodalPlane2_dip_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane2_rake_value DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_rake_uncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_rake_lowerUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_rake_upperUncertainty DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_rake_confidenceLevel DOUBLE PRECISION,
	m_nodalPlanes_nodalPlane2_rake_pdf_variable_content BYTEA,
	m_nodalPlanes_nodalPlane2_rake_pdf_probability_content BYTEA,
	m_nodalPlanes_nodalPlane2_rake_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_nodalPlane2_used BOOLEAN NOT NULL DEFAULT '0',
	m_nodalPlanes_preferredPlane INT,
	m_nodalPlanes_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_tAxis_azimuth_value DOUBLE PRECISION,
	m_principalAxes_tAxis_azimuth_uncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_azimuth_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_azimuth_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_azimuth_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_tAxis_azimuth_pdf_variable_content BYTEA,
	m_principalAxes_tAxis_azimuth_pdf_probability_content BYTEA,
	m_principalAxes_tAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_tAxis_plunge_value DOUBLE PRECISION,
	m_principalAxes_tAxis_plunge_uncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_plunge_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_plunge_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_plunge_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_tAxis_plunge_pdf_variable_content BYTEA,
	m_principalAxes_tAxis_plunge_pdf_probability_content BYTEA,
	m_principalAxes_tAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_tAxis_length_value DOUBLE PRECISION,
	m_principalAxes_tAxis_length_uncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_length_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_length_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_tAxis_length_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_tAxis_length_pdf_variable_content BYTEA,
	m_principalAxes_tAxis_length_pdf_probability_content BYTEA,
	m_principalAxes_tAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_pAxis_azimuth_value DOUBLE PRECISION,
	m_principalAxes_pAxis_azimuth_uncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_azimuth_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_azimuth_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_azimuth_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_pAxis_azimuth_pdf_variable_content BYTEA,
	m_principalAxes_pAxis_azimuth_pdf_probability_content BYTEA,
	m_principalAxes_pAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_pAxis_plunge_value DOUBLE PRECISION,
	m_principalAxes_pAxis_plunge_uncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_plunge_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_plunge_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_plunge_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_pAxis_plunge_pdf_variable_content BYTEA,
	m_principalAxes_pAxis_plunge_pdf_probability_content BYTEA,
	m_principalAxes_pAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_pAxis_length_value DOUBLE PRECISION,
	m_principalAxes_pAxis_length_uncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_length_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_length_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_pAxis_length_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_pAxis_length_pdf_variable_content BYTEA,
	m_principalAxes_pAxis_length_pdf_probability_content BYTEA,
	m_principalAxes_pAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_nAxis_azimuth_value DOUBLE PRECISION,
	m_principalAxes_nAxis_azimuth_uncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_azimuth_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_azimuth_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_azimuth_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_nAxis_azimuth_pdf_variable_content BYTEA,
	m_principalAxes_nAxis_azimuth_pdf_probability_content BYTEA,
	m_principalAxes_nAxis_azimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_nAxis_plunge_value DOUBLE PRECISION,
	m_principalAxes_nAxis_plunge_uncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_plunge_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_plunge_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_plunge_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_nAxis_plunge_pdf_variable_content BYTEA,
	m_principalAxes_nAxis_plunge_pdf_probability_content BYTEA,
	m_principalAxes_nAxis_plunge_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_nAxis_length_value DOUBLE PRECISION,
	m_principalAxes_nAxis_length_uncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_length_lowerUncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_length_upperUncertainty DOUBLE PRECISION,
	m_principalAxes_nAxis_length_confidenceLevel DOUBLE PRECISION,
	m_principalAxes_nAxis_length_pdf_variable_content BYTEA,
	m_principalAxes_nAxis_length_pdf_probability_content BYTEA,
	m_principalAxes_nAxis_length_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_nAxis_used BOOLEAN NOT NULL DEFAULT '0',
	m_principalAxes_used BOOLEAN NOT NULL DEFAULT '0',
	m_azimuthalGap DOUBLE PRECISION,
	m_stationPolarityCount INT,
	m_misfit DOUBLE PRECISION,
	m_stationDistributionRatio DOUBLE PRECISION,
	m_methodID VARCHAR(255),
	m_evaluationMode VARCHAR(64),
	m_evaluationStatus VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX FocalMechanism__parent_oid ON FocalMechanism(_parent_oid);
CREATE INDEX FocalMechanism_m_triggeringOriginID ON FocalMechanism(m_triggeringOriginID);

CREATE TRIGGER FocalMechanism_update BEFORE UPDATE ON FocalMechanism FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Amplitude (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_type VARCHAR(16) NOT NULL,
	m_amplitude_value DOUBLE PRECISION,
	m_amplitude_uncertainty DOUBLE PRECISION,
	m_amplitude_lowerUncertainty DOUBLE PRECISION,
	m_amplitude_upperUncertainty DOUBLE PRECISION,
	m_amplitude_confidenceLevel DOUBLE PRECISION,
	m_amplitude_pdf_variable_content BYTEA,
	m_amplitude_pdf_probability_content BYTEA,
	m_amplitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_amplitude_used BOOLEAN NOT NULL DEFAULT '0',
	m_timeWindow_reference TIMESTAMP,
	m_timeWindow_reference_ms INTEGER,
	m_timeWindow_begin DOUBLE PRECISION,
	m_timeWindow_end DOUBLE PRECISION,
	m_timeWindow_used BOOLEAN NOT NULL DEFAULT '0',
	m_period_value DOUBLE PRECISION,
	m_period_uncertainty DOUBLE PRECISION,
	m_period_lowerUncertainty DOUBLE PRECISION,
	m_period_upperUncertainty DOUBLE PRECISION,
	m_period_confidenceLevel DOUBLE PRECISION,
	m_period_pdf_variable_content BYTEA,
	m_period_pdf_probability_content BYTEA,
	m_period_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_period_used BOOLEAN NOT NULL DEFAULT '0',
	m_snr DOUBLE PRECISION,
	m_unit VARCHAR(255),
	m_pickID VARCHAR(255),
	m_waveformID_networkCode VARCHAR(8),
	m_waveformID_stationCode VARCHAR(8),
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_waveformID_used BOOLEAN NOT NULL DEFAULT '0',
	m_filterID VARCHAR(255),
	m_methodID VARCHAR(255),
	m_scalingTime_value TIMESTAMP,
	m_scalingTime_value_ms INTEGER,
	m_scalingTime_uncertainty DOUBLE PRECISION,
	m_scalingTime_lowerUncertainty DOUBLE PRECISION,
	m_scalingTime_upperUncertainty DOUBLE PRECISION,
	m_scalingTime_confidenceLevel DOUBLE PRECISION,
	m_scalingTime_pdf_variable_content BYTEA,
	m_scalingTime_pdf_probability_content BYTEA,
	m_scalingTime_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_scalingTime_used BOOLEAN NOT NULL DEFAULT '0',
	m_magnitudeHint VARCHAR(16),
	m_evaluationMode VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX Amplitude__parent_oid ON Amplitude(_parent_oid);
CREATE INDEX Amplitude_timeWindow_reference ON Amplitude(m_timeWindow_reference,m_timeWindow_reference_ms);
CREATE INDEX Amplitude_m_pickID ON Amplitude(m_pickID);

CREATE TRIGGER Amplitude_update BEFORE UPDATE ON Amplitude FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE StationMagnitudeContribution (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_stationMagnitudeID VARCHAR(255) NOT NULL,
	m_residual DOUBLE PRECISION,
	m_weight DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT stationmagnitudecontribution_composite_index UNIQUE(_parent_oid,m_stationMagnitudeID)
);

CREATE INDEX StationMagnitudeContribution__parent_oid ON StationMagnitudeContribution(_parent_oid);
CREATE INDEX StationMagnitudeContribution_m_stationMagnitudeID ON StationMagnitudeContribution(m_stationMagnitudeID);

CREATE TRIGGER StationMagnitudeContribution_update BEFORE UPDATE ON StationMagnitudeContribution FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Magnitude (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_magnitude_value DOUBLE PRECISION NOT NULL,
	m_magnitude_uncertainty DOUBLE PRECISION,
	m_magnitude_lowerUncertainty DOUBLE PRECISION,
	m_magnitude_upperUncertainty DOUBLE PRECISION,
	m_magnitude_confidenceLevel DOUBLE PRECISION,
	m_magnitude_pdf_variable_content BYTEA,
	m_magnitude_pdf_probability_content BYTEA,
	m_magnitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_type VARCHAR(16),
	m_originID VARCHAR(255),
	m_methodID VARCHAR(255),
	m_stationCount INT,
	m_azimuthalGap DOUBLE PRECISION,
	m_evaluationStatus VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX Magnitude__parent_oid ON Magnitude(_parent_oid);

CREATE TRIGGER Magnitude_update BEFORE UPDATE ON Magnitude FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE StationMagnitude (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_originID VARCHAR(255),
	m_magnitude_value DOUBLE PRECISION NOT NULL,
	m_magnitude_uncertainty DOUBLE PRECISION,
	m_magnitude_lowerUncertainty DOUBLE PRECISION,
	m_magnitude_upperUncertainty DOUBLE PRECISION,
	m_magnitude_confidenceLevel DOUBLE PRECISION,
	m_magnitude_pdf_variable_content BYTEA,
	m_magnitude_pdf_probability_content BYTEA,
	m_magnitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_type VARCHAR(16),
	m_amplitudeID VARCHAR(255),
	m_methodID VARCHAR(255),
	m_waveformID_networkCode VARCHAR(8),
	m_waveformID_stationCode VARCHAR(8),
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_waveformID_used BOOLEAN NOT NULL DEFAULT '0',
	m_passedQC BOOLEAN,
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
		ON DELETE CASCADE
);

CREATE INDEX StationMagnitude__parent_oid ON StationMagnitude(_parent_oid);
CREATE INDEX StationMagnitude_m_amplitudeID ON StationMagnitude(m_amplitudeID);

CREATE TRIGGER StationMagnitude_update BEFORE UPDATE ON StationMagnitude FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Pick (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_time_value TIMESTAMP NOT NULL,
	m_time_value_ms INTEGER NOT NULL,
	m_time_uncertainty DOUBLE PRECISION,
	m_time_lowerUncertainty DOUBLE PRECISION,
	m_time_upperUncertainty DOUBLE PRECISION,
	m_time_confidenceLevel DOUBLE PRECISION,
	m_time_pdf_variable_content BYTEA,
	m_time_pdf_probability_content BYTEA,
	m_time_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_filterID VARCHAR(255),
	m_methodID VARCHAR(255),
	m_horizontalSlowness_value DOUBLE PRECISION,
	m_horizontalSlowness_uncertainty DOUBLE PRECISION,
	m_horizontalSlowness_lowerUncertainty DOUBLE PRECISION,
	m_horizontalSlowness_upperUncertainty DOUBLE PRECISION,
	m_horizontalSlowness_confidenceLevel DOUBLE PRECISION,
	m_horizontalSlowness_pdf_variable_content BYTEA,
	m_horizontalSlowness_pdf_probability_content BYTEA,
	m_horizontalSlowness_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_horizontalSlowness_used BOOLEAN NOT NULL DEFAULT '0',
	m_backazimuth_value DOUBLE PRECISION,
	m_backazimuth_uncertainty DOUBLE PRECISION,
	m_backazimuth_lowerUncertainty DOUBLE PRECISION,
	m_backazimuth_upperUncertainty DOUBLE PRECISION,
	m_backazimuth_confidenceLevel DOUBLE PRECISION,
	m_backazimuth_pdf_variable_content BYTEA,
	m_backazimuth_pdf_probability_content BYTEA,
	m_backazimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_backazimuth_used BOOLEAN NOT NULL DEFAULT '0',
	m_slownessMethodID VARCHAR(255),
	m_onset VARCHAR(64),
	m_phaseHint_code VARCHAR(32),
	m_phaseHint_used BOOLEAN NOT NULL DEFAULT '0',
	m_polarity VARCHAR(64),
	m_evaluationMode VARCHAR(64),
	m_evaluationStatus VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX Pick__parent_oid ON Pick(_parent_oid);
CREATE INDEX Pick_m_time_value_m_time_value_ms ON Pick(m_time_value,m_time_value_ms);

CREATE TRIGGER Pick_update BEFORE UPDATE ON Pick FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE OriginReference (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_originID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT originreference_composite_index UNIQUE(_parent_oid,m_originID)
);

CREATE INDEX OriginReference__parent_oid ON OriginReference(_parent_oid);
CREATE INDEX OriginReference_m_originID ON OriginReference(m_originID);

CREATE TRIGGER OriginReference_update BEFORE UPDATE ON OriginReference FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE FocalMechanismReference (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_focalMechanismID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT focalmechanismreference_composite_index UNIQUE(_parent_oid,m_focalMechanismID)
);

CREATE INDEX FocalMechanismReference__parent_oid ON FocalMechanismReference(_parent_oid);
CREATE INDEX FocalMechanismReference_m_focalMechanismID ON FocalMechanismReference(m_focalMechanismID);

CREATE TRIGGER FocalMechanismReference_update BEFORE UPDATE ON FocalMechanismReference FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Event (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_preferredOriginID VARCHAR(255),
	m_preferredMagnitudeID VARCHAR(255),
	m_preferredFocalMechanismID VARCHAR(255),
	m_type VARCHAR(64),
	m_typeCertainty VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX Event__parent_oid ON Event(_parent_oid);
CREATE INDEX Event_m_preferredOriginID ON Event(m_preferredOriginID);
CREATE INDEX Event_m_preferredMagnitudeID ON Event(m_preferredMagnitudeID);
CREATE INDEX Event_m_preferredFocalMechanismID ON Event(m_preferredFocalMechanismID);

CREATE TRIGGER Event_update BEFORE UPDATE ON Event FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Arrival (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_pickID VARCHAR(255) NOT NULL,
	m_phase_code VARCHAR(32) NOT NULL,
	m_timeCorrection DOUBLE PRECISION,
	m_azimuth DOUBLE PRECISION,
	m_distance DOUBLE PRECISION,
	m_takeOffAngle DOUBLE PRECISION,
	m_timeResidual DOUBLE PRECISION,
	m_horizontalSlownessResidual DOUBLE PRECISION,
	m_backazimuthResidual DOUBLE PRECISION,
	m_timeUsed BOOLEAN,
	m_horizontalSlownessUsed BOOLEAN,
	m_backazimuthUsed BOOLEAN,
	m_weight DOUBLE PRECISION,
	m_earthModelID VARCHAR(255),
	m_preliminary BOOLEAN,
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
	CONSTRAINT arrival_composite_index UNIQUE(_parent_oid,m_pickID)
);

CREATE INDEX Arrival__parent_oid ON Arrival(_parent_oid);
CREATE INDEX Arrival_m_pickID ON Arrival(m_pickID);

CREATE TRIGGER Arrival_update BEFORE UPDATE ON Arrival FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Origin (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_time_value TIMESTAMP NOT NULL,
	m_time_value_ms INTEGER NOT NULL,
	m_time_uncertainty DOUBLE PRECISION,
	m_time_lowerUncertainty DOUBLE PRECISION,
	m_time_upperUncertainty DOUBLE PRECISION,
	m_time_confidenceLevel DOUBLE PRECISION,
	m_time_pdf_variable_content BYTEA,
	m_time_pdf_probability_content BYTEA,
	m_time_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_latitude_value DOUBLE PRECISION NOT NULL,
	m_latitude_uncertainty DOUBLE PRECISION,
	m_latitude_lowerUncertainty DOUBLE PRECISION,
	m_latitude_upperUncertainty DOUBLE PRECISION,
	m_latitude_confidenceLevel DOUBLE PRECISION,
	m_latitude_pdf_variable_content BYTEA,
	m_latitude_pdf_probability_content BYTEA,
	m_latitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_longitude_value DOUBLE PRECISION NOT NULL,
	m_longitude_uncertainty DOUBLE PRECISION,
	m_longitude_lowerUncertainty DOUBLE PRECISION,
	m_longitude_upperUncertainty DOUBLE PRECISION,
	m_longitude_confidenceLevel DOUBLE PRECISION,
	m_longitude_pdf_variable_content BYTEA,
	m_longitude_pdf_probability_content BYTEA,
	m_longitude_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_depth_value DOUBLE PRECISION,
	m_depth_uncertainty DOUBLE PRECISION,
	m_depth_lowerUncertainty DOUBLE PRECISION,
	m_depth_upperUncertainty DOUBLE PRECISION,
	m_depth_confidenceLevel DOUBLE PRECISION,
	m_depth_pdf_variable_content BYTEA,
	m_depth_pdf_probability_content BYTEA,
	m_depth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
	m_depth_used BOOLEAN NOT NULL DEFAULT '0',
	m_depthType VARCHAR(64),
	m_timeFixed BOOLEAN,
	m_epicenterFixed BOOLEAN,
	m_referenceSystemID VARCHAR(255),
	m_methodID VARCHAR(255),
	m_earthModelID VARCHAR(255),
	m_quality_associatedPhaseCount INT,
	m_quality_usedPhaseCount INT,
	m_quality_associatedStationCount INT,
	m_quality_usedStationCount INT,
	m_quality_depthPhaseCount INT,
	m_quality_standardError DOUBLE PRECISION,
	m_quality_azimuthalGap DOUBLE PRECISION,
	m_quality_secondaryAzimuthalGap DOUBLE PRECISION,
	m_quality_groundTruthLevel VARCHAR(16),
	m_quality_maximumDistance DOUBLE PRECISION,
	m_quality_minimumDistance DOUBLE PRECISION,
	m_quality_medianDistance DOUBLE PRECISION,
	m_quality_used BOOLEAN NOT NULL DEFAULT '0',
	m_uncertainty_horizontalUncertainty DOUBLE PRECISION,
	m_uncertainty_minHorizontalUncertainty DOUBLE PRECISION,
	m_uncertainty_maxHorizontalUncertainty DOUBLE PRECISION,
	m_uncertainty_azimuthMaxHorizontalUncertainty DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_semiMajorAxisLength DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_semiMinorAxisLength DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_semiIntermediateAxisLength DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_majorAxisPlunge DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_majorAxisAzimuth DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_majorAxisRotation DOUBLE PRECISION,
	m_uncertainty_confidenceEllipsoid_used BOOLEAN NOT NULL DEFAULT '0',
	m_uncertainty_preferredDescription VARCHAR(64),
	m_uncertainty_used BOOLEAN NOT NULL DEFAULT '0',
	m_type VARCHAR(64),
	m_evaluationMode VARCHAR(64),
	m_evaluationStatus VARCHAR(64),
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
		ON DELETE CASCADE
);

CREATE INDEX Origin__parent_oid ON Origin(_parent_oid);
CREATE INDEX Origin_m_time_value_m_time_value_ms ON Origin(m_time_value,m_time_value_ms);

CREATE TRIGGER Origin_update BEFORE UPDATE ON Origin FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Parameter (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255) NOT NULL,
	m_value BYTEA,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX Parameter__parent_oid ON Parameter(_parent_oid);

CREATE TRIGGER Parameter_update BEFORE UPDATE ON Parameter FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ParameterSet (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_baseID VARCHAR(255),
	m_moduleID VARCHAR(255),
	m_created TIMESTAMP,
	m_created_ms INTEGER,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX ParameterSet__parent_oid ON ParameterSet(_parent_oid);
CREATE INDEX ParameterSet_m_baseID ON ParameterSet(m_baseID);

CREATE TRIGGER ParameterSet_update BEFORE UPDATE ON ParameterSet FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Setup (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(20),
	m_parameterSetID VARCHAR(255),
	m_enabled BOOLEAN NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT setup_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX Setup__parent_oid ON Setup(_parent_oid);
CREATE INDEX Setup_m_parameterSetID ON Setup(m_parameterSetID);

CREATE TRIGGER Setup_update BEFORE UPDATE ON Setup FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ConfigStation (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_networkCode VARCHAR(8) NOT NULL,
	m_stationCode VARCHAR(8) NOT NULL,
	m_enabled BOOLEAN NOT NULL,
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
	CONSTRAINT configstation_composite_index UNIQUE(_parent_oid,m_networkCode,m_stationCode)
);

CREATE INDEX ConfigStation__parent_oid ON ConfigStation(_parent_oid);

CREATE TRIGGER ConfigStation_update BEFORE UPDATE ON ConfigStation FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ConfigModule (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(20) NOT NULL,
	m_parameterSetID VARCHAR(255),
	m_enabled BOOLEAN NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX ConfigModule__parent_oid ON ConfigModule(_parent_oid);
CREATE INDEX ConfigModule_m_parameterSetID ON ConfigModule(m_parameterSetID);

CREATE TRIGGER ConfigModule_update BEFORE UPDATE ON ConfigModule FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE QCLog (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_creatorID VARCHAR(255) NOT NULL,
	m_created TIMESTAMP NOT NULL,
	m_created_ms INTEGER NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP NOT NULL,
	m_end_ms INTEGER NOT NULL,
	m_message BYTEA NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT qclog_composite_index UNIQUE(_parent_oid,m_start,m_start_ms,m_waveformID_networkCode,m_waveformID_stationCode,m_waveformID_locationCode,m_waveformID_channelCode,m_waveformID_resourceURI)
);

CREATE INDEX QCLog__parent_oid ON QCLog(_parent_oid);

CREATE TRIGGER QCLog_update BEFORE UPDATE ON QCLog FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE WaveformQuality (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_creatorID VARCHAR(255) NOT NULL,
	m_created TIMESTAMP NOT NULL,
	m_created_ms INTEGER NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_type VARCHAR(80) NOT NULL,
	m_parameter VARCHAR(80) NOT NULL,
	m_value DOUBLE PRECISION NOT NULL,
	m_lowerUncertainty DOUBLE PRECISION,
	m_upperUncertainty DOUBLE PRECISION,
	m_windowLength DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT waveformquality_composite_index UNIQUE(_parent_oid,m_start,m_start_ms,m_waveformID_networkCode,m_waveformID_stationCode,m_waveformID_locationCode,m_waveformID_channelCode,m_waveformID_resourceURI,m_type,m_parameter)
);

CREATE INDEX WaveformQuality__parent_oid ON WaveformQuality(_parent_oid);
CREATE INDEX WaveformQuality_m_start_m_start_ms ON WaveformQuality(m_start,m_start_ms);
CREATE INDEX WaveformQuality_m_end_m_end_ms ON WaveformQuality(m_end,m_end_ms);

CREATE TRIGGER WaveformQuality_update BEFORE UPDATE ON WaveformQuality FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Outage (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_waveformID_networkCode VARCHAR(8) NOT NULL,
	m_waveformID_stationCode VARCHAR(8) NOT NULL,
	m_waveformID_locationCode VARCHAR(8),
	m_waveformID_channelCode VARCHAR(8),
	m_waveformID_resourceURI VARCHAR(255),
	m_creatorID VARCHAR(255) NOT NULL,
	m_created TIMESTAMP NOT NULL,
	m_created_ms INTEGER NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT outage_composite_index UNIQUE(_parent_oid,m_waveformID_networkCode,m_waveformID_stationCode,m_waveformID_locationCode,m_waveformID_channelCode,m_waveformID_resourceURI,m_start,m_start_ms)
);

CREATE INDEX Outage__parent_oid ON Outage(_parent_oid);

CREATE TRIGGER Outage_update BEFORE UPDATE ON Outage FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE StationReference (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_stationID VARCHAR(255) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT stationreference_composite_index UNIQUE(_parent_oid,m_stationID)
);

CREATE INDEX StationReference__parent_oid ON StationReference(_parent_oid);
CREATE INDEX StationReference_m_stationID ON StationReference(m_stationID);

CREATE TRIGGER StationReference_update BEFORE UPDATE ON StationReference FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE StationGroup (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_type VARCHAR(64),
	m_code VARCHAR(20),
	m_start TIMESTAMP,
	m_start_ms INTEGER,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_description VARCHAR(255),
	m_latitude DOUBLE PRECISION,
	m_longitude DOUBLE PRECISION,
	m_elevation DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT stationgroup_composite_index UNIQUE(_parent_oid,m_code)
);

CREATE INDEX StationGroup__parent_oid ON StationGroup(_parent_oid);

CREATE TRIGGER StationGroup_update BEFORE UPDATE ON StationGroup FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE AuxSource (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255) NOT NULL,
	m_description VARCHAR(255),
	m_unit VARCHAR(20),
	m_conversion VARCHAR(80),
	m_sampleRateNumerator INT,
	m_sampleRateDenominator INT,
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT auxsource_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX AuxSource__parent_oid ON AuxSource(_parent_oid);

CREATE TRIGGER AuxSource_update BEFORE UPDATE ON AuxSource FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE AuxDevice (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255) NOT NULL,
	m_description VARCHAR(255),
	m_model VARCHAR(80),
	m_manufacturer VARCHAR(50),
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT auxdevice_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX AuxDevice__parent_oid ON AuxDevice(_parent_oid);

CREATE TRIGGER AuxDevice_update BEFORE UPDATE ON AuxDevice FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE SensorCalibration (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_serialNumber VARCHAR(80) NOT NULL,
	m_channel INT NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT sensorcalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms)
);

CREATE INDEX SensorCalibration__parent_oid ON SensorCalibration(_parent_oid);

CREATE TRIGGER SensorCalibration_update BEFORE UPDATE ON SensorCalibration FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Sensor (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255) NOT NULL,
	m_description VARCHAR(255),
	m_model VARCHAR(80),
	m_manufacturer VARCHAR(50),
	m_type VARCHAR(10),
	m_unit VARCHAR(20),
	m_lowFrequency DOUBLE PRECISION,
	m_highFrequency DOUBLE PRECISION,
	m_response VARCHAR(255),
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT sensor_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX Sensor__parent_oid ON Sensor(_parent_oid);

CREATE TRIGGER Sensor_update BEFORE UPDATE ON Sensor FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ResponsePAZ (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_type VARCHAR(1),
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_normalizationFactor DOUBLE PRECISION,
	m_normalizationFrequency DOUBLE PRECISION,
	m_numberOfZeros SMALLINT,
	m_numberOfPoles SMALLINT,
	m_zeros_content BYTEA,
	m_zeros_used BOOLEAN NOT NULL DEFAULT '0',
	m_poles_content BYTEA,
	m_poles_used BOOLEAN NOT NULL DEFAULT '0',
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	m_decimationFactor SMALLINT,
	m_delay DOUBLE PRECISION,
	m_correction DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT responsepaz_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX ResponsePAZ__parent_oid ON ResponsePAZ(_parent_oid);

CREATE TRIGGER ResponsePAZ_update BEFORE UPDATE ON ResponsePAZ FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ResponsePolynomial (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_frequencyUnit VARCHAR(1),
	m_approximationType VARCHAR(1),
	m_approximationLowerBound DOUBLE PRECISION,
	m_approximationUpperBound DOUBLE PRECISION,
	m_approximationError DOUBLE PRECISION,
	m_numberOfCoefficients SMALLINT,
	m_coefficients_content BYTEA,
	m_coefficients_used BOOLEAN NOT NULL DEFAULT '0',
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT responsepolynomial_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX ResponsePolynomial__parent_oid ON ResponsePolynomial(_parent_oid);

CREATE TRIGGER ResponsePolynomial_update BEFORE UPDATE ON ResponsePolynomial FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ResponseFAP (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_numberOfTuples SMALLINT,
	m_tuples_content BYTEA,
	m_tuples_used BOOLEAN NOT NULL DEFAULT '0',
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT responsefap_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX ResponseFAP__parent_oid ON ResponseFAP(_parent_oid);

CREATE TRIGGER ResponseFAP_update BEFORE UPDATE ON ResponseFAP FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ResponseFIR (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_decimationFactor SMALLINT,
	m_delay DOUBLE PRECISION,
	m_correction DOUBLE PRECISION,
	m_numberOfCoefficients SMALLINT,
	m_symmetry VARCHAR(1),
	m_coefficients_content BYTEA,
	m_coefficients_used BOOLEAN NOT NULL DEFAULT '0',
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT responsefir_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX ResponseFIR__parent_oid ON ResponseFIR(_parent_oid);

CREATE TRIGGER ResponseFIR_update BEFORE UPDATE ON ResponseFIR FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ResponseIIR (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_type VARCHAR(1),
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_decimationFactor SMALLINT,
	m_delay DOUBLE PRECISION,
	m_correction DOUBLE PRECISION,
	m_numberOfNumerators SMALLINT,
	m_numberOfDenominators SMALLINT,
	m_numerators_content BYTEA,
	m_numerators_used BOOLEAN NOT NULL DEFAULT '0',
	m_denominators_content BYTEA,
	m_denominators_used BOOLEAN NOT NULL DEFAULT '0',
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT responseiir_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX ResponseIIR__parent_oid ON ResponseIIR(_parent_oid);

CREATE TRIGGER ResponseIIR_update BEFORE UPDATE ON ResponseIIR FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE DataloggerCalibration (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_serialNumber VARCHAR(80) NOT NULL,
	m_channel INT NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT dataloggercalibration_composite_index UNIQUE(_parent_oid,m_serialNumber,m_channel,m_start,m_start_ms)
);

CREATE INDEX DataloggerCalibration__parent_oid ON DataloggerCalibration(_parent_oid);

CREATE TRIGGER DataloggerCalibration_update BEFORE UPDATE ON DataloggerCalibration FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Decimation (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_sampleRateNumerator INT NOT NULL,
	m_sampleRateDenominator INT NOT NULL,
	m_analogueFilterChain_content BYTEA,
	m_analogueFilterChain_used BOOLEAN NOT NULL DEFAULT '0',
	m_digitalFilterChain_content BYTEA,
	m_digitalFilterChain_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT decimation_composite_index UNIQUE(_parent_oid,m_sampleRateNumerator,m_sampleRateDenominator)
);

CREATE INDEX Decimation__parent_oid ON Decimation(_parent_oid);

CREATE TRIGGER Decimation_update BEFORE UPDATE ON Decimation FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Datalogger (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(255),
	m_description VARCHAR(255),
	m_digitizerModel VARCHAR(80),
	m_digitizerManufacturer VARCHAR(50),
	m_recorderModel VARCHAR(80),
	m_recorderManufacturer VARCHAR(50),
	m_clockModel VARCHAR(80),
	m_clockManufacturer VARCHAR(50),
	m_clockType VARCHAR(10),
	m_gain DOUBLE PRECISION,
	m_maxClockDrift DOUBLE PRECISION,
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT datalogger_composite_index UNIQUE(_parent_oid,m_name)
);

CREATE INDEX Datalogger__parent_oid ON Datalogger(_parent_oid);

CREATE TRIGGER Datalogger_update BEFORE UPDATE ON Datalogger FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE AuxStream (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_code VARCHAR(3) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_device VARCHAR(255),
	m_deviceSerialNumber VARCHAR(80),
	m_source VARCHAR(80),
	m_format VARCHAR(50),
	m_flags VARCHAR(20),
	m_restricted BOOLEAN,
	m_shared BOOLEAN,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT auxstream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms)
);

CREATE INDEX AuxStream__parent_oid ON AuxStream(_parent_oid);

CREATE TRIGGER AuxStream_update BEFORE UPDATE ON AuxStream FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Stream (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_code VARCHAR(3) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_datalogger VARCHAR(255),
	m_dataloggerSerialNumber VARCHAR(80),
	m_dataloggerChannel INT,
	m_sensor VARCHAR(255),
	m_sensorSerialNumber VARCHAR(80),
	m_sensorChannel INT,
	m_clockSerialNumber VARCHAR(80),
	m_sampleRateNumerator INT,
	m_sampleRateDenominator INT,
	m_depth DOUBLE PRECISION,
	m_azimuth DOUBLE PRECISION,
	m_dip DOUBLE PRECISION,
	m_gain DOUBLE PRECISION,
	m_gainFrequency DOUBLE PRECISION,
	m_gainUnit VARCHAR(20),
	m_format VARCHAR(50),
	m_flags VARCHAR(20),
	m_restricted BOOLEAN,
	m_shared BOOLEAN,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT stream_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms)
);

CREATE INDEX Stream__parent_oid ON Stream(_parent_oid);

CREATE TRIGGER Stream_update BEFORE UPDATE ON Stream FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE SensorLocation (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_code VARCHAR(8) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_latitude DOUBLE PRECISION,
	m_longitude DOUBLE PRECISION,
	m_elevation DOUBLE PRECISION,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT sensorlocation_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms)
);

CREATE INDEX SensorLocation__parent_oid ON SensorLocation(_parent_oid);

CREATE TRIGGER SensorLocation_update BEFORE UPDATE ON SensorLocation FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Station (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_code VARCHAR(8) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_description VARCHAR(255),
	m_latitude DOUBLE PRECISION,
	m_longitude DOUBLE PRECISION,
	m_elevation DOUBLE PRECISION,
	m_place VARCHAR(80),
	m_country VARCHAR(50),
	m_affiliation VARCHAR(50),
	m_type VARCHAR(50),
	m_archive VARCHAR(20),
	m_archiveNetworkCode VARCHAR(8),
	m_restricted BOOLEAN,
	m_shared BOOLEAN,
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT station_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms)
);

CREATE INDEX Station__parent_oid ON Station(_parent_oid);

CREATE TRIGGER Station_update BEFORE UPDATE ON Station FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Network (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_code VARCHAR(8) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP,
	m_end_ms INTEGER,
	m_description VARCHAR(255),
	m_institutions VARCHAR(100),
	m_region VARCHAR(100),
	m_type VARCHAR(50),
	m_netClass VARCHAR(1),
	m_archive VARCHAR(20),
	m_restricted BOOLEAN,
	m_shared BOOLEAN,
	m_remark_content BYTEA,
	m_remark_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT network_composite_index UNIQUE(_parent_oid,m_code,m_start,m_start_ms)
);

CREATE INDEX Network__parent_oid ON Network(_parent_oid);

CREATE TRIGGER Network_update BEFORE UPDATE ON Network FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE RouteArclink (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_address VARCHAR(50) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_end TIMESTAMP,
	m_priority SMALLINT,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT routearclink_composite_index UNIQUE(_parent_oid,m_address,m_start)
);

CREATE INDEX RouteArclink__parent_oid ON RouteArclink(_parent_oid);

CREATE TRIGGER RouteArclink_update BEFORE UPDATE ON RouteArclink FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE RouteSeedlink (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_address VARCHAR(50) NOT NULL,
	m_priority SMALLINT,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT routeseedlink_composite_index UNIQUE(_parent_oid,m_address)
);

CREATE INDEX RouteSeedlink__parent_oid ON RouteSeedlink(_parent_oid);

CREATE TRIGGER RouteSeedlink_update BEFORE UPDATE ON RouteSeedlink FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Route (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_networkCode VARCHAR(8) NOT NULL,
	m_stationCode VARCHAR(8) NOT NULL,
	m_locationCode VARCHAR(8) NOT NULL,
	m_streamCode VARCHAR(8) NOT NULL,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT route_composite_index UNIQUE(_parent_oid,m_networkCode,m_stationCode,m_locationCode,m_streamCode)
);

CREATE INDEX Route__parent_oid ON Route(_parent_oid);

CREATE TRIGGER Route_update BEFORE UPDATE ON Route FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE Access (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_networkCode VARCHAR(8) NOT NULL,
	m_stationCode VARCHAR(8) NOT NULL,
	m_locationCode VARCHAR(8) NOT NULL,
	m_streamCode VARCHAR(8) NOT NULL,
	m_user VARCHAR(50) NOT NULL,
	m_start TIMESTAMP NOT NULL,
	m_end TIMESTAMP,
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT access_composite_index UNIQUE(_parent_oid,m_networkCode,m_stationCode,m_locationCode,m_streamCode,m_user,m_start)
);

CREATE INDEX Access__parent_oid ON Access(_parent_oid);

CREATE TRIGGER Access_update BEFORE UPDATE ON Access FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE JournalEntry (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_created TIMESTAMP,
	m_created_ms INTEGER,
	m_objectID VARCHAR(255) NOT NULL,
	m_sender VARCHAR(80) NOT NULL,
	m_action VARCHAR(160) NOT NULL,
	m_parameters VARCHAR(160),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE
);

CREATE INDEX JournalEntry__parent_oid ON JournalEntry(_parent_oid);
CREATE INDEX JournalEntry_m_objectID ON JournalEntry(m_objectID);

CREATE TRIGGER JournalEntry_update BEFORE UPDATE ON JournalEntry FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ArclinkUser (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_name VARCHAR(80) NOT NULL,
	m_email VARCHAR(80),
	m_password VARCHAR(80),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT arclinkuser_composite_index UNIQUE(_parent_oid,m_name,m_email)
);

CREATE INDEX ArclinkUser__parent_oid ON ArclinkUser(_parent_oid);

CREATE TRIGGER ArclinkUser_update BEFORE UPDATE ON ArclinkUser FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ArclinkStatusLine (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_type VARCHAR(80) NOT NULL,
	m_status VARCHAR(160) NOT NULL,
	m_size INT,
	m_message VARCHAR(160),
	m_volumeID VARCHAR(80),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT arclinkstatusline_composite_index UNIQUE(_parent_oid,m_volumeID,m_type,m_status)
);

CREATE INDEX ArclinkStatusLine__parent_oid ON ArclinkStatusLine(_parent_oid);

CREATE TRIGGER ArclinkStatusLine_update BEFORE UPDATE ON ArclinkStatusLine FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ArclinkRequestLine (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_start TIMESTAMP NOT NULL,
	m_start_ms INTEGER NOT NULL,
	m_end TIMESTAMP NOT NULL,
	m_end_ms INTEGER NOT NULL,
	m_streamID_networkCode VARCHAR(8) NOT NULL,
	m_streamID_stationCode VARCHAR(8) NOT NULL,
	m_streamID_locationCode VARCHAR(8),
	m_streamID_channelCode VARCHAR(8),
	m_streamID_resourceURI VARCHAR(255),
	m_restricted BOOLEAN,
	m_shared BOOLEAN,
	m_netClass VARCHAR(1),
	m_constraints VARCHAR(160),
	m_status_type VARCHAR(80) NOT NULL,
	m_status_status VARCHAR(160) NOT NULL,
	m_status_size INT,
	m_status_message VARCHAR(160),
	m_status_volumeID VARCHAR(80),
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT arclinkrequestline_composite_index UNIQUE(_parent_oid,m_start,m_start_ms,m_end,m_end_ms,m_streamID_networkCode,m_streamID_stationCode,m_streamID_locationCode,m_streamID_channelCode,m_streamID_resourceURI)
);

CREATE INDEX ArclinkRequestLine__parent_oid ON ArclinkRequestLine(_parent_oid);

CREATE TRIGGER ArclinkRequestLine_update BEFORE UPDATE ON ArclinkRequestLine FOR EACH ROW EXECUTE PROCEDURE update_modified();


CREATE TABLE ArclinkRequest (
	_oid BIGINT NOT NULL,
	_parent_oid BIGINT NOT NULL,
	_last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	m_requestID VARCHAR(255) NOT NULL,
	m_userID VARCHAR(80) NOT NULL,
	m_userIP VARCHAR(16),
	m_clientID VARCHAR(80),
	m_clientIP VARCHAR(16),
	m_type VARCHAR(80) NOT NULL,
	m_created TIMESTAMP NOT NULL,
	m_created_ms INTEGER NOT NULL,
	m_status VARCHAR(80) NOT NULL,
	m_message VARCHAR(160),
	m_label VARCHAR(80),
	m_header VARCHAR(160),
	m_summary_okLineCount INT,
	m_summary_totalLineCount INT,
	m_summary_averageTimeWindow INT,
	m_summary_used BOOLEAN NOT NULL DEFAULT '0',
	PRIMARY KEY(_oid),
	FOREIGN KEY(_oid)
		REFERENCES Object(_oid)
		ON DELETE CASCADE,
	CONSTRAINT arclinkrequest_composite_index UNIQUE(_parent_oid,m_created,m_created_ms,m_requestID,m_userID)
);

CREATE INDEX ArclinkRequest__parent_oid ON ArclinkRequest(_parent_oid);

CREATE TRIGGER ArclinkRequest_update BEFORE UPDATE ON ArclinkRequest FOR EACH ROW EXECUTE PROCEDURE update_modified();


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


CREATE OR REPLACE FUNCTION Fix_bytea_output() RETURNS void
AS $BODY$
BEGIN
	IF (SELECT current_setting('server_version_num'))::int >= 90000 THEN
		EXECUTE('ALTER DATABASE ' || current_database() || ' SET bytea_output TO ''escape''');
	END IF;
END;
$BODY$ LANGUAGE plpgsql;

SELECT Fix_bytea_output();
