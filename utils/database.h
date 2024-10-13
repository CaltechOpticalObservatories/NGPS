/**
 * @file    database.h
 * @brief   defines the Database class for communicating with a MySQL database
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <mysqlx/xdevapi.h>
#include <vector>
#include <memory>
#include <atomic>
#include "logentry.h"


/***** Database ***************************************************************/
/**
 * @namespace Database 
 *
 */
namespace Database {

  /***** Database::Database ***************************************************/
  /**
   * @class  Database
   * @brief  constructs a MySQL database object
   *
   */
  class Database {
    private:

      // This is the database info needed to construct a
      // Database object and connect to a table in the database.
      //
      std::string _dbhost;
      int         _dbport;
      std::string _dbuser;
      std::string _dbpass;
      std::string _dbschema;
      std::string _dbtable;

      std::atomic<bool> _dbconfigured;
      std::atomic<bool> _dbconnected;
      std::atomic<bool> _sessionopen;

      // For these mysqlx objects an instance cannot be directly created
      // so instead create pointers.
      //
      std::unique_ptr<mysqlx::Session> _session;
      std::unique_ptr<mysqlx::Table>   _table;
      std::unique_ptr<mysqlx::Schema>  _schema;

      void _create_connection();

      // These map stores in memory the record to be written to
      // the database. The data is essentially a variant type so
      // it can hold values of any type, indexed by column name.
      //
      std::map<std::string, mysqlx::Value> _data;

    public:
      Database() : _dbconfigured(false), _dbconnected(false), _sessionopen(false),
                   _session(nullptr), _table(nullptr), _schema(nullptr) { }
      Database( std::vector<std::string> info );
      Database( std::string host,
                int         port,
                std::string user,
                std::string pass,
                std::string schema,
                std::string table );

      ~Database();

      inline bool dbconfigured() { return _dbconfigured; }
      inline bool dbconnected()  { return _dbconnected; }
      inline bool sessionopen()  { return _sessionopen; }

      void initialize_class( std::vector<std::string> info );
      void close();

      /***** Database::Database::add_telem_entry ******************************/
      /**
       * @brief      adds a key,value pair to the Database class
       * @details    This adds a row into memory that will make up the complete
       *             record to be written. The private _data map contains data
       *             as mysqlx::Value indexed by name. msqlx::Value is essentially
       *             a variant so it can hold a mix of any different type.
       * @param[in]  key    name for the database column, or field
       * @param[in]  value  value of entry can be any type
       */
      template <typename T>
      void add_telem_entry( const std::string &key, T value ) {
        _data[key] = value;
      }
      /***** Database::Database::add_telem_entry ******************************/

      void write( std::map<std::string, mysqlx::Value> data );  ///< write passed map
      void write();                                             ///< write class map

      /**
       * @brief      writes an STL map of any single type rather than mysqlx::Value
       * @param[in]  data  map of data of any single type, indexed by field name
       */
      template <typename T>
      void write( std::map<std::string, T> data ) {
        this->write(data);
      }
  };
  /***** Database::Database ***************************************************/

}
