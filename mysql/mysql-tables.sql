/*
Scratch -- work in progress
*/

CREATE TABLE thermald(
   datetime DATETIME(3) NOT NULL PRIMARY KEY
  ,TCCD_U  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,TCCD_G  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,TCCD_R  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,TCCD_I  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,Tdewar_U  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,Tdewar_G  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,Tdewar_R  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,Tdewar_I  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,heat_U  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
  ,heat_G  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
  ,heat_R  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
  ,heat_I  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
);
SHOW FULL COLUMNS FROM thermald;

-- Dummy table
-- CREATE TABLE focusd(datetime VARCHAR(23) NOT NULL PRIMARY KEY);

-- Inserting a row
-- cmd="INSERT INTO tablename(datetime,col1,col2,…) VALUES (\"10/19/23 21:30:17\",val1,val2,…);"
-- mysql -u telemlogger -pPassword-123 -Dtelemetry --execute="$cmd"

/*
CREATE TABLE thermaldx(
   datetime DATETIME(3) NOT NULL PRIMARY KEY
  ,TCCD_U  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,TCCD_G  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,TCCD_R  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,TCCD_I  NUMERIC(7,3) COMMENT 'CCD temperature (K)'
  ,Tdewar_U  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,Tdewar_G  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,Tdewar_R  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,Tdewar_I  NUMERIC(7,3) COMMENT 'dewar temperature (K)'
  ,heat_U  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
  ,heat_G  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
  ,heat_R  NUMERIC(7,3) COMMENT 'CCD heater power (%)'
  ,heat_I  BOOL COMMENT 'CCD heater power (%)'
);
SHOW FULL COLUMNS FROM thermaldx;
*/
