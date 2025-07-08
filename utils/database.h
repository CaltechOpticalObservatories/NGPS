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
#include <optional>
#include "logentry.h"


/***** Database ***************************************************************/
/**
 * @namespace Database 
 *
 */
namespace Database {

  constexpr int DBPOOLSIZE = 10;

  void get_mysql_type( mysqlx::Value value, std::string &type );

  /***** Database::SessionPool ************************************************/
  /**
   * @class    SessionPool
   * @brief    manages a pool of database sessions
   * @details  This uses a queue object to create a pool of database sessions,
   *           to provides for safe multi-threaded access to a MySQL database by
   *           returning a unique connection for each request. A mutex and
   *           condition variable are used for thread synchronization. A pool
   *           of established connections improves performance. Only the session
   *           is pooled; the schema and table names must be provided.
   *
   */
  class SessionPool {
    private:
      std::queue<std::shared_ptr<mysqlx::Session>> _queue;                    ///< pool of database sessions is a queue

      std::shared_ptr<mysqlx::Session> _create_session();                     /// create a single mysqlx database session
      std::shared_ptr<mysqlx::Session> _borrow_session(int timeout_ms=5000);  ///< get a session from pool
      void _return_session(std::shared_ptr<mysqlx::Session> db);              ///< return a session to pool
      bool _test_session(std::shared_ptr<mysqlx::Session> db);                ///< test a session

      // This is the minimum info needed to create a database session/connection.
      std::string _dbhost;
      int         _dbport;
      std::string _dbuser;
      std::string _dbpass;

      std::mutex _mtx;        ///< protects access to the pool
      std::condition_variable _cv;

      /**
       * @brief   helper class to ensure sessions are always returned
       * @details By wrapping the session pool in this class, if something
       *          causes the class to go out of scope, this class's destructor
       *          guarantees the session is returned to the pool.
       */
      class SessionGuard {
        private:
          SessionPool* _pool;                         // pointer to pool to manage
          std::shared_ptr<mysqlx::Session> _session;  // borrowed database session
        public:
          /// borrows a session from the pool on construction
          SessionGuard(SessionPool* pool) : _pool(pool), _session(pool->_borrow_session()) { }

          /// when destructed the session is returned
          ~SessionGuard() { if (_session) _pool->_return_session(_session); }

          /// returns the session that was borrowed on construction
          std::shared_ptr<mysqlx::Session> get() { return _session; }
      };

    public:
      SessionPool(const std::string &host, int port, const std::string &user, const std::string &pass);
      ~SessionPool();

      void insert(const std::string &schemaname, const std::string &tablename,
                  const std::map<std::string, mysqlx::Value> &data);

      void update(const std::string &schemaname, const std::string &tablename,
                  const std::map<std::string, mysqlx::Value> &data,
                  const std::string &condition,
                  const std::map<std::string, mysqlx::Value> &bindings);

      mysqlx::RowResult read(const std::string &schemaname, const std::string &tablename,
                             const std::vector<std::string> &columns,
                             const std::string &where_clause,
                             const std::map<std::string, mysqlx::Value> &bind_params={},
                             const std::string &order_by = "",
                             std::optional<int> limit = std::nullopt,
                             std::optional<int> offset = std::nullopt);

      std::list<std::string> get_tablenames(const std::string &schemaname);
  };
  /***** Database::SessionPool ************************************************/


  /***** Database::Database ***************************************************/
  /**
   * @class    Database
   * @brief    provides an interface to a MySQL database
   * @details  Makes use of SessionPool for safe multi-threaded access.
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

      std::string _dbschema;  ///< optional on construction
      std::string _dbtable;   ///< optional on construction

      std::atomic<bool> _dbconfigured;

      // These map stores in memory the record to be written to
      // the database. The data is essentially a variant type so
      // it can hold values of any type, indexed by column name.
      //
      std::map<std::string, mysqlx::Value> _data;

      std::mutex _data_mtx;       ///< protecs _data map access

      std::unique_ptr<SessionPool> _pool;  ///< this is the pool of database connections
      void _initialize_pool();

    public:
      Database() : _dbconfigured(false) { }
      Database( std::vector<std::string> info );
      // schema and table are optional at construction,
      // specify both, neither, or schema only.
      Database( std::string host,
                int         port,
                std::string user,
                std::string pass,
                std::string schema="",
                std::string table="" );

      ~Database();

      void initialize_class();
      void initialize_class(std::vector<std::string> dbinfo);

      // read() is overloaded
      mysqlx::RowResult read(const std::vector<std::string> &columns,
                             const std::string &where_clause,
                             const std::map<std::string, mysqlx::Value> &bind_params,
                             const std::string &order_by = "",
                             std::optional<int> limit = std::nullopt,
                             std::optional<int> offset = std::nullopt);
      mysqlx::RowResult read(const std::string &tablename,
                             const std::vector<std::string> &columns,
                             const std::string &where_clause,
                             const std::map<std::string, mysqlx::Value> &bind_params,
                             const std::string &order_by = "",
                             std::optional<int> limit = std::nullopt,
                             std::optional<int> offset = std::nullopt);
      mysqlx::RowResult read(const std::string &schemaname, const std::string &tablename,
                             const std::vector<std::string> &columns,
                             const std::string &where_clause,
                             const std::map<std::string, mysqlx::Value> &bind_params={},
                             const std::string &order_by = "",
                             std::optional<int> limit = std::nullopt,
                             std::optional<int> offset = std::nullopt);

      // insert() is overloaded
      void insert();
      void insert(const std::map<std::string, mysqlx::Value> &data);
      void insert(const std::string &tablename,
                  const std::map<std::string, mysqlx::Value> &data);
      void insert(const std::string &schemaname, const std::string &tablename,
                  const std::map<std::string, mysqlx::Value> &data);

      // update() is overloaded
      void update(const std::map<std::string, mysqlx::Value> &data,
                  const std::string &condition,
                  const std::map<std::string, mysqlx::Value> &bindings);
      void update(const std::string &schemaname, const std::string &tablename,
                  const std::map<std::string, mysqlx::Value> &data,
                  const std::string &condition,
                  const std::map<std::string, mysqlx::Value> &bindings);


      // get_tablenames() is overloaded
      std::list<std::string> get_tablenames();
      std::list<std::string> get_tablenames(const std::string &schemaname);

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

      /**
       * @brief      insert an STL map of any single type rather than mysqlx::Value
       * @param[in]  data  map of data of any single type, indexed by field name
       */
      template <typename T>
      void insert( std::map<std::string, T> data ) {
        this->insert(data);
      }
  };
  /***** Database::Database ***************************************************/

}
