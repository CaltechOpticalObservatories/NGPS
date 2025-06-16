/**
 * @file    database.cpp
 * @brief   Database class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "database.h"


namespace Database {

  /***** DatabasePool::DatabasePool *******************************************/
  /**
   * @brief      DatabasePool class constructor
   * @details    creates a pool (queue) of connected database sessions on construction
   *
   */
  DatabasePool::DatabasePool(const std::string &host, int port, const std::string &user,
                             const std::string &pass, const std::string &schema,
                             const std::string &table, int poolsz) :
    _dbhost(host), _dbport(port), _dbuser(user), _dbpass(pass),
    _dbschema(schema), _dbtable(table), _poolsz(poolsz) {
    // create the pool of sessions
    for (int i = 0; i < _poolsz; ++i) { _db_queue.push(_create_handle()); }
  }
  /***** DatabasePool::DatabasePool *******************************************/


  /***** DatabasePool::~DatabasePool ******************************************/
  /**
   * @brief      DatabasePool class destructor
   * @details    connections will be automatically closed and queue emptied
   *             when the object is destroyed
   *
   */
  DatabasePool::~DatabasePool() {
    // Don't want to re-throw anything on exception because no one will be catching
    // it on destruction, but catch here to avoid a potential problem on destruction.
    //
    std::lock_guard<std::mutex> lock(_mtx);
    while (!_db_queue.empty()) {
      auto db = _db_queue.front();
      _db_queue.pop();
      try { db->session->close(); }
      catch (...) { }
    }
  }
  /***** DatabasePool::~DatabasePool ******************************************/


  /***** DatabasePool::_create_handle *****************************************/
  /**
   * @brief      private function to create a single mysqlx database connection
   * @details    Creating a session establishes a connection to the MySQL server.
   *             The DB info comes from the class contruction.
   *             This function will throw an exception on error.
   * @return     shared pointer to a DatabaseHandle
   *
   */
  std::shared_ptr<DatabasePool::DatabaseHandle> DatabasePool::_create_handle() {
    auto db = std::make_shared<DatabaseHandle>();

    db->session = std::make_unique<mysqlx::Session>( mysqlx::SessionOption::HOST, _dbhost,
                                                     mysqlx::SessionOption::PORT, _dbport,
                                                     mysqlx::SessionOption::USER, _dbuser,
                                                     mysqlx::SessionOption::PWD,  _dbpass );
    db->schema  = std::make_unique<mysqlx::Schema>( *(db->session), _dbschema );
    db->table   = std::make_unique<mysqlx::Table>( db->schema->getTable( _dbtable ) );

    return db;
  }
  /***** DatabasePool::_create_handle *****************************************/


  /***** DatabasePool::_test_connection ***************************************/
  /**
   * @brief      tests db connection using a simple table query
   * @paramm[in] db  database handle
   * @return     true|false
   *
   */
  bool DatabasePool::_test_connection(std::shared_ptr<DatabasePool::DatabaseHandle> db) {
    // pass-fail
    try {
      db->session->sql("SELECT 1").execute();
      return true;                             // it either works,
    }
    catch (...) { return false; }              // or it doesn't.
  }
  /***** DatabasePool::_test_connection ***************************************/


  /***** DatabasePool::_borrow_handle *****************************************/
  /**
   * @brief      get a db handle from the pool
   * @details    Database connections are always tested before given out, and
   *             if bad/stale then a new one is opened.
   * @return     shared pointer to a DatabaseHandle
   *
   */
  std::shared_ptr<DatabasePool::DatabaseHandle> DatabasePool::_borrow_handle(int timeout_ms) {
    std::unique_lock<std::mutex> lock(_mtx);
    if (!_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {return !_db_queue.empty(); })) {
      throw std::runtime_error("timeout waiting for database connection");
    }
    auto db = _db_queue.front();
    _db_queue.pop();

    // make a new handle if this one is bad
    if (!_test_connection(db)) db = _create_handle();

    return db;
  }
  /***** DatabasePool::_borrow_handle *****************************************/


  /***** DatabasePool::_return_handle *****************************************/
  /**
   * @brief      returns a db handle back to the pool
   * @param[in]  db  shared pointer to a DatabaseHandle
   *
   */
  void DatabasePool::_return_handle(std::shared_ptr<DatabasePool::DatabaseHandle> db) {
    {
    std::lock_guard<std::mutex> lock(_mtx);
    _db_queue.push(db);
    }
    _cv.notify_one();
  }
  /***** DatabasePool::_return_handle *****************************************/


  /***** DatabasePool::write **************************************************/
  /**
   * @brief      write the supplied data to the database table
   * @details    may throw an exception
   * @param[in]  data  STL map containing mysqlx data indexed by DB column name
   *
   * This function is overloaded by a version that writes the data stored
   * in the class.
   *
   */
  void DatabasePool::write( std::map<std::string, mysqlx::Value> data ) {

    if (data.empty()) throw std::runtime_error("no data");

    // Create vectors from the supplied STL map, pre-allocating memory for them.
    // Vectors are easily passed directly to the mysqlx API.
    //
    int ncols = data.size();
    std::vector<std::string>   cols; cols.reserve( ncols );
    std::vector<mysqlx::Value> vals; vals.reserve( ncols );

    for ( const auto &dat : data ) {
      cols.push_back( dat.first );   // database columns
      vals.push_back( dat.second );  // value for that column
    }

    if ( cols.empty() || vals.empty() || cols.size() != vals.size() ) {
      throw std::runtime_error( "data empty or improperly formatted" );
    }

    // Insert a row into the database table
    //
    try {
      // safely get a DB handle from the pool
      HandleGuard handle(this);
      auto db = handle.get();

      db->table->insert( cols ).values( vals ).execute();
    }
    catch ( const std::exception &err ) {
      throw;
    }
  }
  /***** DatabasePool::write **************************************************/


  /***** Database::get_mysql_type *********************************************/
  /**
   * @brief      gets a string representation of the type of mysqlx value
   * @param[in]  value  mysqlx::Value
   * @param[out] type   return string representing type of value
   *
   */
  void get_mysql_type( mysqlx::Value value, std::string &type ) {

    if ( value.isNull() ) {
      type="NULL";
      return;
    }

    if ( value.getType() == mysqlx::Value::VNULL ) {
      type="NULL";
    }
    else
    if ( value.getType() == mysqlx::Value::UINT64 ) {
      type="uint64_t";
    }
    else
    if ( value.getType() == mysqlx::Value::INT64 ) {
      type="int64_t";
    }
    else
    if ( value.getType() == mysqlx::Value::FLOAT ) {
      type="float";
    }
    else
    if ( value.getType() == mysqlx::Value::DOUBLE ) {
      type="double";
    }
    else
    if ( value.getType() == mysqlx::Value::BOOL ) {
      type="bool";
    }
    else
    if ( value.getType() == mysqlx::Value::STRING ) {
      type="string";
    }
    else
    if ( value.getType() == mysqlx::Value::RAW ) {
      type="raw";
    }
    else
    if ( value.getType() == mysqlx::Value::ARRAY ) {
      type="array";
    }
    else
      type="unknown";

    return;
  }
  /***** Database::get_mysql_type *********************************************/


  /***** Database::Database ***************************************************/
  /**
   * @brief      Database class constructor
   * @details    may throw an exception on error
   * @param[in]  info  vector of database parameters: {host,port,user,pass,schema,table}
   *
   */
  Database::Database( std::vector<std::string> info ) :
    _dbconfigured(false), _pool(nullptr) {
    initialize_class(info);
  }
  /***** Database::Database ***************************************************/


  /***** Database::Database ***************************************************/
  /**
   * @brief      Database class constructor
   * @details    may throw an exception on error
   * @param[in]  host    hostname serving database
   * @param[in]  port    port (integer) on host
   * @param[in]  user    user on host
   * @param[in]  pass    password for user
   * @param[in]  schema  database schema
   * @param[in]  table   database table in schema
   *
   */
  Database::Database( std::string host,
                      int         port,
                      std::string user,
                      std::string pass,
                      std::string schema,
                      std::string table ) :
    _dbhost( host ), _dbport( port ), _dbuser( user ), _dbpass( pass ), _dbschema( schema ), _dbtable( table ),
    _dbconfigured(false), _pool(nullptr) {

    initialize_class();
  }
  /***** Database::Database ***************************************************/


  Database::~Database() {
    try {
      if (_pool) _pool.reset();
    }
    catch (...) { }
  }


  /***** Database::initialize_class *******************************************/
  /**
   * @brief      initialize database class object using stored class information
   * @details    may throw an exception on error
   *
   */
  void Database::initialize_class() {
    try {
      if (!_dbhost.empty() && !_dbuser.empty() && !_dbpass.empty() &&
          !_dbschema.empty() && !_dbtable.empty() && _dbport>0) {
        _dbconfigured = true;
        _initialize_pool();
      }
      else throw std::runtime_error("bad or missing database info (check config file)");
    }
    catch( const std::exception &e ) {
      throw;
    }
  }
  /***** Database::initialize_class *******************************************/
  /**
   * @brief      initialize database class object using supplied information
   * @details    may throw an exception on error
   * @param[in]  info  vector of database parameters: {host,port,user,pass,schema,table}
   *
   */
  void Database::initialize_class(std::vector<std::string> dbinfo) {
    if ( dbinfo.size() != 6 ) {
      throw std::invalid_argument( "bad info vector size constructing Database object (check config file)" );
    }

    try {
      _dbhost   = dbinfo.at(0);
      _dbport   = std::stoi( dbinfo.at(1) );
      _dbuser   = dbinfo.at(2);
      _dbpass   = dbinfo.at(3);
      _dbschema = dbinfo.at(4);
      _dbtable  = dbinfo.at(5);

      initialize_class();
    }
    catch( const std::exception &e ) {
      throw;
    }
  }
  /***** Database::initialize_class *******************************************/


  /***** Database::initialize_pool ********************************************/
  /**
   * @brief      initialize database pool
   * @details    may throw an exception on error
   *
   */
  void Database::_initialize_pool() {
    if (!_dbconfigured) throw std::runtime_error("database not configured");

    // create a pool which will own all DB connections
    _pool = std::make_unique<DatabasePool>(_dbhost, _dbport, _dbuser, _dbpass, _dbschema, _dbtable,
                                           DBPOOLSIZE);
  }
  /***** Database::initialize_pool ********************************************/


  /***** Database::write ******************************************************/
  /**
   * @brief      write the supplied data to the indicated database table
   * @details    may throw an exception
   * @param[in]  data  STL map containing mysqlx data indexed by DB column name
   *
   * This function is overloaded by a version that writes the data stored
   * in the class.
   *
   */
  void Database::write( std::map<std::string, mysqlx::Value> data ) {
    std::string function = "Database::Database::write";
    std::stringstream message;

    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      _pool->write(data);
    }
    catch (const std::exception &e) {
      logwrite(function, "ERROR writing data: "+std::string(e.what()));
    }
  }
  /***** Database::write ******************************************************/


  /***** Database::write ******************************************************/
  /**
   * @brief      write the class private _data to the MySQL database
   * @details    may throw an exception
   *             This function overloads the version which accepts a map,
   *             by passing it the class map
   *
   */
  void Database::write() {
    std::lock_guard<std::mutex> lock(_data_mtx);
    this->write( this->_data );  // Write the record stored in the class,
    this->_data.clear();         // then erase it.
  }
  /***** Database::write ******************************************************/

}
