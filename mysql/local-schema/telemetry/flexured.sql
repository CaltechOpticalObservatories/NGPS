CREATE TABLE `flexured` (
  `datetime` varchar(23) NOT NULL,
  `alt` decimal(8,3) DEFAULT NULL COMMENT 'TCS altitude (deg)',
  `az` decimal(8,3) DEFAULT NULL COMMENT 'TCS azimuth (deg)',
  `z` decimal(8,3) DEFAULT NULL COMMENT 'TCS zenith angle (90 deg - alt)',
  `casangle` decimal(8,3) DEFAULT NULL COMMENT 'Cassegrain angle (deg)',
  `ShiftDisp_U` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in dispersion direction (µm)',
  `ShiftDisp_G` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in dispersion direction (µm)',
  `ShiftDisp_R` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in dispersion direction (µm)',
  `ShiftDisp_I` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in dispersion direction (µm)',
  `ShiftSpat_U` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in spatial direction (µm)',
  `ShiftSpat_G` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in spatial direction (µm)',
  `ShiftSpat_R` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in spatial direction (µm)',
  `ShiftSpat_I` decimal(8,3) DEFAULT NULL COMMENT 'Predicted spectrum shift in spatial direction (µm)',
  `pztip_U` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tip setting (unit?)',
  `pztip_G` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tip setting (unit?)',
  `pztip_R` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tip setting (unit?)',
  `pztip_I` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tip setting (unit?)',
  `pztilt_U` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tilt setting (unit?)',
  `pztilt_G` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tilt setting (unit?)',
  `pztilt_R` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tilt setting (unit?)',
  `pztilt_I` decimal(8,3) DEFAULT NULL COMMENT 'Piezo tilt setting (unit?)',
  `pzpist_U` decimal(8,3) DEFAULT NULL COMMENT 'Piezo piston setting (unit?)',
  `pzpist_G` decimal(8,3) DEFAULT NULL COMMENT 'Piezo piston setting (unit?)',
  `pzpist_R` decimal(8,3) DEFAULT NULL COMMENT 'Piezo piston setting (unit?)',
  `pzpist_I` decimal(8,3) DEFAULT NULL COMMENT 'Piezo piston setting (unit?)',
  `pztip0_U` decimal(8,3) DEFAULT NULL COMMENT 'Tip reference position at zenith (unit?)',
  `pztip0_G` decimal(8,3) DEFAULT NULL COMMENT 'Tip reference position at zenith (unit?)',
  `pztip0_R` decimal(8,3) DEFAULT NULL COMMENT 'Tip reference position at zenith (unit?)',
  `pztip0_I` decimal(8,3) DEFAULT NULL COMMENT 'Tip reference position at zenith (unit?)',
  `pztilt0_U` decimal(8,3) DEFAULT NULL COMMENT 'Tilt reference position at zenith (unit?)',
  `pztilt0_G` decimal(8,3) DEFAULT NULL COMMENT 'Tilt reference position at zenith (unit?)',
  `pztilt0_R` decimal(8,3) DEFAULT NULL COMMENT 'Tilt reference position at zenith (unit?)',
  `pztilt0_I` decimal(8,3) DEFAULT NULL COMMENT 'Tilt reference position at zenith (unit?)',
  `pzpist0_U` decimal(8,3) DEFAULT NULL COMMENT 'Piston reference position at zenith (unit?)',
  `pzpist0_G` decimal(8,3) DEFAULT NULL COMMENT 'Piston reference position at zenith (unit?)',
  `pzpist0_R` decimal(8,3) DEFAULT NULL COMMENT 'Piston reference position at zenith (unit?)',
  `pzpist0_I` decimal(8,3) DEFAULT NULL COMMENT 'Piston reference position at zenith (unit?)',
  PRIMARY KEY (`datetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;