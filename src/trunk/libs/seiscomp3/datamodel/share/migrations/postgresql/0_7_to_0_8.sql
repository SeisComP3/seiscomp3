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
  UNIQUE(_parent_oid,m_name)
);

UPDATE Meta SET value='0.8' WHERE name='Schema-Version';
