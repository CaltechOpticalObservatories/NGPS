CREATE TABLE `completed_obs` (
  `LOG_ID`                int NOT NULL                AUTO_INCREMENT,
  `OWNER`                 varchar(32) NOT NULL        COMMENT "Name of target list owner",
  `OBSERVATION_ID`        int                         COMMENT "ID from the master targets table",
  `SET_ID`                int                         COMMENT "ID of the user's target set",
  `TARGET_NUMBER`         int                         COMMENT "",
  `SEQUENCE_NUMBER`       int                         COMMENT "",

  `NAME`                  varchar(128) NOT NULL       COMMENT "Name of the astronomical target or calibration",
  `FITSFILE`              varchar(512)                COMMENT "File with the spectrum images",
            
  `RA`                    varchar(32)                 COMMENT "Right ascension",
  `DECL`                  varchar(32)                 COMMENT "Declination",
  `ALT`                   float                       COMMENT "Altitude (deg)",
  `AZ`                    float                       COMMENT "Azimuth (deg)",
  `AIRMASS`               float                       COMMENT "Airmass",                         -- START OR END??
  `CASANGLE`              float                       COMMENT "Cassegrain angle of the P200",
  `SLITANGLE_REQ`         varchar(8)                  COMMENT "Slit angle request",
  `POINTMODE`             varchar(8)                  COMMENT "Where to place target (ACAM or SLIT)",
            
  `NOTBEFORE`             datetime                    COMMENT "Earliest date/time to start exposure",
  `SLEW_START`            datetime                    COMMENT "Slew start date/time",
  `EXPTIME`               float                       COMMENT "Exposure time (s)",
  `EXPTIME_REQ`           varchar(12)                 COMMENT "Exposure time request",
  `EXP_START`             datetime                    COMMENT "Exposure start date/time",
  `EXP_END`               datetime                    COMMENT "Exposure end date/time",
  `SLITWIDTH`             float                       COMMENT "Spectrograph slit width (arcsec)",
  `SLITWIDTH_REQ`         varchar(12)                 COMMENT "Slit width request",
  `SLITOFFSET`            float                       COMMENT "Slit lateral offset (arcsec)",
            
  `BINSPECT`              int                         COMMENT "CCD binning in spectral direction",
  `BINSPAT`               int                         COMMENT "CCD binning in spatial direction",
  `OBSMODE`               varchar(32)                 COMMENT "Observation mode, CCD settings",
            
  `NOTE`                  varchar(512)                COMMENT "Observer's note on this target",
  `OTMFLAG`               varchar(20)                 COMMENT "OTM flag codes at time of exposure",

  PRIMARY KEY (`LOG_ID`,`NAME`),
  UNIQUE KEY `LOG_ID_UNIQUE` (`LOG_ID`)
) ENGINE=InnoDB AUTO_INCREMENT=81 DEFAULT CHARSET=utf8mb3;
