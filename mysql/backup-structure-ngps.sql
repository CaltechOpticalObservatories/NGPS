-- MySQL dump 10.13  Distrib 8.0.26, for Linux (x86_64)
--
-- Host: localhost    Database: ngps
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
-- Table structure for table `completed_obs`
--

DROP TABLE IF EXISTS `completed_obs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `completed_obs` (
  `LOG_ID` int NOT NULL AUTO_INCREMENT,
  `OWNER` varchar(32) DEFAULT NULL COMMENT 'Name of target list owner',
  `OBSERVATION_ID` int DEFAULT NULL COMMENT 'ID from the master targets table',
  `SET_ID` int DEFAULT NULL COMMENT 'ID of the user''s target set',
  `TARGET_NUMBER` int DEFAULT NULL,
  `SEQUENCE_NUMBER` int DEFAULT NULL,
  `NAME` varchar(128) NOT NULL COMMENT 'Name of the astronomical target or calibration',
  `FITSFILE` varchar(512) DEFAULT NULL COMMENT 'File with the spectrum images',
  `RA` varchar(32) DEFAULT NULL COMMENT 'Right ascension',
  `DECL` varchar(32) DEFAULT NULL COMMENT 'Declination',
  `ALT` float DEFAULT NULL COMMENT 'Altitude (deg)',
  `AZ` float DEFAULT NULL COMMENT 'Azimuth (deg)',
  `AIRMASS` float DEFAULT NULL COMMENT 'Airmass',
  `CASANGLE` float DEFAULT NULL COMMENT 'Cassegrain angle of the P200',
  `SLITANGLE_REQ` varchar(8) DEFAULT NULL COMMENT 'Slit angle request',
  `POINTMODE` varchar(8) DEFAULT NULL COMMENT 'Where to place target (ACAM or SLIT)',
  `NOTBEFORE` datetime DEFAULT NULL COMMENT 'Earliest date/time to start exposure',
  `SLEW_START` datetime DEFAULT NULL COMMENT 'Slew start date/time',
  `EXPTIME` float DEFAULT NULL COMMENT 'Exposure time (s)',
  `EXPTIME_REQ` varchar(12) DEFAULT NULL COMMENT 'Exposure time request',
  `EXP_START` datetime DEFAULT NULL COMMENT 'Exposure start date/time',
  `EXP_END` datetime DEFAULT NULL COMMENT 'Exposure end date/time',
  `SLITWIDTH` float DEFAULT NULL COMMENT 'Spectrograph slit width (arcsec)',
  `SLITWIDTH_REQ` varchar(12) DEFAULT NULL COMMENT 'Slit width request',
  `SLITOFFSET` float DEFAULT NULL COMMENT 'Slit lateral offset (arcsec)',
  `BINSPECT` int DEFAULT NULL COMMENT 'CCD binning in spectral direction',
  `BINSPAT` int DEFAULT NULL COMMENT 'CCD binning in spatial direction',
  `OBSMODE` varchar(32) DEFAULT NULL COMMENT 'Observation mode, CCD settings',
  `NOTE` varchar(512) DEFAULT NULL COMMENT 'Observer''s note on this target',
  `OTMFLAG` varchar(20) DEFAULT NULL COMMENT 'OTM flag codes at time of exposure',
  PRIMARY KEY (`LOG_ID`,`NAME`),
  UNIQUE KEY `LOG_ID_UNIQUE` (`LOG_ID`)
) ENGINE=InnoDB AUTO_INCREMENT=81 DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `completed_observations`
--

DROP TABLE IF EXISTS `completed_observations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `completed_observations` (
  `OBSERVATION_LOG_ID` int NOT NULL AUTO_INCREMENT,
  `OBSERVATION_ID` int DEFAULT NULL,
  `SET_ID` int DEFAULT NULL,
  `TARGET_NUMBER` int DEFAULT NULL,
  `SEQUENCE_NUMBER` int DEFAULT NULL,
  `NAME` varchar(128) NOT NULL,
  `FITSFILE` varchar(512) DEFAULT NULL,
  `EXP_START` datetime DEFAULT NULL,
  `EXP_END` datetime DEFAULT NULL,
  `DURATION` int DEFAULT NULL,
  `RA` varchar(32) DEFAULT NULL,
  `DECL` varchar(32) DEFAULT NULL,
  `EPOCH` varchar(8) DEFAULT NULL,
  `EXPTIME` double DEFAULT NULL,
  `SLITWIDTH` double DEFAULT NULL,
  `SLITOFFSET` double DEFAULT NULL,
  `BINSPECT` int DEFAULT NULL,
  `BINSPAT` int DEFAULT NULL,
  `OBSMODE` varchar(32) DEFAULT NULL,
  `CASANGLE` float DEFAULT NULL,
  PRIMARY KEY (`OBSERVATION_LOG_ID`,`NAME`),
  UNIQUE KEY `OBSERVATION-LOG-ID_UNIQUE` (`OBSERVATION_LOG_ID`)
) ENGINE=InnoDB AUTO_INCREMENT=81 DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `owner`
--

DROP TABLE IF EXISTS `owner`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `owner` (
  `OWNER_ID` varchar(64) NOT NULL,
  `PROPOSAL_ID` varchar(64) DEFAULT NULL,
  `PROPOSAL_TITLE` varchar(128) DEFAULT NULL,
  `PASSWORD` varchar(64) DEFAULT NULL,
  PRIMARY KEY (`OWNER_ID`),
  UNIQUE KEY `OWNER_ID_UNIQUE` (`OWNER_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `target_sets`
--

DROP TABLE IF EXISTS `target_sets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `target_sets` (
  `SET_ID` int NOT NULL AUTO_INCREMENT,
  `SET_NAME` varchar(64) NOT NULL,
  `OWNER` varchar(45) DEFAULT NULL,
  `STATE` varchar(16) DEFAULT NULL,
  `NUM_OBSERVATIONS` int DEFAULT NULL,
  `SET_CREATION_TIMESTAMP` timestamp(1) NOT NULL,
  `LAST_UPDATE_TIMESTAMP` timestamp(1) NULL DEFAULT NULL,
  PRIMARY KEY (`SET_ID`),
  UNIQUE KEY `set_id_UNIQUE` (`SET_ID`)
) ENGINE=InnoDB AUTO_INCREMENT=120 DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `targets`
--

DROP TABLE IF EXISTS `targets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `targets` (
  `OBSERVATION_ID` int NOT NULL AUTO_INCREMENT,
  `SET_ID` int NOT NULL,
  `STATE` varchar(36) DEFAULT NULL,
  `OBS_ORDER` int NOT NULL,
  `TARGET_NUMBER` int NOT NULL,
  `SEQUENCE_NUMBER` int NOT NULL,
  `NAME` varchar(128) NOT NULL,
  `RA` varchar(32) DEFAULT NULL,
  `DECL` varchar(32) DEFAULT NULL,
  `OFFSET_RA` double DEFAULT NULL,
  `OFFSET_DEC` double DEFAULT NULL,
  `EXPTIME` varchar(32) NOT NULL,
  `SLITWIDTH` varchar(32) NOT NULL,
  `SLITOFFSET` double DEFAULT NULL,
  `OBSMODE` varchar(32) DEFAULT NULL,
  `BINSPECT` int DEFAULT NULL,
  `BINSPAT` int DEFAULT NULL,
  `SLITANGLE` varchar(32) DEFAULT NULL,
  `AIRMASS_MAX` varchar(32) DEFAULT NULL,
  `WRANGE_LOW` int DEFAULT NULL,
  `WRANGE_HIGH` int DEFAULT NULL,
  `CHANNEL` varchar(16) DEFAULT NULL,
  `MAGNITUDE` double DEFAULT NULL,
  `MAGSYSTEM` varchar(16) DEFAULT NULL,
  `MAGFILTER` varchar(16) DEFAULT NULL,
  `SRCMODEL` varchar(128) DEFAULT NULL,
  `OTMexpt` double DEFAULT NULL,
  `OTMslitwidth` double NOT NULL,
  `OTMcass` double DEFAULT NULL,
  `OTMairmass_start` double DEFAULT NULL,
  `OTMairmass_end` double DEFAULT NULL,
  `OTMsky` double DEFAULT NULL,
  `OTMdead` double DEFAULT NULL,
  `OTMslewgo` timestamp(1) NULL DEFAULT NULL,
  `OTMexp_start` timestamp(1) NULL DEFAULT NULL,
  `OTMexp_end` timestamp(1) NULL DEFAULT NULL,
  `OTMpa` double DEFAULT NULL,
  `OTMwait` double DEFAULT NULL,
  `OTMflag` varchar(32) DEFAULT NULL,
  `OTMlast` varchar(32) DEFAULT NULL,
  `OTMslew` double DEFAULT NULL,
  `OTMmoon` varchar(32) DEFAULT NULL,
  `OTMSNR` varchar(32) DEFAULT NULL,
  `OTMres` double DEFAULT NULL,
  `OTMseeing` double DEFAULT NULL,
  `OTMslitangle` double DEFAULT NULL,
  `NOTE` varchar(128) DEFAULT NULL,
  `COMMENT` varchar(1024) DEFAULT NULL,
  `OWNER` varchar(64) DEFAULT NULL,
  `NOTBEFORE` timestamp NULL DEFAULT NULL,
  `POINTMODE` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`OBSERVATION_ID`,`OTMslitwidth`)
) ENGINE=InnoDB AUTO_INCREMENT=1996 DEFAULT CHARSET=utf8mb3;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-07-18 10:50:04
