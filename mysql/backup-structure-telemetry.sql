-- MySQL dump 10.13  Distrib 8.0.26, for Linux (x86_64)
--
-- Host: localhost    Database: telemetry
-- ------------------------------------------------------
-- Server version	8.0.26

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `acamd`
--

DROP TABLE IF EXISTS `acamd`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `acamd` (
  `datetime` varchar(23) NOT NULL,
  PRIMARY KEY (`datetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `detector_temps`
--

DROP TABLE IF EXISTS `detector_temps`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
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
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `flexured`
--

DROP TABLE IF EXISTS `flexured`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
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
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `focusd`
--

DROP TABLE IF EXISTS `focusd`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `focusd` (
  `datetime` varchar(23) NOT NULL,
  `T_U` decimal(8,3) DEFAULT NULL COMMENT 'Structure temperature (K)',
  `T_G` decimal(8,3) DEFAULT NULL COMMENT 'Structure temperature (K)',
  `T_R` decimal(8,3) DEFAULT NULL COMMENT 'Structure temperature (K)',
  `T_I` decimal(8,3) DEFAULT NULL COMMENT 'Structure temperature (K)',
  `T_air` decimal(8,3) DEFAULT NULL COMMENT 'Air temperature (K)',
  `ShiftFocus_U` decimal(8,3) DEFAULT NULL COMMENT 'Predicted shift in focus position (µm)',
  `ShiftFocus_G` decimal(8,3) DEFAULT NULL COMMENT 'Predicted shift in focus position (µm)',
  `ShiftFocus_R` decimal(8,3) DEFAULT NULL COMMENT 'Predicted shift in focus position (µm)',
  `ShiftFocus_I` decimal(8,3) DEFAULT NULL COMMENT 'Predicted shift in focus position (µm)',
  `Offset_U` decimal(8,3) DEFAULT NULL COMMENT 'Focus motor offset from reference position (µm)',
  `Offset_G` decimal(8,3) DEFAULT NULL COMMENT 'Focus motor offset from reference position (µm)',
  `Offset_R` decimal(8,3) DEFAULT NULL COMMENT 'Focus motor offset from reference position (µm)',
  `Offset_I` decimal(8,3) DEFAULT NULL COMMENT 'Focus motor offset from reference position (µm)',
  `T0_U` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) reference temperature (K)',
  `T0_G` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) reference temperature (K)',
  `T0_R` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) reference temperature (K)',
  `T0_I` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) reference temperature (K)',
  `MotorPos_U` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) Motor position at T0 (µm)',
  `MotorPos_G` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) Motor position at T0 (µm)',
  `MotorPos_R` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) Motor position at T0 (µm)',
  `MotorPos_I` decimal(8,3) DEFAULT NULL COMMENT '(CONSTANT) Motor position at T0 (µm)',
  PRIMARY KEY (`datetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `thermald`
--

DROP TABLE IF EXISTS `thermald`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `thermald` (
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
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `thermaldx`
--

DROP TABLE IF EXISTS `thermaldx`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `thermaldx` (
  `datetime` datetime(3) NOT NULL,
  `TCCD_U` decimal(7,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `TCCD_G` decimal(7,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `TCCD_R` decimal(7,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `TCCD_I` decimal(7,3) DEFAULT NULL COMMENT 'CCD temperature (K)',
  `Tdewar_U` decimal(7,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `Tdewar_G` decimal(7,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `Tdewar_R` decimal(7,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `Tdewar_I` decimal(7,3) DEFAULT NULL COMMENT 'dewar temperature (K)',
  `heat_U` decimal(7,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  `heat_G` decimal(7,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  `heat_R` decimal(7,3) DEFAULT NULL COMMENT 'CCD heater power (%)',
  `heat_I` tinyint(1) DEFAULT NULL COMMENT 'CCD heater power (%)',
  PRIMARY KEY (`datetime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-07-17 10:10:05
