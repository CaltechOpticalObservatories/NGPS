CREATE TABLE `thermald_old` (
  `datetime` datetime(3) NOT NULL,
  `TCCD_U` decimal(8,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `TCCD_G` decimal(8,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `TCCD_R` decimal(8,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `TCCD_I` decimal(8,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `Tdewar_U` decimal(8,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `Tdewar_G` decimal(8,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `Tdewar_R` decimal(8,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `Tdewar_I` decimal(8,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `heat_U` decimal(8,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  `heat_G` decimal(8,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  `heat_R` decimal(8,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  `heat_I` decimal(8,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  PRIMARY KEY (`datetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
