#ifndef SEQUENCER_INTERFACE_H
#define SEQUENER_INTERFACE_H

#include <map>
#include <string>

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

#include <vector>
#include <mysqlx/xdevapi.h>

#define ERROR_TARGETLIST_BAD_HEADER 1001  // TODO change this

namespace Sequencer {

  // These are not the values of the configuration parameters,
  // but the names of the parameters themselves, as used in the .cfg file
  // and other places
  //
  const std::string DB_HOST="DB_HOST";            /// name of the database host configuration parameter used in .cfg file and elsewhere
  const std::string DB_PORT="DB_PORT";            /// name of the database port configuration parameter used in .cfg file and elsewhere
  const std::string DB_USER="DB_USER";            /// name of the database username configuration parameter used in .cfg file and elsewhere
  const std::string DB_PASS="DB_PASS";            /// name of the database password configuration parameter used in .cfg file and elsewhere
  const std::string DB_SCHEMA="DB_SCHEMA";        /// name of the database schema configuration parameter used in .cfg file and elsewhere
  const std::string DB_ACTIVE="DB_ACTIVE";        /// name of the active target table configuration parameter
  const std::string DB_COMPLETED="DB_COMPLETED";  /// name of the completed observations table configuration parameter

  // These are the possible target states
  //
  const std::string TARGET_PENDING="pending";        /// target status pending
  const std::string TARGET_COMPLETE="complete";      /// target status complete
  const std::string TARGET_UNASSIGNED="unassigned";  /// target status unassigned


  /** Daemon ******************************************************************/
  /**
   * @class  Daemon
   * @brief  daemon class
   *
   */
  class Daemon {
    private:
      bool isopen;                  /// is a connection to the daemon open?

    public:
      Daemon();
      ~Daemon();

      std::string name;             /// name of the daemon
      std::string host;             /// host where the daemon is running
      int port;                     /// port that the daemon is listening on

      Network::TcpSocket socket;    /// socket object for communications with daemon

      long command( std::string args );                            /// commands to daemon
      long command( std::string args, std::string &retstring );    /// commands to daemon that need a reply
      long send( std::string command, std::string &reply );        /// for internal use only
      long connect();                                              /// initialize socket connection to daemon
      void set_name( std::string name_in ) { this->name=name_in; } /// name this daemon
      void set_port( int port );                                   /// set the port number

      bool is_open() { return this->isopen; }
      long is_connected( std::string &reply );
  };
  /** Daemon ******************************************************************/


  /** TargetInfo **************************************************************/
  /**
   * @class  TargetInfo
   * @brief  target information class
   *
   * This class contains the information for a target row
   * and how to read it, etc.
   *
   */
  class TargetInfo {
    private:
      std::string db_host;                /// database host
      int         db_port;                /// database port
      std::string db_user;                /// database username
      std::string db_pass;                /// database password
      std::string db_schema;              /// database schema
      std::string db_active;              /// database active observations table
      std::string db_completed;           /// database completed observations table
      bool        db_configured;          /// have the database params been set?

    public:
      TargetInfo();
      ~TargetInfo();

      enum TargetState {
        TARGET_ERROR=0,
        TARGET_NOT_FOUND,
        TARGET_FOUND
      };

      bool is_db_configured() { return this->db_configured; };   /// get the private db_configured state

      const float RA_MIN = 0;             /// minimum target right ascension
      const float RA_MAX = 24;            /// maximum target right ascension
      const float DEC_MIN = -90;          /// minimum target declination
      const float DEC_MAX = 90;           /// maximum target declination
      const float SLIT_MIN = 0;           /// minimum slit width
      const float SLIT_MAX = 10;          /// maximum slit width
      const long  EXPTIME_MIN = 0;        /// minimum value for exptime
      const long  EXPTIME_MAX = 1 << 24;  /// maximum value for exptime
      const long  NEXP_MIN = 0;           /// minimum value for nexp
      const long  NEXP_MAX = 1 << 24;     /// maximum value for nexp

      std::vector<std::string> targetlist;/// target list fields, used for accessing the target table, which accepts a variadic param

      int            obsid;               /// current target observation ID (DB internal, used for record-checking)
      int            obsorder;            /// observation order (DB internal)
      mysqlx::string name;                /// current target name
      mysqlx::string ra;                  /// current target right ascension
      mysqlx::string dec;                 /// current target declination
      mysqlx::string epoch;               /// current target coordinates epoch
      mysqlx::string casangle;            /// current target cass angle
      double         slitwidth;           /// slit width for this target
      double         slitoffset;          /// slit offset for this target
      double         exptime;             /// exposure time in seconds for this target
      long           nexp;                /// number of repeat exposures on this target
      mysqlx::string obsplan;             /// TBD
      int            binspect;            /// binning in spectral direction for this target
      int            binspat;             /// binning in spatial direction for this target

      int  colnum( std::string field );   /// get column number of requested field from this->targetlist
      TargetInfo::TargetState get_next();                    /// get the next target from the database
      long add_row();                     /// connect to the database
      long update_state( std::string newstate );        /// update the target status in the database DB_ACTIVE table
      void init_record();

      long configure_db( std::string param, std::string value );      /// configure the database connection parameters (host, user, etc.)

  };
  /** TargetInfo **************************************************************/

}
#endif
