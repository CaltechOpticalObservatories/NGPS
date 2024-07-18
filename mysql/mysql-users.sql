/* 
User permissions are set on a “need to have” basis to increase security and reduce accidental data loss.  
Except for root, user passwords will generally be exposed in the code.

•	root
•	maybe we need a secure user with read-only on all tables?
•	telemlogger – write-only to any table in telemetry
•	obseq – permissions needed by OS on ngps
•	gui – most permissions allowed on ngps
•	grafana – read only from telemetry
•	drp – write-only to science

If EXECUTE is needed, must apply to db not 1 table:
GRANT EXECUTE ON db.* TO user@'%';
*/

-- grafana
CREATE USER grafana@localhost IDENTIFIED BY 'Write1Spec2.!';
GRANT SELECT on telemetry.* TO grafana@localhost;
SHOW GRANTS FOR grafana@localhost;

-- obseq
CREATE USER obseq@localhost IDENTIFIED BY 'Write1Spec2.!';
GRANT SELECT on ngps.target_sets TO obseq@localhost;
GRANT SELECT, UPDATE, INSERT on ngps.targets TO obseq@localhost;
GRANT INSERT on ngps.completed_observations TO obseq@localhost; -- DEPRECATED
GRANT INSERT ON ngps.completed_obs TO obseq@localhost;
SHOW GRANTS FOR obseq@localhost;

-- gui
CREATE USER gui@localhost IDENTIFIED BY 'Write1Spec2.!';
GRANT ALL ON ngps.owner TO gui@localhost;
GRANT ALL ON ngps.targets TO gui@localhost;
GRANT ALL ON ngps.target_sets TO gui@localhost;
GRANT SELECT ON ngps.completed_observations TO gui@localhost;
SHOW GRANTS FOR gui@localhost;

-- telemlogger
CREATE USER telemlogger@localhost IDENTIFIED BY 'Password-123';
GRANT CREATE, INSERT ON telemetry.* TO telemlogger@localhost;
SHOW GRANTS FOR telemlogger@localhost;

