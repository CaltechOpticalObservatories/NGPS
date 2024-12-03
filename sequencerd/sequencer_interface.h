/**
 * @file     sequencer_interface.h
 * @brief    defines classes used to interface with various subsystems needed for the sequencer
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <map>
#include <string>
#include <atomic>
#include "skyinfo.h"

#include "network.h"
#include "logentry.h"
#include "common.h"
#include "database.h"
#include <sys/stat.h>

#include <vector>
#include <mysqlx/xdevapi.h>
#include <mysqlx/devapi/collection_crud.h>

#include "acam_interface_shared.h"

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
  const std::string DB_SETS="DB_SETS";            ///< name of the completed target sets table configuration parameter

  const std::string CAMERA_PROLOGUE="CAMERA_PROLOGUE";  ///< parameter name which defines CAMERA_PROLOGUE commands
  const std::string CAMERA_EPILOGUE="CAMERA_EPILOGUE";  ///< parameter name which defines CAMERA_EPILOGUE commands

  const std::string POWER_SLIT="POWER_SLIT";         ///< parameter name which defines NPS_PLUG names required for slit hardware
  const std::string POWER_CAMERA="POWER_CAMERA";     ///< parameter name which defines NPS_PLUG names required for camera hardware
  const std::string POWER_CALIB="POWER_CALIB";       ///< parameter name which defines NPS_PLUG names required for calib hardware
  const std::string POWER_FLEXURE="POWER_FLEXURE";   ///< parameter name which defines NPS_PLUG names required for flexure hardware
  const std::string POWER_FILTER="POWER_FILTER";     ///< parameter name which defines NPS_PLUG names required for filter hardware
  const std::string POWER_FOCUS="POWER_FOCUS";       ///< parameter name which defines NPS_PLUG names required for focus hardware
  const std::string POWER_TELEM="POWER_TELEM";       ///< parameter name which defines NPS_PLUG names required for telem hardware
  const std::string POWER_THERMAL="POWER_THERMAL";   ///< parameter name which defines NPS_PLUG names required for thermal hardware
  const std::string POWER_ACAM="POWER_ACAM";         ///< parameter name which defines NPS_PLUG names required for ACAM (A&G) hardware
  const std::string POWER_SLICECAM="POWER_SLICECAM"; ///< parameter name which defines NPS_PLUG names required for SLICECAM hardware

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
      PowerSwitch() = default;

      std::vector<std::string> plugname;      ///< vector of plug names required for this hardware subsystem

      long configure( std::string arglist );  ///< function to load plugname from cfg file
  };
  /***** Sequencer::PowerSwitch ***********************************************/


  class DatabaseManager {
    private:
      mysqlx::Session session;
      mysqlx::Schema db;
      mysqlx::Table table;

    public:
      DatabaseManager( const std::string &host, int port,
                       const std::string &user, const std::string &password,
                       const std::string &schema, const std::string tablename )
        : session( mysqlx::SessionOption::HOST, host,
                   mysqlx::SessionOption::PORT, port,
                   mysqlx::SessionOption::USER, user,
                   mysqlx::SessionOption::PWD, password ),
          db( session.getSchema(schema) ),
          table( db.getTable(tablename) ) { }

      mysqlx::RowResult do_query( const std::string &condition, const std::string &order,
                                  const std::map<std::string, std::string> &bindings,
                                  const std::vector<std::string> &columns={"*"} ) {
        auto query = table.select(columns).where(condition).orderBy(order);
        for ( const auto &[key, value] : bindings ) {
          query.bind(key, value);
        }
        return query.execute();
      }
  };


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
      TargetInfo() : db_port(-1), db_configured(false),
                     /**
                      * @var targetlist_cols fields used for accessing the target table,
                      *                      these define what you get with get_next()
                      */
                     targetlist_cols { "OWNER",
                                       "OBSERVATION_ID",
                                       "OBS_ORDER",
                                       "TARGET_NUMBER",
                                       "SEQUENCE_NUMBER",
                                       "STATE",
                                       "NAME",
                                       "RA",
                                       "DECL",
                                       "OTMexpt",
                                       "OTMslitwidth",
                                       "OTMslitangle",
                                       "SLITOFFSET",
                                       "BINSPECT",
                                       "BINSPAT",
                                       "OTMcass",
                                       "POINTMODE",
                                       "NOTE",
                                       "OTMFLAG",
                                       "NOTBEFORE",
                                       "AIRMASS_MAX" },
                     targetset_cols  { "SET_ID",
                                       "SET_NAME" },
                     offset_threshold(0), max_tcs_offset(0) { init_record(); }

      std::unique_ptr<DatabaseManager> dbManager;

      SkyInfo::FPOffsets fpoffsets;       ///< for calling Python fpoffsets, defined in ~/Software/common/skyinfo.h

      /***** Sequencer::TargetInfo::extract_column_from_row *******************/
      /**
       * @brief      gets named column value of the correct type from a mysqlx Row
       * @param[in]  field         database column name
       * @param[in]  row           mysqlx Row read from database
       * @param[in]  defaultvalue  optional arg to set value in case it's null
       *
       */
      template <typename T>
      T extract_column_from_row( const std::string &field, mysqlx::Row row, T defaultvalue=T{} ) {
        std::stringstream message;
        mysqlx::col_count_t col = std::numeric_limits<mysqlx::col_count_t>::max();
        std::string type;
        try {
          // find the column number for this field in the targetlist_cols vector
          //
          col = colnum( field, this->targetlist_cols );

          if ( col == std::numeric_limits<mysqlx::col_count_t>::max() ) throw std::runtime_error("field not found");

          // field is not empty
          //
          if ( !row[col].isNull() ) {
            return row.get(col).get<T>();
          }

          // If the field is empty and a default value was specified,
          // then return that default value.
          //
          if ( row[col].isNull() ) {
            if ( defaultvalue != T{} ) return defaultvalue;
            else return T{};
          }
          else
          if constexpr ( std::is_same<T, std::string>::value ) {
            if ( !defaultvalue.empty() ) return defaultvalue;
          }
          else
          if constexpr ( std::is_arithmetic<T>::value ) {
            if ( defaultvalue != T{} ) return defaultvalue;
          }
          else
          if constexpr ( std::is_default_constructible<T>::value ) {
            if ( defaultvalue != T{} ) return defaultvalue;
            else return T{};
          }
          message.str("");
          message << "field \"" << field << "\" empty and no default value specified";
          throw std::runtime_error(message.str());
        }
        catch( const mysqlx::Error &e ) {
          Database::get_mysql_type( row.get(col), type );
          message.str(""); message << "ERROR MySQL extracting \"" << field << "\"<" << type << "> col " << col;
          logwrite( "Sequencer::TargetInfo::extract_column_from_row", message.str() );
          message.str(""); message << "MySQL extracting \"" << field << "\"<" << type << ">: " << e.what();
          throw std::runtime_error(message.str());
        }
        catch( const std::runtime_error &e ) {
          Database::get_mysql_type( row.get(col), type );
          message.str(""); message << "ERROR MySQL extracting \"" << field << "\"<" << type << "> col " << col;
          logwrite( "Sequencer::TargetInfo::extract_column_from_row", message.str() );
          message.str(""); message << "extracting \"" << field << "\"<" << type << ">: " << e.what();
          throw std::runtime_error(message.str());
        }
      }
      /***** Sequencer::TargetInfo::extract_column_from_row *******************/

      /**
       * TargetState enums are the possible return values from the TargetInfo::TargetState get_next() function
       */
      enum TargetState {
        TARGET_ERROR=0,                   ///< error getting target from database
        TARGET_NOT_FOUND,                 ///< no active targets found with requested state
        TARGET_FOUND                      ///< target found
      };

      /**
       * @var  structure for external_telemetry map
       */
      struct ColumnData {
        bool valid;           // set when value is valid
        mysqlx::Value value;  // value
      };

      std::map<std::string, ColumnData> external_telemetry;


      /***** Sequencer::TargetInfo::column_from_json **************************/
      /**
       * @brief      assigns value from json telemetry message to a database column
       * @details    This will extract the value for the named jkey from the JSON
       *             message and assign it to external_telemetry map using colname
       *             as the map key, where colname is expected to be a database
       *             column name. When a non-null value is successfully found then
       *             set the .valid flag true, which will enable writing the value to
       *             the database. When valid is false then the column will not be
       *             written to the database, which is the sign of a bad value.
       * @param[in]  colname   database field name to assign value
       * @param[in]  jkey      key to find value from jmessage
       * @param[in]  jmessage  json message from telemetry provider
       *
       */
      template <typename T>
      void column_from_json( const std::string &colname,
                             const std::string &jkey,
                             const nlohmann::json &jmessage ) {
        const std::string function="Sequencer::TargetInfo::column_from_json";
        std::stringstream message;

        try {
          // If jkey is in jmessage and the value isn't null then
          // put that value in the external_telemetry map and set the
          // valid flag true,
          //
          if ( jmessage.contains(jkey) && !jmessage[jkey].is_null() ) {
            this->external_telemetry[colname].valid = true;
            this->external_telemetry[colname].value = jmessage[jkey].get<T>();
          }
          else {
            // otherwise clear the valid flag, which will prevent this column
            // from being added to the database.
            //
            this->external_telemetry[colname].valid = false; 
          }
        }
        catch( const nlohmann::json::type_error &e ) {
          message << "ERROR cannot associate value with key \"" << jkey << "\"";
          logwrite( function, message.str() );
          throw;
        }
        catch( const nlohmann::json::out_of_range &e ) {
          message << "ERROR key \"" << jkey << "\" not a valid index";
          logwrite( function, message.str() );
          throw;
        }
        catch( const std::exception &e ) {
          message << "ERROR getting jkey \"" << jkey << "\" for column " << colname;
          logwrite( function, message.str() );
          throw;
        }
        return;
      }
      /***** Sequencer::TargetInfo::column_from_json **************************/

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

      mysqlx::string owner;               ///< target table owner
      int            obsid;               ///< current target observation ID (DB internal, used for record-checking)
      int            setid;               ///< ID of the set to get from the targets table
      mysqlx::string setname;             ///< set name associated with setid
      int            obsorder;            ///< observation order (DB internal)
      long           targetnum;           ///< ??
      long           sequencenum;         ///< ??
      mysqlx::string name;                ///< name of astronomical target or calibration
      mysqlx::string fitsfile;            ///< file with the spectrum images
      mysqlx::string ra_hms;              ///< current target right ascension in units hh:mm:ss
      mysqlx::string dec_dms;             ///< current target declination in units dd:mm:ss
      mysqlx::string tel_ra;              ///< current target right ascension in units hh:mm:ss
      mysqlx::string tel_dec;             ///< current target declination in units dd:mm:ss
      double         tel_alt;             ///< telescope alt (deg) at exposure start
      double         tel_az;              ///< telescope az (deg) at exposure start
      double         airmass;             ///< airmass at exposure start
      double         casangle;            ///< current target cass angle
      double         slitangle_req;       ///< slit angle requested
      mysqlx::string pointmode;           ///< pointing mode contains slit|acam with optional space-delimited filter
      mysqlx::string notbefore;           ///< earliest date/time to start exposure
      mysqlx::string slewstart;           ///< slew start date/time
      mysqlx::string slewend;             ///< slew end date/time
      double         exptime_act;         ///< exposure time in seconds for this target comes from camerad telemetry
      double         exptime_req;         ///< exposure time request
      mysqlx::string expstart;            ///< exposure start date/time
      mysqlx::string expend;              ///< exposure end date/time
      double         slitwidth;           ///< slit width for this target
      double         slitwidth_req;       ///< slit width request
      double         slitoffset;          ///< slit offset for this target
      double         slitoffset_req;      ///< slit offset for this target
      int            binspect;            ///< binning in spectral direction for this target
      int            binspat;             ///< binning in spatial direction for this target
      double         airmasslimit;        ///< individual target airmass limit, above which we don't observe
      mysqlx::string obsmode;             ///< observation mode contains CCD settings TBD
      mysqlx::string note;                ///< observer notes, read just so that it can be written back to the DB completed table (!)
      mysqlx::string otmflag;             ///< OTM flag, read just so that it can be written back to the DB completed table (!)

      mysqlx::string state;               ///< current target state
      double         slitangle;           ///< current slit angle
      long           nexp;                ///< number of repeat exposures on this target

      double         offset_threshold;    ///< computed offset below this threshold (in arcsec) defines successful acquisition
      double         max_tcs_offset;      ///< max allowable TCS offset
      int            min_repeat;          ///< minimum number of sequentiall successful acquires
      std::atomic<bool> acquired;         ///< true on successful acquisition and while guiding

      mysqlx::col_count_t  colnum( std::string field, std::vector<std::string> vec );   ///< get column number of requested field from specified vector list
      TargetInfo::TargetState get_specified_target( std::string args, std::string &retstring );
      TargetInfo::TargetState get_next( );                     ///< get the next target from the database with state=Sequencer::TARGET_PENDING
      TargetInfo::TargetState get_next( std::string &status ); ///< get the next target from the database with state=Sequencer::TARGET_PENDING
      TargetInfo::TargetState get_next( std::string state_in, std::string &status );    ///< get the next target from the database with state=state_in
      long parse_target_from_row( const mysqlx::Row &row );  ///< fills target class using cols from DB row
      long target_qc_check( std::string &status );                   ///< target info quality control check
      long add_row( int number_in, std::string name_in, std::string ra_hms_in, std::string dec_dms_in,
                    double slita_in, double slitw_in, double exptime_in, std::string pointmode_in );   ///< add a row to the database
      long update_state( std::string newstate );  ///< update the target status in the database DB_ACTIVE table
      long insert_completed();            ///< insert target record into completed observations table
      long get_table_names();             ///< utility to print all database table names
      void init_record();                 ///< initialize current target record variables

      long configure_db( std::string param, std::string value );  ///< configure the database connection parameters (host, user, etc.)
      long targetset( std::string args, std::string &retstring ); ///< set or get the target set to read from the targets table

  };
  /***** Sequencer::TargetInfo ************************************************/

}
/***** Sequencer **************************************************************/
