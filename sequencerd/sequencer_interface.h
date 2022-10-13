/**
 * @file     sequencer_interface.h
 * @brief    defines classes used to interface with various subsystems needed for the sequencer
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SEQUENCER_INTERFACE_H
#define SEQUENCER_INTERFACE_H

#include <map>
#include <string>

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

#include <vector>
#include <mysqlx/xdevapi.h>

#define ERROR_TARGETLIST_BAD_HEADER 1001  ///< TODO change this

/***** Sequencer **************************************************************/
/**
 * @namespace Sequencer
 * @brief     namespace for the observation sequencer
 *
 */
namespace Sequencer {

  // These are not the values of the configuration parameters,
  // but the names of the parameters themselves, as used in the .cfg file
  // and other places
  //
  const std::string DB_HOST="DB_HOST";            ///< name of the database host configuration parameter used in .cfg file and elsewhere
  const std::string DB_PORT="DB_PORT";            ///< name of the database port configuration parameter used in .cfg file and elsewhere
  const std::string DB_USER="DB_USER";            ///< name of the database username configuration parameter used in .cfg file and elsewhere
  const std::string DB_PASS="DB_PASS";            ///< name of the database password configuration parameter used in .cfg file and elsewhere
  const std::string DB_SCHEMA="DB_SCHEMA";        ///< name of the database schema configuration parameter used in .cfg file and elsewhere
  const std::string DB_ACTIVE="DB_ACTIVE";        ///< name of the active target table configuration parameter
  const std::string DB_COMPLETED="DB_COMPLETED";  ///< name of the completed observations table configuration parameter

  const std::string POWER_SLIT="POWER_SLIT";         ///< parameter name which defines NPS_PLUG names required for slit hardware
  const std::string POWER_CAMERA="POWER_CAMERA";     ///< parameter name which defines NPS_PLUG names required for camera hardware
  const std::string POWER_CALIB="POWER_CALIB";       ///< parameter name which defines NPS_PLUG names required for calib hardware
  const std::string POWER_FLEXURE="POWER_FLEXURE";   ///< parameter name which defines NPS_PLUG names required for flexure hardware
  const std::string POWER_FILTER="POWER_FILTER";     ///< parameter name which defines NPS_PLUG names required for filter hardware
  const std::string POWER_FOCUS="POWER_FOCUS";       ///< parameter name which defines NPS_PLUG names required for focus hardware
  const std::string POWER_TELEM="POWER_TELEM";       ///< parameter name which defines NPS_PLUG names required for telem hardware
  const std::string POWER_THERMAL="POWER_THERMAL";   ///< parameter name which defines NPS_PLUG names required for thermal hardware
  const std::string POWER_AG="POWER_AG";             ///< parameter name which defines NPS_PLUG names required for A&G hardware

  // These are the possible target states
  //
  const std::string TARGET_PENDING="pending";        ///< target status pending
  const std::string TARGET_EXPOSING="exposing";      ///< target status exposing
  const std::string TARGET_COMPLETE="complete";      ///< target status complete
  const std::string TARGET_UNASSIGNED="unassigned";  ///< target status unassigned


  /***** Sequencer::Daemon ****************************************************/
  /**
   * @class  Daemon
   * @brief  defines a daemon-object for each daemon that the Sequencer connects to
   *
   */
  class Daemon {
    private:
      bool isopen;                  ///< is a connection to the daemon open?

    public:
      Daemon();
      ~Daemon();

      std::string name;             ///< name of the daemon
      std::string host;             ///< host where the daemon is running
      int port;                     ///< port that the daemon is listening on

      Network::TcpSocket socket;    ///< socket object for communications with daemon

      long command( std::string args );                            ///< commands to daemon
      long command( std::string args, std::string &retstring );    ///< commands to daemon that need a reply
      long send( std::string command, std::string &reply );        ///< for internal use only
      long connect();                                              ///< initialize socket connection to daemon
      void set_name( std::string name_in ) { this->name=name_in; } ///< name this daemon
      void set_port( int port );                                   ///< set the port number

      bool is_open() { return this->isopen; }
      long is_connected( std::string &reply );
  };
  /***** Sequencer::Daemon ****************************************************/


  /***** Sequencer::PowerSwitch ***********************************************/
  /**
   * @class  PowerSwitch
   * @brief  power switch information class, associates a vector of plug name(s) with a given subsystem
   *
   * This class contains the list (vector) of plug names for a given subsystem
   * and a function for loading the vector from the configuration file.
   *
   * This is only information -- power control is performed using the power daemon, but this allows
   * the Sequencer to know which plugs are associated with a given subsystem. This vector is read
   * from the configuration file.
   *
   * For proper usage, an STL map is created in the Sequence class,
   *   std::map<std::string, class PowerSwitch> power_switch
   * so there is a map between each hardware subsystem and this class.
   *
   * The subsystem names mirror the configuration parameter names defined
   * above, i.e.
   *   const std::string POWER_SLIT
   *   const std::string POWER_CAMERA
   *   etc.
   *
   */
  class PowerSwitch {
    private:
    public:
      PowerSwitch();
      ~PowerSwitch();

      std::vector<std::string> plugname;      ///< vector of plug names required for this hardware subsystem

      long configure( std::string arglist );  ///< function to load plugname from cfg file
  };
  /***** Sequencer::PowerSwitch ***********************************************/


  /***** Sequencer::TargetInfo ************************************************/
  /**
   * @class  TargetInfo
   * @brief  target information class contains info for a single target row and how to read it
   *
   */
  class TargetInfo {
    private:
      std::string db_host;                ///< database host
      int         db_port;                ///< database port
      std::string db_user;                ///< database username
      std::string db_pass;                ///< database password
      std::string db_schema;              ///< database schema
      std::string db_active;              ///< database active observations table
      std::string db_completed;           ///< database completed observations table
      bool        db_configured;          ///< have the database params been set?

    public:
      TargetInfo();
      ~TargetInfo();

      /**
       * TargetState enums are the possible return values from the TargetInfo::TargetState get_next() function
       */
      enum TargetState {
        TARGET_ERROR=0,                   ///< error getting target from database
        TARGET_NOT_FOUND,                 ///< no active targets found with requested state
        TARGET_FOUND                      ///< target found
      };

      bool is_db_configured() { return this->db_configured; };   ///< get the private db_configured state

      const float RA_MIN = 0;             ///< minimum target right ascension
      const float RA_MAX = 24;            ///< maximum target right ascension
      const float DEC_MIN = -90;          ///< minimum target declination
      const float DEC_MAX = 90;           ///< maximum target declination
      const float SLIT_MIN = 0;           ///< minimum slit width
      const float SLIT_MAX = 10;          ///< maximum slit width
      const long  EXPTIME_MIN = 0;        ///< minimum value for exptime
      const long  EXPTIME_MAX = 1 << 24;  ///< maximum value for exptime

      std::vector<std::string> targetlist;///< target list fields, used for accessing the target table, which accepts a variadic param

      int            obsid;               ///< current target observation ID (DB internal, used for record-checking)
      int            obsorder;            ///< observation order (DB internal)
      mysqlx::string name;                ///< current target name
      mysqlx::string ra;                  ///< current target right ascension
      mysqlx::string dec;                 ///< current target declination
      mysqlx::string epoch;               ///< current target coordinates epoch
      double         casangle;            ///< current target cass angle
      double         slitwidth;           ///< slit width for this target
      double         slitoffset;          ///< slit offset for this target
      double         exptime;             ///< exposure time in seconds for this target
      long           nexp;                ///< number of repeat exposures on this target
      long           targetnum;           ///< ??
      long           sequencenum;         ///< ??
      mysqlx::string obsplan;             ///< TBD
      int            binspect;            ///< binning in spectral direction for this target
      int            binspat;             ///< binning in spatial direction for this target

      int  colnum( std::string field );   ///< get column number of requested field from this->targetlist
      TargetInfo::TargetState get_next(); ///< get the next target from the database with state=Sequencer::TARGET_PENDING
      TargetInfo::TargetState get_next( std::string state_in);    ///< get the next target from the database with state=state_in
      long add_row( int number, std::string name, std::string ra, std::string dec );   ///< add a row to the database
      long update_state( std::string newstate );  ///< update the target status in the database DB_ACTIVE table
      long insert_completed();            ///< insert target record into completed observations table
      long get_table_names();             ///< utility to print all database table names
      void init_record();

      long configure_db( std::string param, std::string value );  ///< configure the database connection parameters (host, user, etc.)

  };
  /***** Sequencer::TargetInfo ************************************************/

}
/***** Sequencer **************************************************************/
#endif
