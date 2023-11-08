/**
 * @file    database.h
 * @brief   defines the Database class for communicating with a MySQL database
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef DATABASE_H
#define DATABASE_H

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

    public:
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

      void close();

      void write( std::map<std::string, std::string> data );

  };
  /***** Database::Database ***************************************************/

}
#endif
