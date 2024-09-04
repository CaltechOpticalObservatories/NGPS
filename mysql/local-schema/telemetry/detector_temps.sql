CREATE TABLE `detector_temps` (
  `datetime` varchar(17) NOT NULL,
  `AU_CCD` decimal(6,3) NOT NULL,
  `BU_dewar` decimal(6,3) NOT NULL,
  `CR_CCD` decimal(8,3) NOT NULL,
  `DR_dewar` decimal(7,3) NOT NULL,
  `1U_heat` decimal(6,3) NOT NULL,
  `2R_heat` decimal(7,3) NOT NULL,
  `AG_CCD` decimal(6,3) NOT NULL,
  `BG_dewar` decimal(6,3) NOT NULL,
  `CI_CCD` decimal(8,3) NOT NULL,
  `DI_dewar` decimal(6,3) NOT NULL,
  `1G_heat` decimal(6,3) NOT NULL,
  `2I_heat` decimal(6,3) NOT NULL,
  PRIMARY KEY (`datetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
