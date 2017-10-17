CREATE TABLE ResponseFAP (
  _oid INTEGER(11) NOT NULL,
  _parent_oid INTEGER(11) NOT NULL,
  _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  name VARCHAR(255),
  gain DOUBLE UNSIGNED,
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
  UNIQUE(_parent_oid,name)
) ENGINE=INNODB;

UPDATE Meta SET value='0.8' WHERE name='Schema-Version';
