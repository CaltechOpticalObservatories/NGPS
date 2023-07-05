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
#include <atomic>
#include "cpython.h"

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

#include <vector>
#include <mysqlx/xdevapi.h>

#define ERROR_TARGETLIST_BAD_HEADER 1001  ///< TODO change this
#define PYTHON_PATH "/home/developer/Software/Python/acam_skyinfo"
#define PYTHON_FPOFFSETS_MODULE "FPoffsets"
#define PYTHON_FPOFFSETS_FUNCTION "compute_offset"

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
  const std::string DB_SETS="DB_SETS";            ///< name of the completed target sets table configuration parameter

  const std::string POWER_SLIT="POWER_SLIT";         ///< parameter name which defines NPS_PLUG names required for slit hardware
  const std::string POWER_CAMERA="POWER_CAMERA";     ///< parameter name which defines NPS_PLUG names required for camera hardware
  const std::string POWER_CALIB="POWER_CALIB";       ///< parameter name which defines NPS_PLUG names required for calib hardware
  const std::string POWER_FLEXURE="POWER_FLEXURE";   ///< parameter name which defines NPS_PLUG names required for flexure hardware
  const std::string POWER_FILTER="POWER_FILTER";     ///< parameter name which defines NPS_PLUG names required for filter hardware
  const std::string POWER_FOCUS="POWER_FOCUS";       ///< parameter name which defines NPS_PLUG names required for focus hardware
  const std::string POWER_TELEM="POWER_TELEM";       ///< parameter name which defines NPS_PLUG names required for telem hardware
  const std::string POWER_THERMAL="POWER_THERMAL";   ///< parameter name which defines NPS_PLUG names required for thermal hardware
  const std::string POWER_ACAM="POWER_ACAM";         ///< parameter name which defines NPS_PLUG names required for ACAM (A&G) hardware

  const std::string ACQUIRE_OFFSET_THRESHOLD="ACQUIRE_OFFSET_THRESHOLD";    ///< below this in arcsec defines successful acquisition
  const std::string ACQUIRE_MIN_REPEAT="ACQUIRE_MIN_REPEAT";    ///< minimum number of successful sequential acquires
  const std::string ACQUIRE_TCS_MAX_OFFSET="ACQUIRE_TCS_MAX_OFFSET";    ///< max allowable TCS offset

  // These are the possible target states
  //
  const std::string TARGET_PENDING="pending";        ///< target status pending
  const std::string TARGET_ACTIVE="active";          ///< target status active (slew and acquire)
  const std::string TARGET_EXPOSING="exposing";      ///< target status exposing
  const std::string TARGET_COMPLETE="completed";     ///< target status complete
  const std::string TARGET_UNASSIGNED="inactive";    ///< target status unassigned

  /***** Sequencer::Daemon ****************************************************/
  /**
   * @class  Daemon
   * @brief  defines a daemon-object for each daemon that the Sequencer connects to
   *
   */
  class Daemon {
    private:
      bool isopen;                  ///< is a connection to the daemon open?
      bool timedout;

    public:
      Daemon();
      Daemon( std::string name );   ///< preferred constructor with name to identify daemon
      ~Daemon();

      std::string name;             ///< name of the daemon
      std::string host;             ///< host where the daemon is running
      int port;                     ///< blocking port that the daemon is listening on
      int nbport;                   ///< non-blocking port that the daemon is listening on

      Network::TcpSocket socket;    ///< socket object for communications with daemon

      long async( std::string args );                              ///< async (non-blocking) commands to daemon that don't need a reply
      long async( std::string args, std::string &retstring );      ///< async (non-blocking) commands to daemon that need a reply

      static void foo( );
      static void dothread_command( Sequencer::Daemon &daemon, std::string args );
      long command( std::string args );                            ///< commands to daemon
      long command( std::string args, std::string &retstring );    ///< commands to daemon that need a reply
      long send( std::string command, std::string &reply );        ///< for internal use only
      long connect();                                              ///< initialize socket connection to daemon
      long disconnect();                                           ///< close socket connection to daemon
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


  /***** Sequencer::FPOffsets *************************************************/
  /**
   * @class   FPOffsets
   * @brief   class for calling FPOffsets python functions
   * @details makes use of CPython::CPytInstance defined in cpython.h
   *
   */
  class FPOffsets {
    private:
      char* restore_path;       /// if the PYTHONPATH env variable is changed then remember the original
      bool python_initialized;  /// set true when the Python interpreter has been initialized

    public:
      FPOffsets();
      ~FPOffsets();

      inline bool is_initialized() { return this->python_initialized; };

      // store the in/out coordinates in the class
      //
      typedef struct {
        double ra;
        double dec;
        double angle;
      } coords_t;

      coords_t coords_in { 0, 0, 0 };
      coords_t coords_out { 0, 0, 0 };

      CPython::CPyInstance py_instance { PYTHON_PATH };  /// initialize the Python interpreter
      PyObject* pModuleName;
      PyObject* pModule;

      inline void set_inputs( double dec, double ra, double angle ) {
        this->coords_in.ra=ra;
        this->coords_in.dec=dec;
        this->coords_in.angle=angle;
        return;
      }

      inline void get_outputs( double &dec, double &ra, double &angle ) {
	dec=this->coords_out.dec;
	ra=this->coords_out.ra;
	angle=this->coords_out.angle;
        return;
      }

      // compute_offset() is overloaded
      //
      // It can be called with no coordinates (using the class variables for input/output).
      // It can be called with inputs only (usng the class variables for output).
      // It can be called with both inputs and a reference to output variables so that outputs are returned directly.
      // In all cases, the class variables will be updated with the results.
      //
      long compute_offset( std::string from, std::string to );
      long compute_offset( std::string from, std::string to, double ra_in, double dec_in, double angle_in );
      long compute_offset( std::string from, std::string to, 
                           double ra_in, double dec_in, double angle_in,
                           double &ra_out, double &dec_out, double &angle_out );
  };
  /***** Sequencer::FPOffsets *************************************************/


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
      std::string db_sets;                ///< database table of target sets
      bool        db_configured;          ///< have the database params been set?

    public:
      TargetInfo();
      ~TargetInfo();

      FPOffsets fpoffsets;

      /**
       * TargetState enums are the possible return values from the TargetInfo::TargetState get_next() function
       */
      enum TargetState {
        TARGET_ERROR=0,                   ///< error getting target from database
        TARGET_NOT_FOUND,                 ///< no active targets found with requested state
        TARGET_FOUND                      ///< target found
      };

      bool is_db_configured() { return this->db_configured; };   ///< get the private db_configured state

      const double RA_MIN = 0;            ///< minimum target right ascension
      const double RA_MAX = 24;           ///< maximum target right ascension
      const double DEC_MIN = -90;         ///< minimum target declination
      const double DEC_MAX = 90;          ///< maximum target declination
      const double SLIT_MIN = 0;          ///< minimum slit width
      const double SLIT_MAX = 10;         ///< maximum slit width
      const long  EXPTIME_MIN = 0;        ///< minimum value for exptime
      const long  EXPTIME_MAX = 1 << 24;  ///< maximum value for exptime

      std::vector<std::string> targetlist_cols;  ///< target list fields, used for accessing the target table, which accepts a variadic param
      std::vector<std::string> targetset_cols;   ///< target set fields, used for accessing the table of target sets

      int            setid;               ///< ID of the set to get from the targets table
      mysqlx::string setname;             ///< set name associated with setid
      int            obsid;               ///< current target observation ID (DB internal, used for record-checking)
      int            obsorder;            ///< observation order (DB internal)
      mysqlx::string state;               ///< current target state
      mysqlx::string name;                ///< current target name
      mysqlx::string ra_hms;              ///< current target right ascension in units hh:mm:ss
      mysqlx::string dec_dms;             ///< current target declination in units dd:mm:ss
      double         casangle;            ///< current target cass angle
      double         slitangle;           ///< current slit angle
      double         slitwidth;           ///< slit width for this target
      double         slitoffset;          ///< slit offset for this target
      double         exptime;             ///< exposure time in seconds for this target
      long           nexp;                ///< number of repeat exposures on this target
      long           targetnum;           ///< ??
      long           sequencenum;         ///< ??
      mysqlx::string obsplan;             ///< TBD
      int            binspect;            ///< binning in spectral direction for this target
      int            binspat;             ///< binning in spatial direction for this target

      double         offset_threshold;    ///< computed offset below this threshold (in arcsec) defines successful acquisition
      double         max_tcs_offset;      ///< max allowable TCS offset
      int            min_repeat;          ///< minimum number of sequentiall successful acquires
      std::atomic<bool> acquired;         ///< true on successful acquisition and while guiding

      int  colnum( std::string field, std::vector<std::string> vec );   ///< get column number of requested field from specified vector list
      TargetInfo::TargetState get_next( );                     ///< get the next target from the database with state=Sequencer::TARGET_PENDING
      TargetInfo::TargetState get_next( std::string &status ); ///< get the next target from the database with state=Sequencer::TARGET_PENDING
      TargetInfo::TargetState get_next( std::string state_in, std::string &status );    ///< get the next target from the database with state=state_in
      long target_qc( std::string &status );                   ///< target info quality control
      long add_row( int number, std::string name, std::string ra_hms, std::string dec_dms );   ///< add a row to the database
      long update_state( std::string newstate );  ///< update the target status in the database DB_ACTIVE table
      long insert_completed();            ///< insert target record into completed observations table
      long get_table_names();             ///< utility to print all database table names
      void init_record();

      long configure_db( std::string param, std::string value );  ///< configure the database connection parameters (host, user, etc.)
      long targetset( std::string args, std::string &retstring ); ///< set or get the target set to read from the targets table

      double radec_to_decimal( std::string str_in );                           ///< convert ra,dec from string to double
      double radec_to_decimal( std::string str_in, std::string &retstring );   ///< convert ra,dec from string to double
      void decimal_to_sexa( double dec_in, std::string &retstring );           ///< convert decimal to sexagesimal

  };
  /***** Sequencer::TargetInfo ************************************************/

}
/***** Sequencer **************************************************************/
#endif
