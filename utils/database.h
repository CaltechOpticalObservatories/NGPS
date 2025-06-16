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
#include <condition_variable>
#include <mutex>
#include <queue>
#include <map>
#include <string>
#include "logentry.h"


/***** Database ***************************************************************/
/**
 * @namespace Database 
 *
 */
namespace Database {

  constexpr int DBPOOLSIZE = 10;

  void get_mysql_type( mysqlx::Value value, std::string &type );

  /***** Database::DatabasePool ***********************************************/
  /**
   * @class    DatabasePool
   * @brief    manages a pool of database sessions
   * @details  This uses a queue object to create a pool of database sessions,
   *           to provides for safe multi-threaded access to a MySQL database by
   *           returning a unique connection for each request. A mutex and
   *           condition variable are used for thread synchronization. A pool
   *           of established connections improves performance.
   *
   */
  class DatabasePool {
    private:
      // For these mysqlx objects an instance cannot be directly created
      // so instead create pointers and wrap them into one struct.
      struct DatabaseHandle {
        std::unique_ptr<mysqlx::Session> session;
        std::unique_ptr<mysqlx::Table>   table;
        std::unique_ptr<mysqlx::Schema>  schema;
      };

      std::queue<std::shared_ptr<DatabaseHandle>> _db_queue;                    ///< pool of database connections

      std::shared_ptr<DatabaseHandle> _create_handle();                         /// create a single mysqlx database connection
      std::shared_ptr<DatabaseHandle> _borrow_handle(int timeout_ms=5000);      ///< get connection from pool
      void _return_handle(std::shared_ptr<DatabaseHandle> db);                  ///< return connection to pool
      bool _test_connection(std::shared_ptr<DatabasePool::DatabaseHandle> db);  ///< test connection

      // This is the database info needed to construct a
      // Database object and connect to a table in the database.
      std::string _dbhost;
      int         _dbport;
      std::string _dbuser;
      std::string _dbpass;
      std::string _dbschema;
      std::string _dbtable;

      int _poolsz;

      std::mutex _mtx;
      std::condition_variable _cv;

      /**
       * @brief  helper class to ensure handles are always returned
       */
      class HandleGuard {
        private:
          DatabasePool* _pool;                      // pointer to pool
          std::shared_ptr<DatabaseHandle> _handle;  // borrowed database handle
        public:
          /// borrows a handle from the pool on construction
          HandleGuard(DatabasePool* pool) : _pool(pool), _handle(pool->_borrow_handle()) { }

          /// when destructed the handle is returned
          ~HandleGuard() { if (_handle) _pool->_return_handle(_handle); }

          /// returns the handle that was borrowed on construction
          std::shared_ptr<DatabasePool::DatabaseHandle> get() { return _handle; }
      };

    public:
      DatabasePool(const std::string &host, int port, const std::string &user,
                   const std::string &pass, const std::string &schema,
                   const std::string &table, int poolsz=DBPOOLSIZE);
      ~DatabasePool();

      void write(std::map<std::string, mysqlx::Value> data);
  };
  /***** Database::DatabasePool ***********************************************/


  /***** Database::Database ***************************************************/
  /**
   * @class    Database
   * @brief    provides an interface to a MySQL database
   * @details  Makes use of DatabasePool for safe multi-threaded access.
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

      // These map stores in memory the record to be written to
      // the database. The data is essentially a variant type so
      // it can hold values of any type, indexed by column name.
      //
      std::map<std::string, mysqlx::Value> _data;

      std::mutex _data_mtx;       ///< protecs _data map access

      std::unique_ptr<DatabasePool> _pool;  ///< this is the pool of database connections
      void _initialize_pool();

    public:
      Database() : _dbconfigured(false) { }
      Database( std::vector<std::string> info );
      Database( std::string host,
                int         port,
                std::string user,
                std::string pass,
                std::string schema,
                std::string table );

      ~Database();

      void initialize_class();
      void initialize_class(std::vector<std::string> dbinfo);

      /***** Database::Database::add_key_val **********************************/
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
      void add_key_val( const std::string &key, T value ) {
        std::lock_guard<std::mutex> lock(_data_mtx);
        _data[key] = value;
      }
      /***** Database::Database::add_key_val **********************************/

      /***** Database::Database::add_from_json ********************************/
      /**
       * @brief      verifies key in json message and adds key/val to Database::_data
       * @details    Accepts a json message and key, verifies the key is in the message
       *             and the value is not null, and if that passes then use the
       *             add_key_val() function to add the associated value to the
       *             Database class internal _data map.
       * @param[in]  jmessage  json message
       * @param[in]  jkey      key to extract value from json message
       * @param[in]  optional  optional database key, defaults to json key
       */
      template <typename T>
      void add_from_json( const nlohmann::json &jmessage, const std::string &jkey, const std::string &optional="" ) {
        // The dbkey used to write to the database is the same as the jkey, unless
        // otherwise specified.
        //
        const std::string &dbkey = ( optional.empty() ? jkey : optional );

        // index jmessage with jkey
        if ( jmessage.contains(jkey) ) {
          if ( !jmessage[jkey].is_null() ) {
            try {
              // index database with dbkey
              this->add_key_val( dbkey, jmessage[jkey].get<T>() );
            }
            catch ( const nlohmann::json::type_error &e ) {
              logwrite( "Database::Database::add_from_json", "ERROR key \""+jkey+"\" not expected type: "+e.what() );
            }
          }
        }
      }
      /***** Database::Database::add_from_json ********************************/

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
