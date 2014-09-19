/* host+port combined identify the Arclink server for which these
 * statistics were generated
 */

/* DROP TABLES WHERE name LIKE 'ArcStats%'; */
DROP TABLE IF EXISTS ArcStatsSource;
DROP TABLE IF EXISTS ArcStatsReport;
DROP TABLE IF EXISTS ArcStatsSummary;
DROP TABLE IF EXISTS ArcStatsUser;
DROP TABLE IF EXISTS ArcStatsRequest;
DROP TABLE IF EXISTS ArcStatsVolume;
DROP TABLE IF EXISTS ArcStatsStation;
DROP TABLE IF EXISTS ArcStatsNetwork;
DROP TABLE IF EXISTS ArcStatsMessages;
DROP TABLE IF EXISTS ArcStatsUserIP;
DROP TABLE IF EXISTS ArcStatsClientIP;


/* Key to a particular reporting source: primary key is (host, port, dcid) */
CREATE TABLE ArcStatsSource (
    id INTEGER PRIMARY KEY,
    host VARCHAR(255),
    port INT UNSIGNED,
    dcid VARCHAR(10),
    description VARCHAR(255));

CREATE TABLE ArcStatsReport (
    start_day DATETIME NOT NULL,
    start_time DATETIME NOT NULL,
    end_time DATETIME,
    /* tables ... pointers to all its tables by _oid */
    /* where did this report come from? */
    reporter VARCHAR(255));

/* This table and ArcStatsReport might be merged (normalized?) */
CREATE TABLE ArcStatsSummary (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    requests INT UNSIGNED,
    requests_with_errors INT UNSIGNED,
    error_count INT UNSIGNED,
    users INT UNSIGNED,
    /* Assert: count(*) from ArcStatsUsers with start_day, host, port matching == users */
    stations INT UNSIGNED,
    total_lines INT UNSIGNED,
    total_size INT UNSIGNED,
    PRIMARY KEY (src, start_day)
);

CREATE TABLE ArcStatsUser (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    userID VARCHAR(80) NOT NULL,
    requests INT UNSIGNED,
    lines INT UNSIGNED,
    errors INT UNSIGNED,
    /* some long int type FIXME */
    size BIGINT,
    PRIMARY KEY (src, start_day, userID)
);

CREATE TABLE ArcStatsRequest (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    type VARCHAR(80),
    requests INT UNSIGNED,
    lines INT UNSIGNED,
    nodata INT UNSIGNED,
    errors INT UNSIGNED,
    size INT UNSIGNED default 0,
    PRIMARY KEY (src, start_day, type)
    );

CREATE TABLE ArcStatsVolume (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    type VARCHAR(80),
    requests INT UNSIGNED,
    /* called "count" in webreqlog.py */
    lines INT UNSIGNED,
    /* not currently displayed, but why not? */
    errors INT UNSIGNED,
    size INT UNSIGNED default 0,
    PRIMARY KEY (src, start_day, type)
    );

CREATE TABLE ArcStatsStation (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    streamID_networkCode CHAR(8) NOT NULL,
    streamID_stationCode CHAR(8) NOT NULL,
    streamID_locationCode CHAR(8),
    streamID_channelCode CHAR(8),
    requests INT UNSIGNED,
    lines INT UNSIGNED,    /* not currently displayed, but why not? */
    errors INT UNSIGNED,
    size INT UNSIGNED default 0,
    time INT UNSIGNED default 0
);

CREATE TABLE ArcStatsNetwork (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    networkCode CHAR(8) NOT NULL,
    requests INT UNSIGNED,
    lines INT UNSIGNED,
    nodata INT UNSIGNED,
    errors INT UNSIGNED,
    size INT UNSIGNED default 0,
    time INT UNSIGNED default 0
);

CREATE TABLE ArcStatsMessages (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    message VARCHAR(160),
    count INT UNSIGNED default 0
);

CREATE TABLE ArcStatsUserIP (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    userIP VARCHAR(16),
    /* Okay for IP v4 but not v6. */
    requests INT UNSIGNED,
    lines INT UNSIGNED,  
    errors INT UNSIGNED,
    size INT UNSIGNED default 0,
    /* not currently displayed, but why not? */
    PRIMARY KEY (src, start_day, userIP)
);

CREATE TABLE ArcStatsClientIP (
    start_day DATETIME NOT NULL,
    src INT UNSIGNED NOT NULL,
    clientIP VARCHAR(16),
    /* Okay for IP v4 but not v6. */
    requests INT UNSIGNED,
    lines INT UNSIGNED,  
    errors INT UNSIGNED,
    size INT UNSIGNED default 0,
    /* not currently displayed, but why not? */
    PRIMARY KEY (src, start_day, clientIP)
);
