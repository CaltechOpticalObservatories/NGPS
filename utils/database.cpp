/**
 * @file    database.cpp
 * @brief   Database class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "database.h"


namespace Database {

  /***** SessionPool::SessionPool *********************************************/
  /**
   * @brief      SessionPool class constructor
   * @details    creates a pool (queue) of connected database sessions on construction
   *
   */
  SessionPool::SessionPool(const std::string &host, int port, const std::string &user, const std::string &pass):
    _dbhost(host), _dbport(port), _dbuser(user), _dbpass(pass) {
    // create the pool of sessions
    for (int i = 0; i < DBPOOLSIZE; ++i) { _queue.push(_create_session()); }
  }
  /***** SessionPool::SessionPool *********************************************/


  /***** SessionPool::~SessionPool ********************************************/
  /**
   * @brief      SessionPool class destructor
   * @details    connections will be automatically closed and queue emptied
   *             when the object is destroyed
   *
   */
  SessionPool::~SessionPool() {
    // Don't want to re-throw anything on exception because no one will be catching
    // it on destruction, but catch here to avoid a potential problem on destruction.
    //
    std::lock_guard<std::mutex> lock(_mtx);
    while (!_queue.empty()) {
      auto db = _queue.front();
      _queue.pop();
      try { db->close(); }
      catch (...) { }
    }
  }
  /***** SessionPool::~SessionPool ********************************************/


  /***** SessionPool::_create_session *****************************************/
  /**
   * @brief      private function to create a single mysqlx database connection
   * @details    Creating a session establishes a connection to the MySQL server.
   *             The DB info comes from the class contruction.
   * @return     shared pointer to a mysqlx::Session
   *
   */
  std::shared_ptr<mysqlx::Session> SessionPool::_create_session() {
    try {
      return std::make_shared<mysqlx::Session>(mysqlx::SessionOption::HOST, _dbhost,
                                               mysqlx::SessionOption::PORT, _dbport,
                                               mysqlx::SessionOption::USER, _dbuser,
                                               mysqlx::SessionOption::PWD,  _dbpass);
    }
    catch (const mysqlx::Error& err) {
      return nullptr;
    }
  }
  /***** SessionPool::_create_session *****************************************/


  /***** SessionPool::_test_session *******************************************/
  /**
   * @brief      tests db connection using a simple table query
   * @paramm[in] db  database handle
   * @return     true|false
   *
   */
  bool SessionPool::_test_session(std::shared_ptr<mysqlx::Session> db) {
    // pass-fail
    try {
      db->sql("SELECT 1").execute();
      return true;                             // it either works,
    }
    catch (...) { return false; }              // or it doesn't.
  }
  /***** SessionPool::_test_session *******************************************/


  /***** SessionPool::_borrow_session *****************************************/
  /**
   * @brief      get a db session from the pool
   * @details    Database connections are always tested before given out, and
   *             if bad/stale then a new one is opened.
   * @return     shared pointer to a mysqlx::Session
   *
   */
  std::shared_ptr<mysqlx::Session> SessionPool::_borrow_session(int timeout_ms) {
    std::unique_lock<std::mutex> lock(_mtx);
    if (!_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {return !_queue.empty(); })) {
      throw std::runtime_error("timeout waiting for database connection");
    }
    auto db = _queue.front();
    _queue.pop();

    // make a new session if this one is bad
    if (!_test_session(db)) db = _create_session();

    return db;
  }
  /***** SessionPool::_borrow_session *****************************************/


  /***** SessionPool::_return_session *****************************************/
  /**
   * @brief      returns a db session back to the pool
   * @param[in]  db  shared pointer to a mysqlx::Session
   *
   */
  void SessionPool::_return_session(std::shared_ptr<mysqlx::Session> db) {
    {
    std::lock_guard<std::mutex> lock(_mtx);
    _queue.push(db);
    }
    _cv.notify_one();
  }
  /***** SessionPool::_return_session *****************************************/


  /***** SessionPool::insert **************************************************/
  /**
   * @brief      insert a record into the database
   * @param[in]  schemaname database schema name
   * @param[in]  tablename  database table name
   * @param[in]  data       reference to map of column/value data
   *
   * This can throw an exception.
   *
   */
  void SessionPool::insert(const std::string &schemaname, const std::string &tablename, const std::map<std::string, mysqlx::Value> &data) {

    if ( data.empty() ) throw std::runtime_error("no data");

    // Create vectors from the supplied STL map, pre-allocating memory for them.
    // Vectors are easily passed directly to the mysqlx API.
    //
    size_t ncols = data.size();
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
      // safely get a DB session from the pool
      SessionGuard session(this);
      auto db = session.get();

      auto schema = std::make_unique<mysqlx::Schema>( *(db), schemaname );
      auto table  = std::make_unique<mysqlx::Table>( schema->getTable( tablename ) );

      table->insert( cols ).values( vals ).execute();
    }
    catch ( const std::exception &err ) {
      throw;
    }
  }
  /***** SessionPool::insert **************************************************/


  /***** SessionPool::update **************************************************/
  /**
   * @brief      update an existing record
   * @details    may throw an exception
   * @param[in]  schemaname  schema name (optional)
   * @param[in]  tablename   table name (optional)
   * @param[in]  setdata     STL map containing mysqlx data indexed by DB column name
   *
   */
  void SessionPool::update(const std::string &schemaname, const std::string &tablename,
                           const std::map<std::string, mysqlx::Value> &setdata,
                           const std::string &condition,
                           const std::map<std::string, mysqlx::Value> &bindings) {

    if ( setdata.empty() ) throw std::runtime_error("no data to update");

    // Update the database table
    //
    try {
      // safely get a DB session from the pool
      SessionGuard session(this);
      auto db = session.get();

      auto schema = std::make_unique<mysqlx::Schema>( *(db), schemaname );
      auto table  = std::make_unique<mysqlx::Table>( schema->getTable( tablename ) );

      // create an updater
      auto updater = table->update();

      // apply .set() calls for each column-data pair
      for ( const auto &[column, data] : setdata ) updater.set( column, data );

      updater.where(condition);

      // apply .bindings()
      for ( const auto &[key, val] : bindings ) updater.bind( key, val );

      // perform the update
      updater.execute();
    }
    catch ( const std::exception &err ) {
      throw;
    }
  }
  /***** SessionPool::update **************************************************/


  /***** SessionPool::read ****************************************************/
  /**
   * @brief      read from the database
   * @param[in]  schemaname
   * @param[in]  tablename
   * @param[in]  columns
   * @param[in]  where_clause
   * @param[in]  bind_params
   * @param[in]  order_by
   * @param[in]  limit
   * @param[in]  offset
   * @return     mysqlx::RowResult
   *
   * This can throw an exception.
   *
   */
  mysqlx::RowResult SessionPool::read(const std::string &schemaname, const std::string &tablename,
                                      const std::vector<std::string> &columns,
                                      const std::string &where_clause,
                                      const std::map<std::string, mysqlx::Value> &bind_params,
                                      const std::string &order_by,
                                      std::optional<int> limit,
                                      std::optional<int> offset) {
    try {
      // safely get a DB session from the pool
      SessionGuard session(this);
      auto db = session.get();

      auto schema = std::make_unique<mysqlx::Schema>( *(db), schemaname );
      auto table  = std::make_unique<mysqlx::Table>( schema->getTable( tablename ) );

      mysqlx::TableSelect select = table->select(columns);

      if ( !where_clause.empty() ) select = select.where(where_clause);
      if ( !order_by.empty() )     select = select.orderBy(order_by);

      for (const auto &[key, val] : bind_params) select.bind(key, val);

      if (limit)  select = select.limit(*limit);
      if (offset) select = select.offset(*offset);

      return select.execute();
    }
    catch (const std::exception &e) { throw; }
  }
  /***** SessionPool::read ****************************************************/


  /***** SessionPool::get_tablenames ******************************************/
  /**
   * @brief      return a list of database tablenames
   * @param[in]  schemaname  schema name
   * @return     list of strings
   *
   */
  std::list<std::string> SessionPool::get_tablenames(const std::string &schemaname) {
    try {
      // safely get a DB session from the pool
      SessionGuard session(this);
      auto db = session.get();
      auto schema = std::make_unique<mysqlx::Schema>( *(db), schemaname );
      return schema->getTableNames();
    }
    catch (const std::exception &e) { throw; }
  }
  /***** SessionPool::get_tablenames ******************************************/


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
      if (!_dbhost.empty() && !_dbuser.empty() && !_dbpass.empty() && _dbport>0) {
        _dbconfigured = true;
        _initialize_pool();
      }
      else throw std::runtime_error("bad or missing database info");
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
    try {
      switch ( dbinfo.size() ) {
        case 6:
          _dbtable  = dbinfo.at(5);  // optional at construction
        case 5:
          _dbschema = dbinfo.at(4);  // optional at construction
        case 4:
          _dbpass   = dbinfo.at(3);
          _dbuser   = dbinfo.at(2);
          _dbport   = std::stoi( dbinfo.at(1) );
          _dbhost   = dbinfo.at(0);
          break;
        default:
          throw std::invalid_argument( "bad info vector size constructing Database object" );
      }
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
    _pool = std::make_unique<SessionPool>(_dbhost, _dbport, _dbuser, _dbpass);
  }
  /***** Database::initialize_pool ********************************************/


  /***** SessionPool::read ****************************************************/
  /**
   * @brief      read from the database
   * @details    may throw an exception
   * @param[in]  schemaname     optional
   * @param[in]  tablename      optional
   * @param[in]  columns
   * @param[in]  where_clause
   * @param[in]  bind_params
   * @param[in]  order_by
   * @param[in]  limit
   * @param[in]  offset
   * @return     mysqlx::RowResult
   *
   */
  mysqlx::RowResult Database::read(const std::vector<std::string> &columns,
                                   const std::string &where_clause,
                                   const std::map<std::string, mysqlx::Value> &bind_params,
                                   const std::string &order_by,
                                   std::optional<int> limit,
                                   std::optional<int> offset) {
    if ( _dbschema.empty() || _dbtable.empty() ) throw std::runtime_error("missing schema or table");
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      return _pool->read( _dbschema, _dbtable, columns, where_clause, bind_params, order_by, limit, offset );
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::read", "ERROR reading: "+std::string(e.what()));
      throw;
    }
  }
  /***** SessionPool::read ****************************************************/
  mysqlx::RowResult Database::read(const std::string &tablename,
                                   const std::vector<std::string> &columns,
                                   const std::string &where_clause,
                                   const std::map<std::string, mysqlx::Value> &bind_params,
                                   const std::string &order_by,
                                   std::optional<int> limit,
                                   std::optional<int> offset) {
    if ( _dbschema.empty() ) throw std::runtime_error("missing schema");
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      return _pool->read( _dbschema, tablename, columns, where_clause, bind_params, order_by, limit, offset );
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::read", "ERROR reading: "+std::string(e.what()));
      throw;
    }
  }
  /***** SessionPool::read ****************************************************/
  mysqlx::RowResult Database::read(const std::string &schemaname, const std::string &tablename,
                                   const std::vector<std::string> &columns,
                                   const std::string &where_clause,
                                   const std::map<std::string, mysqlx::Value> &bind_params,
                                   const std::string &order_by,
                                   std::optional<int> limit,
                                   std::optional<int> offset) {
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      return _pool->read( schemaname, tablename, columns, where_clause, bind_params, order_by, limit, offset );
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::read", "ERROR reading: "+std::string(e.what()));
      throw;
    }
  }
  /***** SessionPool::read ****************************************************/


  /***** Database::insert *****************************************************/
  /**
   * @brief      insert the supplied data to the indicated database table
   * @details    may throw an exception
   * @param[in]  schemaname  schema name (optional)
   * @param[in]  tablename   table name (optional)
   * @param[in]  data        STL map containing mysqlx data indexed by DB column name
   *
   */
  void Database::insert(const std::map<std::string, mysqlx::Value> &data) {
    // schema or table names must come from the class
    if ( _dbschema.empty() || _dbtable.empty() ) throw std::runtime_error("missing schema or table");
    insert(_dbschema, _dbtable, data);
  }
  /***** Database::insert *****************************************************/
  void Database::insert(const std::string &tablename,
                        const std::map<std::string, mysqlx::Value> &data) {
    // schema name must come from the class
    if ( _dbschema.empty() ) throw std::runtime_error("missing schema");
    insert(_dbschema, tablename, data);
  }
  /***** Database::insert *****************************************************/
  void Database::insert(const std::string &schemaname, const std::string &tablename,
                        const std::map<std::string, mysqlx::Value> &data) {
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      _pool->insert(schemaname, tablename, data);
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::insert",
               "ERROR writing data: "+std::string(e.what()));
      throw;
    }
  }
  /***** Database::insert *****************************************************/


  /***** Database::insert *****************************************************/
  /**
   * @brief      insert the class private _data to the MySQL database
   * @details    may throw an exception
   *             This function overloads the version which accepts a map,
   *             by passing it the class map
   *
   */
  void Database::insert() {
    std::lock_guard<std::mutex> lock(_data_mtx);
    this->insert( this->_data );  // insert the record stored in the class,
    this->_data.clear();          // then erase it.
  }
  /***** Database::insert *****************************************************/


  /***** Database::update *****************************************************/
  /**
   * @brief      update column(s) in existing database record
   * @details    Gets database session from pool. Schema and Table come from
   *             the class. may throw an exception
   * @param[in]  data       map of data
   * @param[in]  condition
   * @param[in]  bindings
   *
   */
  void Database::update(const std::map<std::string, mysqlx::Value> &data,
                        const std::string &condition,
                        const std::map<std::string, mysqlx::Value> &bindings) {
    // schema or table names must come from the class
    if ( _dbschema.empty() || _dbtable.empty() ) throw std::runtime_error("missing schema or table");
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      _pool->update(_dbschema, _dbtable, data, condition, bindings);
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::get_tablenames",
               "ERROR: "+std::string(e.what()));
      throw;
    }
  }
  /***** Database::update *****************************************************/
  /**
   * @brief      update column(s) in existing database record
   * @details    Gets database session from pool. may throw an exception
   * @param[in]  schemaname  name of schema
   * @param[in]  tablename   name of table
   * @param[in]  data        map of data
   * @param[in]  condition
   * @param[in]  bindings
   *
   */
  void Database::update(const std::string &schemaname, const std::string &tablename,
                        const std::map<std::string, mysqlx::Value> &data,
                        const std::string &condition,
                        const std::map<std::string, mysqlx::Value> &bindings) {
    // schema or table names come from args
    if ( schemaname.empty() || tablename.empty() ) throw std::runtime_error("missing schema or table");
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      _pool->update(schemaname, tablename, data, condition, bindings);
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::get_tablenames",
               "ERROR: "+std::string(e.what()));
      throw;
    }
  }
  /***** Database::update *****************************************************/


  /***** Database::get_tablenames *********************************************/
  /**
   * @brief      return a list of database tablenames
   * @return     list of strings
   *
   */
  std::list<std::string> Database::get_tablenames() {
    // schema must come from the class
    if ( _dbschema.empty() ) throw std::runtime_error("missing schema");
    return get_tablenames(_dbschema);
  }
  /***** Database::get_tablenames *********************************************/
  /**
   * @brief      return a list of database tablenames
   * @param[in]  schemaname  schema name
   * @return     list of strings
   *
   */
  std::list<std::string> Database::get_tablenames(const std::string &schemaname) {
    try {
      if (!_pool) throw std::runtime_error("not connected to database");
      return _pool->get_tablenames(schemaname);
    }
    catch (const std::exception &e) {
      logwrite("Database::Database::get_tablenames",
               "ERROR: "+std::string(e.what()));
      throw;
    }
  }
  /***** Database::get_tablenames *********************************************/
}
