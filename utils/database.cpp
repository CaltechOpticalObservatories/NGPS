/**
 * @file    database.cpp
 * @brief   Database class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "database.h"


namespace Database {

  /***** Database::Database ***************************************************/
  /**
   * @brief      Database class constructor
   * @details    may throw an exception on error
   * @param[in]  info  vector of database parameters: {host,port,user,pass,schema,table}
   *
   */
  Database::Database( std::vector<std::string> info ) {
    this->initialize_class( info );
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
    _dbhost( host ), _dbport( port ), _dbuser( user ), _dbpass( pass ), _dbschema( schema ), _dbtable( table ) {

    _dbconnected  = false;
    _dbconfigured = true;

    _create_connection();  // may throw exception

    return;
  }
  /***** Database::Database ***************************************************/


  /***** Database::initialize_class *******************************************/
  /**
   * @brief      initialize database class object
   * @details    may throw an exception on error
   * @param[in]  info  vector of database parameters: {host,port,user,pass,schema,table}
   *
   */
  void Database::initialize_class( std::vector<std::string> info ) {
    std::string function = "Database::Database::initialize_class";
    _dbconfigured = false;
    _dbconnected  = false;  // set true in _create_connection()

    if ( info.size() != 6 ) {
      logwrite( function, "ERROR constructing: bad info vector. check config file" );
      throw std::invalid_argument( "constructing Database object: bad info vector. check config file" );
    }

    try {
      _dbhost   = info.at(0);
      _dbport   = std::stoi( info.at(1) );
      _dbuser   = info.at(2);
      _dbpass   = info.at(3);
      _dbschema = info.at(4);
      _dbtable  = info.at(5);
    }
    catch( std::exception &e ) {
      std::stringstream message;
      message << "ERROR constructing Database object (check config file): " << e.what();
      logwrite( function, message.str() );
      throw std::exception( e );
    }

    _dbconfigured = true;  // configured now

    _create_connection();  // may throw exception

    return;
  }
  /***** Database::initialize_class *******************************************/


  /***** Database::_create_connection *****************************************/
  /**
   * @brief      private function to create a single mysqlx database connection
   * @details    The DB info comes from the class contruction.
   *             This function will throw an exception on error.
   *
   */
  void Database::_create_connection() {
    std::string function = "Database::Database::_create_connection";
    std::stringstream message;

    try {
      _session = std::make_unique<mysqlx::Session> ( mysqlx::SessionOption::HOST,   _dbhost,
                                                     mysqlx::SessionOption::PORT,   _dbport,
                                                     mysqlx::SessionOption::USER,   _dbuser,
                                                     mysqlx::SessionOption::PWD,    _dbpass );

      _sessionopen = true;

      _schema  = std::make_unique<mysqlx::Schema>( *_session, _dbschema );
      _table   = std::make_unique<mysqlx::Table>( _schema->getTable( _dbtable ) );

    }
    catch ( const mysqlx::Error &err ) {
      message << "ERROR: " << err;
      logwrite( function, message.str() );
      throw mysqlx::Error( err );
    }
    catch ( std::exception &e ) {
      message << "ERROR: " << e.what();
      logwrite( function, message.str() );
      throw std::exception( e );
    }

    _dbconnected = true;

    return;
  }
  /***** Database::_create_connection *****************************************/


  /***** Database::close ******************************************************/
  /**
   * @brief      close a database connection
   * @brief      may throw an exception
   *
   */
  void Database::close() {
    std::string function = "Database::Database::close";
    std::stringstream message;

    try {
      if ( _sessionopen ) _session->close();
      _sessionopen = false;

      _table.reset();
      _schema.reset();
      _session.reset();

      _dbconnected = false;
    }
    catch ( const mysqlx::Error &err ) {
      message.str(""); message << "ERROR from mySQL: " << err;
      logwrite( function, message.str() );
      throw mysqlx::Error( err );
    }

    return;
  }
  /***** Database::close ******************************************************/


  /***** Database::~Database **************************************************/
  /**
   * @brief      Database class destructor
   * @details    connection will be automatically closed when the object is destroyed
   *
   */
  Database::~Database() {
    // Don't want to re-throw anything on exception because no one will be catching
    // it on destruction, but catch here to avoid a potential problem on destruction.
    //
    try { if ( _sessionopen ) _session->close(); }
    catch ( const mysqlx::Error &err ) { }
  }
  /***** Database::~Database **************************************************/


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

    if ( ! _dbconnected || ! _session || ! _sessionopen ) {
      logwrite( function, "ERROR not connected to database or session not open" );
      throw std::runtime_error( "not connected to database or session not open" );
    }

    if ( ! _table || ! _schema ) {
      logwrite( function, "ERROR table or schema not initialized" );
      throw std::runtime_error( "table or schema not initialized" );
    }

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
      logwrite( function, "ERROR data empty or improperly formatted" );
      throw std::runtime_error( "data empty or improperly formatted" );
    }

    // Insert a row into the database table
    //
    try {
      _table->insert( cols ).values( vals ).execute();
    }
    catch ( const mysqlx::Error &err ) {
      message.str(""); message << "ERROR from mySQL: " << err.what();
      logwrite( function, message.str() );
      throw mysqlx::Error( err );
    }
    catch ( const std::exception &err ) {
      message.str(""); message << "ERROR: " << err.what();
      logwrite( function, message.str() );
      throw std::exception( err );
    }
    catch ( ... ) {
      logwrite( function, "ERROR unknown exception" );
      throw;
    }

    return;
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
    this->write( this->_data );  // Write the record stored in the class,
    this->_data.clear();         // then erase it.
  }
  /***** Database::write ******************************************************/

}
