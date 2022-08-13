#include "sequencer_interface.h"
#define BUFSIZE 1024

namespace Sequencer {

  /** Sequencer::TargetInfo::TargetInfo ***************************************/
  /**
   * @fn     TargetInfo
   * @brief  class constructor
   * @param  none
   * @return none
   *
   * This constructor initializes the database configuration variables as empty,
   * requiring them to be properly configured from the .cfg file.
   *
   * This also defines the target list fields which are accessed in the active 
   * target table, the table identified by DB_ACTIVE.
   *
   */
  TargetInfo::TargetInfo() {
    this->db_host       = "";
    this->db_port       = -1;
    this->db_user       = "";
    this->db_pass       = "";
    this->db_schema     = "";
    this->db_active     = "";
    this->db_completed  = "";
    this->db_configured = false;

    this->init_record();

    // These are the fields (or columns) to be accessed in the DB_ACTIVE table.
    // They can be listed in any order.
    //
    this->targetlist = { "OBSERVATION_ID",
                         "OBS_ORDER",
                         "STATE",
                         "NAME",
                         "RA",
                         "DECL",
                         "EPOCH",
                         "EXPTIME",
                         "NEXP",
                         "SLITWIDTH",
                         "SLITOFFSET",
                         "BINSPECT",
                         "BINSPAT",
                         "CASANGLE"
                       };                /// initialize the target list fields for accessing the active target table
  }
  /** Sequencer::TargetInfo::TargetInfo ***************************************/


  /** Sequencer::TargetInfo::~TargetInfo **************************************/
  /**
   * @fn     ~TargetInfo
   * @brief  class deconstructor
   * @param  
   * @return 
   *
   */
  TargetInfo::~TargetInfo() {
  }
  /** Sequencer::TargetInfo::~TargetInfo **************************************/


  /** Sequencer::TargetInfo::colnum *******************************************/
  /**
   * @fn         colnum
   * @brief      get column number of requested field from this->targetlist
   * @param[in]  field, string to search for
   * @return     integer, -1 on error
   *
   * This function is used to return the location of the field name in the targetlist
   * vector. This is needed because the mysqlx Connector/C++ X DEV API does not have
   * its own capability for this. If the field is not found then return -1. This should
   * cause a std::exception row column to be thrown if you try to access a row[ -1 ].
   *
   */
  int TargetInfo::colnum( std::string field ) {
    std::string function = "Sequencer::TargetInfo::colnum";
    std::stringstream message;
    std::vector<std::string>::iterator it;
    it = std::find( this->targetlist.begin(), this->targetlist.end(), field );
    if ( it != this->targetlist.end() ) {
      return ( it - this->targetlist.begin() );
    }
    else {
      message.str(""); message << "ERROR: requested field " << field << " not found in targetlist";
      logwrite( function, message.str() );
      return -1;
    }
  }
  /** Sequencer::TargetInfo::colnum *******************************************/


  /** Sequencer::TargetInfo::init_record **************************************/
  /**
   * @fn         init_record
   * @brief      initialize current target record variables
   * @param[in]  none
   * @return     none
   *
   */
  void TargetInfo::init_record() {
    std::string function = "Sequencer::TargetInfo::init_record";
    std::stringstream message;
    this->obsid=-1;
    this->obsorder=-1;
    this->name="";
    this->ra="";
    this->dec="";
    this->epoch="";
    this->casangle="";
    this->slitwidth=-1;
    this->slitoffset=-1;
    this->exptime=-1;
    this->nexp=-1;
    this->obsplan="";
    this->binspect=-1;
    this->binspat=-1;

  }
  /** Sequencer::TargetInfo::init_record **************************************/


  /** Sequencer::TargetInfo::configure_db *************************************/
  /**
   * @fn         configure_db
   * @brief      configure database connection parameters
   * @param[in]  param, name of parameter
   * @param[in]  value, value of parameter
   * @return     ERROR or NO_ERROR
   *
   */
  long TargetInfo::configure_db( std::string param, std::string value ) {
    std::string function = "Sequencer::TargetInfo::configure_db";
    std::stringstream message;
    long error = NO_ERROR;

    if ( param == Sequencer::DB_HOST ) {
      this->db_host = value;
    }
    else
    if ( param == Sequencer::DB_PORT ) {
      int port;
      try {
        port = std::stoi( value );
      }
      catch (std::invalid_argument &) {
        message.str(""); message << "ERROR: bad value " << value << " for parameter " << param << ": unable to convert to integer";
        logwrite(function, message.str() );
        return( ERROR );
      }
      catch (std::out_of_range &) {
        message.str(""); message << "ERROR: bad value " << value << " for parameter " << param << ": out of integer range";
        logwrite(function, message.str() );
        return( ERROR );
      }
      this->db_port = port;
    }
    else
    if ( param == Sequencer::DB_USER ) {
      this->db_user = value;
    }
    else
    if ( param == Sequencer::DB_PASS ) {
      this->db_pass = value;
    }
    else
    if ( param == Sequencer::DB_SCHEMA ) {
      this->db_schema = value;
    }
    else
    if ( param == Sequencer::DB_ACTIVE ) {
      this->db_active = value;
    }
    else
    if ( param == Sequencer::DB_COMPLETED ) {
      this->db_completed = value;
    }
    else
    {
      message.str(""); message << "ERROR: unknown parameter \"" << param << "\"";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // If everything has been set then set the db_configured state true
    //
    if ( !this->db_host.empty()       &&
         !this->db_user.empty()       &&
         !this->db_pass.empty()       &&
         !this->db_schema.empty()     &&
         !this->db_active.empty()     &&
         !this->db_completed.empty()  &&
         this->db_port  != -1         ) {
      this->db_configured = true;
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] host=" << this->db_host << " port=" << this->db_port
                               << " user=" << this->db_user << " pass=" << this->db_pass
                               << " schema=" << this->db_schema << " active table=" << this->db_active
                               << " completed table=" << this->db_completed;
      logwrite( function, message.str() );
#endif
    }
    else { this->db_configured = false; }

    return( error );
  }
  /** Sequencer::TargetInfo::configure_db *************************************/



  /** Sequencer::TargetInfo::add_row ******************************************/
  /**
   * @fn         add_row
   * @brief      adds a row to the database (non-production)
   * @param[in]  none
   * @return     none
   *
   * This is for testing purposes. Adds a row to the database.
   *
   */
  long TargetInfo::add_row() {
    std::string function = "Sequencer::TargetInfo::add_row";
    std::stringstream message;

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      return( ERROR );
    }

    try {
      // create a session for accessing the database
      //
      mysqlx::Session mySession( mysqlx::SessionOption::HOST, this->db_host,
                                 mysqlx::SessionOption::PORT, this->db_port,
                                 mysqlx::SessionOption::USER, this->db_user,
                                 mysqlx::SessionOption::PWD,  this->db_pass   );

      // connect to database
      //
      mysqlx::Schema db( mySession, this->db_schema );

      // create a table object
      //
      mysqlx::Table targettable = db.getTable( this->db_active );

      // add the row
      //
      targettable.insert( "OBSERVATION_ID",
                         "SET_ID",
                         "OBS_ORDER",
                         "STATE",
                         "NAME",
                         "RA",
                         "DECL",
                         "EPOCH",
                         "EXPTIME",
                         "NEXP",
                         "SLITWIDTH",
                         "SLITOFFSET",
                         "BINSPECT",
                         "BINSPAT",
                         "CASANGLE"
                        )
                .values( 6,
                         1,
                         999,
                         "test",
                         "ZTF22aajmnr6",
                         "06:42:57.35",
                         "+51:16:18.53",
                         "J2000",
                         0,
                         1,
                         1.0,
                         0,
                         1,
                         1,
                         0 )
                .execute();
    }
    catch ( const mysqlx::Error &err ) {
      message.str(""); message << "ERROR from mySQL: " << err;
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::exception &ex ) {
      message.str(""); message << "ERROR std exception: " << ex.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( const char *ex ) {
      message.str(""); message << "ERROR other exception: " << ex;
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /** Sequencer::TargetInfo::add_row ******************************************/


  /** Sequencer::TargetInfo::get_next *****************************************/
  /**
   * @fn         get_next
   * @brief      get next target from DB whose state is Sequencer::TARGET_PENDING
   * @param[in]  none
   * @return     ERROR, NO_ERROR, TARGET_FOUND, TARGET_NOT_FOUND
   *
   * This function is overloaded.
   *
   * This version takes no argument. When no argument is supplied then the
   * other function is called with Sequencer::TARGET_PENDING as the argument.
   * This is the normal default usage to look for targets in the TARGET_PENDING
   * state.
   *
   */
  TargetInfo::TargetState TargetInfo::get_next() {
    return( this->get_next( Sequencer::TARGET_PENDING ) );
  }
  /** Sequencer::TargetInfo::get_next *****************************************/


  /** Sequencer::TargetInfo::get_next *****************************************/
  /**
   * @fn         get_next
   * @brief      get next target from DB whose state is state_in
   * @param[in]  state_in, the state to search for
   * @return     ERROR, NO_ERROR, TARGET_FOUND, TARGET_NOT_FOUND
   *
   * This function is overloaded.
   *
   * This version accepts (and requires) a state to look for.
   *
   */
  TargetInfo::TargetState TargetInfo::get_next( std::string state_in ) {
    std::string function = "Sequencer::TargetInfo::get_next";
    std::stringstream message;
    mysqlx::row_count_t rowcount=-1;  /// number of rows that match the select criteria
    mysqlx::col_count_t col=-1;       /// column number returned from this->colnum( FIELD )
    mysqlx::col_count_t colcount=-1;  /// the number of columns in the currently read row

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }

    try {
      // create a session for accessing the database
      //
      mysqlx::Session mySession( mysqlx::SessionOption::HOST, this->db_host,
                                 mysqlx::SessionOption::PORT, this->db_port,
                                 mysqlx::SessionOption::USER, this->db_user,
                                 mysqlx::SessionOption::PWD,  this->db_pass   );

      // connect to database
      //
      mysqlx::Schema db( mySession, this->db_schema );

      // create a table object
      //
      mysqlx::Table targettable = db.getTable( this->db_active );

      // Find a row in the SQL active observations table,
      // the next one (in order) where state is state_in.
      //
      mysqlx::RowResult result = targettable.select( this->targetlist )
                                           .where( "STATE like :state" )
                                           .orderBy( "OBS_ORDER" )
                                           .bind( "state", state_in )
                                           .execute();

      rowcount = result.count();
      colcount = result.getColumnCount();

      if ( rowcount < 1 ) {
        message.str(""); message << "no active targets found with state = " << state_in;
        logwrite( function, message.str() );
        init_record();    // ensures that any previous record's info is not mistaken for this one
        return( TARGET_NOT_FOUND );
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] retrieved " << colcount << " columns from row";
      logwrite( function, message.str() );
#endif

      // fetch the current row
      //
      mysqlx::Row row = result.fetchOne();

      // all of the columns are needed, none can be null
      //
      for ( mysqlx::col_count_t cc = 0; cc < colcount; cc++ ) {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] " << cc << ": " << this->targetlist.at(cc) << " = " << row[cc];
        logwrite( function, message.str() );
#endif
        if ( row[cc].isNull() ) {
          message.str(""); message << this->targetlist.at(cc) << " cannot be empty!";
          logwrite( function, message.str() );
        }
      }

      // Connector/C++ does not support referring to row columns by their name yet (!)
      // so you have to get them by the order requested. The colnum() function returns
      // the correct column number.
      //
      col = this->colnum( "OBSERVATION_ID" ); this->obsid      = row.get( col );
      col = this->colnum( "OBS_ORDER" );      this->obsorder   = row.get( col );
      col = this->colnum( "NAME" );           this->name       = row.get( col );
      col = this->colnum( "RA" );             this->ra         = row.get( col );
      col = this->colnum( "DECL" );           this->dec        = row.get( col );
      col = this->colnum( "EPOCH" );          this->epoch      = row.get( col );
      col = this->colnum( "CASANGLE" );       this->casangle   = row.get( col );
      col = this->colnum( "SLITWIDTH" );      this->slitwidth  = row.get( col );
      col = this->colnum( "SLITOFFSET" );     this->slitoffset = row.get( col );
      col = this->colnum( "EXPTIME" );        this->exptime    = row.get( col );
      col = this->colnum( "NEXP" );           this->nexp       = row.get( col );
      col = this->colnum( "BINSPECT" );       this->binspect   = row.get( col );
      col = this->colnum( "BINSPAT" );        this->binspat    = row.get( col );
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "ERROR from mySQL ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << err;
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "ERROR std exception ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex.what();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }
    catch ( const char *ex ) {            /// catch everything else
      message.str(""); message << "ERROR other exception ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex;
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }

    message.str(""); message << "retrieved target " << this->name << " id " << this->obsid << " order " << this->obsorder;
    logwrite( function, message.str() );

    return TARGET_FOUND;
  }
  /** Sequencer::TargetInfo::get_next *****************************************/


  /** Sequencer::TargetInfo::update_state *************************************/
  /**
   * @fn         update_state
   * @brief      update the target state in the database DB_ACTIVE table
   * @param[in]  newstate, string to update the STATE column
   * @return     ERROR or NO_ERROR
   *
   */
  long TargetInfo::update_state( std::string newstate ) {
    std::string function = "Sequencer::TargetInfo::update_state";
    std::stringstream message;
    mysqlx::row_count_t rowcount=-1;

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      return( ERROR );
    }

    if ( this->obsid < 0 ) {
      logwrite( function, "ERROR: no current target selected!" );
      return( ERROR );
    }

    try {
      // create a session for accessing the database
      //
      mysqlx::Session mySession( mysqlx::SessionOption::HOST, this->db_host,
                                 mysqlx::SessionOption::PORT, this->db_port,
                                 mysqlx::SessionOption::USER, this->db_user,
                                 mysqlx::SessionOption::PWD,  this->db_pass   );

      // connect to database
      //
      mysqlx::Schema db( mySession, this->db_schema );

      // create a table object
      //
      mysqlx::Table targettable = db.getTable( this->db_active );

      // Find the row in the SQL active observations table
      // which matches the current observation ID.
      //
      mysqlx::RowResult result = targettable.select( this->targetlist )
                                           .where( "OBSERVATION_ID like :obsid" )
                                           .bind( "obsid", this->obsid )
                                           .execute();

      rowcount = result.count();  /// number or rows that match the select criteria

      // there should be only one row with this OBSERVATION_ID
      //
      if ( rowcount != 1 ) {
        message.str(""); message << "ERROR: expected 1 but " << rowcount
                                 << " rows matched search criteria OBSERVATION_ID=" << this->obsid;
        logwrite( function, message.str() );
        return( ERROR );
      }

      // update the state here
      //
      targettable.update( )
                 .set( "STATE", newstate )
                 .where( "OBSERVATION_ID like :obsid" )
                 .bind( "obsid", this->obsid )
                 .execute();

      message.str(""); message << "target " << this->name << " id " << this->obsid << " state " << newstate;
      logwrite( function, message.str() );
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "ERROR from mySQL: " << err;
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "ERROR std exception: " << ex.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( const char *ex ) {            /// catch everything else
      message.str(""); message << "ERROR other exception: " << ex;
      logwrite( function, message.str() );
      return( ERROR );
    }

    return NO_ERROR;
  }
  /** Sequencer::TargetInfo::update_state *************************************/


  /** Sequencer::TargetInfo::insert_completed *********************************/
  /**
   * @fn         insert_completed
   * @brief      insert current target record into completed observations table
   * @param[in]  none
   * @return     none
   *
   */
  long TargetInfo::insert_completed() {
    std::string function = "Sequencer::TargetInfo::insert_completed";
    std::stringstream message;

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      return( ERROR );
    }

    if ( this->name.empty() ) {
      logwrite( function, "ERROR: no record selected" );
      return( ERROR );
    }

    try {
      // create a session for accessing the database
      //
      mysqlx::Session mySession( mysqlx::SessionOption::HOST, this->db_host,
                                 mysqlx::SessionOption::PORT, this->db_port,
                                 mysqlx::SessionOption::USER, this->db_user,
                                 mysqlx::SessionOption::PWD,  this->db_pass   );

      // connect to database
      //
      mysqlx::Schema db( mySession, this->db_schema );

      // create a table object
      //
      mysqlx::Table targettable = db.getTable( this->db_completed );

      // add the row
      //
      targettable.insert( "OBSERVATION_ID",
                          "OBS_ORDER",
                          "STATE",
                          "NAME",
                          "RA",
                          "DECL",
                          "EPOCH",
                          "EXPTIME",
                          "NEXP",
                          "SLITWIDTH",
                          "SLITOFFSET",
                          "BINSPECT",
                          "BINSPAT",
                          "CASANGLE"
                        )
                .values( this->obsid,
                         this->obsorder,
                         Sequencer::TARGET_COMPLETE,
                         this->name,
                         this->ra,
                         this->dec,
                         this->epoch,
                         this->exptime,
                         this->nexp,
                         this->slitwidth,
                         this->slitoffset,
                         this->binspect,
                         this->binspat,
                         this->casangle )
                .execute();
    }
    catch ( const mysqlx::Error &err ) {
      message.str(""); message << "ERROR from mySQL: " << err;
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::exception &ex ) {
      message.str(""); message << "ERROR std exception: " << ex.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( const char *ex ) {
      message.str(""); message << "ERROR other exception: " << ex;
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /** Sequencer::TargetInfo::insert_completed *********************************/


  /** Sequencer::TargetInfo::get_table_names **********************************/
  /**
   * @fn         get_table_names
   * @brief      utility to print all of the table names from the database
   * @param[in]  none
   * @return     none
   *
   */
  long TargetInfo::get_table_names() {
    std::string function = "Sequencer::TargetInfo::get_table_names";
    std::stringstream message;
    std::list<std::string> mylist;

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      return( ERROR );
    }

    try {
      // create a session for accessing the database
      //
      mysqlx::Session mySession( mysqlx::SessionOption::HOST, this->db_host,
                                 mysqlx::SessionOption::PORT, this->db_port,
                                 mysqlx::SessionOption::USER, this->db_user,
                                 mysqlx::SessionOption::PWD,  this->db_pass   );

      // connect to database
      //
      mysqlx::Schema db( mySession, this->db_schema );

      mylist = db.getTableNames();
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "ERROR from mySQL: " << err;
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "ERROR std exception: " << ex.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( const char *ex ) {            /// catch everything else
      message.str(""); message << "ERROR other exception: " << ex;
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str("");
    for ( auto name : mylist ) message << name << " ";
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /** Sequencer::TargetInfo::get_table_names **********************************/


  /** Sequencer::Daemon::Daemon ***********************************************/
  /**
   * @fn         Daemon
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Daemon::Daemon() {
    this->name = "";
    this->socket.sethost( "localhost" );
    this->port = -1;    /// port comes from config file, in Sequencer::Server::configure_sequencer()
  }
  /** Sequencer::Daemon::Daemon ***********************************************/


  /** Sequencer::Daemon::~Daemon **********************************************/
  /**
   * @fn         ~Daemon
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Daemon::~Daemon( ) {
  }
  /** Sequencer::Daemon::~Daemon **********************************************/


  /** Sequencer::Daemon::configure ********************************************/
  /**
   * @fn         configure
   * @brief      configure the (private) variables for daemon communication
   * @param[in]  port of daemon (int)
   * @return     none
   *
  void Daemon::configure( int port ) {
    std::string function = "Sequencer::Daemon::configure";
    std::stringstream message;
    this->port = port;
    message << this->name << " configured with port " << this->port;
    logwrite( function, message.str() );
    return;
  }
   */
  /** Sequencer::Daemon::configure ********************************************/


  /** Sequencer::Daemon::send *************************************************/
  /**
   * @fn     send
   * @brief  
   * @param  
   * @return 
   *
   */
  long Daemon::send( std::string command, std::string &reply ) {
    std::string function = "Sequencer::Daemon::send["+this->name+"]";
    std::stringstream message;
    long ret;

//#ifdef LOGLEVEL_DEBUG
    if ( command.find( std::string( "poll" ) ) == std::string::npos ) {
      message.str(""); message << "sending \"" << command << "\" to " << this->name << " on fd " << this->socket.getfd();
      logwrite( function, message.str() );
    }
//#endif

    // send the command
    //
    command.append( "\n" );
    this->socket.Write( command );

    // Wait (poll) connected socket for incoming data...
    //
    int pollret;
    if ( ( pollret = this->socket.Poll() ) <= 0 ) {
      if ( pollret == 0 ) {
        message.str(""); message << "Poll timeout on fd " << this->socket.getfd();
        logwrite( function, message.str() );
      }
      if ( pollret <0 ) {
        message.str(""); message << "Poll error on fd " << this->socket.getfd() << ": " << strerror(errno);
        logwrite( function, message.str() );
      }
      return ERROR;
    }

    // read the response
    //
    char delim = '\n';
    if ( ( ret = this->socket.Read( reply, delim ) ) <= 0 ) {
      if ( ret < 0 && errno != EAGAIN ) {             // could be an actual read error
        message.str(""); message << "Read error on fd " << this->socket.getfd() << ": " << strerror(errno);
        logwrite(function, message.str());
      }
      if ( ret==0 ) {
        message.str(""); message << "timeout reading from socket on fd " << this->socket.getfd();
        logwrite( function, message.str() );
      }
    }

    // assign the response to the reply string, passed in by reference
    //
    reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
    reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );

    // If the reply contains "ERROR" then return ERROR, otherwise NO_ERROR.
    //
    if ( reply.find( std::string( "ERROR" ) ) != std::string::npos ) {
      return( ERROR );
    }
    else return( NO_ERROR );
  }
  /** Sequencer::Daemon::send *************************************************/


  /** Sequencer::Daemon::command **********************************************/
  /**
   * @fn     command
   * @brief  
   * @param[in]  args, string to send
   * @return 
   *
   */
  long Daemon::command( std::string args ) {
    std::string function = "Sequencer::Daemon::command["+this->name+"]";
    std::stringstream message;
    std::string reply;
    std::string done = "DONE";
    long retval = this->command( args, reply );
    if ( reply.find( std::string( "DONE" ) ) == std::string::npos ) {
      message.str(""); message << "sending " << args << " to " << this->name << " returned " << reply;
      logwrite( function, message.str() );
    }
#ifdef LOGLEVEL_DEBUG
    else {
      message.str(""); message << "[DEBUG] sending " << args << " to " << this->name << " returned " << reply;
      logwrite( function, message.str() );
    }
#endif
    return( retval );
  }
  long Daemon::command( std::string args, std::string &retstring ) {
    std::string function = "Sequencer::Daemon::command["+this->name+"]";
    std::stringstream message;
    long error = NO_ERROR;

    // this is probably a programming error if we're in here and port is undefined
    //
    if ( this->port < 0 ) {
      message.str(""); message << "daemon \"" << this->name << "\" not configured";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // connect to the daemon
    //
    if ( args == "connect" ) {
      // initialize socket connection
      //
      if ( ( error = this->connect() ) != NO_ERROR ) retstring="ERROR"; else retstring="DONE"; 
/***
      // send the open command
      //
      error = this->send( "open", retstring );

      if ( error == NO_ERROR ) {
        message.str(""); message << "opened " << this->name << "...";
      }
      else {
        message.str(""); message << "error opening " << this->name << "...";
      }
      logwrite( function, message.str() );
***/
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] connected to " << this->name << " on fd " << this->socket.getfd();
      logwrite( function, message.str() );
#endif
    }
    else

    if ( args == "isconnected" ) {
      error = this->is_connected( retstring );
    }
    else

    // disconnect from the daemon
    //
    if ( args == "disconnect" ) {
/***
      // send the close command
      //
      message.str(""); message << "closing " << this->name << "...";
      logwrite( function, message.str() );
      error = this->send( "close", retstring );
***/

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] disconnecting " << this->name << " from fd " << this->socket.getfd();
      logwrite( function, message.str() );
#endif
      // then close the connection
      //
      this->socket.Close();
      retstring="DONE";
    }

    // all other commands go straight on through, as-is
    //
    else {
      // but only if the connection is open of course
      //
      if ( !this->socket.isconnected() ) {
        message.str(""); message << "ERROR: connection not open to " << this->name;
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] sending to " << this->name << " on fd " << this->socket.getfd() << ": " << args;
        logwrite( function, message.str() );
#endif
        error = this->send( args, retstring );
      }
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] reply from " << this->name << " on fd " << this->socket.getfd() << ": " << retstring;
    logwrite( function, message.str() );
#endif

    return( error );
  }
  /** Sequencer::Daemon::command **********************************************/


  /** Sequencer::Daemon::connect **********************************************/
  /**
   * @fn         connect
   * @brief      initialize socket connection to the daemon
   * @param[in]  none  
   * @return     ERROR or NO_ERROR
   *
   * This function establishes a socket connection to the daemon
   * using the Network::TcpSocket class.
   *
   */
  long Daemon::connect( ) {
    std::string function = "Sequencer::Daemon::connect["+this->name+"]";
    std::stringstream message;
    long error = NO_ERROR;

    // probably a programming error if this Daemon object is not configured
    //
    if ( this->port == -1 ) {
      message.str(""); message << "ERROR: port not configured for " << this->name;
      logwrite( function, message.str() );
      error = ERROR;
    }
    else {

      // If already connected then close the connection
      //
      if ( this->socket.isconnected() ) {
        message.str(""); message << "closing existing connection to " << this->name;
        logwrite( function, message.str() );
        this->socket.Close();
      }

      this->socket.setport( this->port );

      // Create and connect to the socket
      //
      if ( this->socket.Connect() < 0 ) {
        message.str(""); message << "ERROR: connecting to " << this->name << " on port " << this->port;
        logwrite( function, message.str() );
        error = ERROR;
      } else {
        message.str(""); message << "connected to " << this->name << " on port " << this->port;
        logwrite( function, message.str() );
      }
    }

    return( error );
  }
  /** Sequencer::Daemon::connect **********************************************/


  /** Sequencer::Daemon::is_connected *****************************************/
  /**
   * @fn         connect
   * @brief      initialize socket connection to the daemon
   * @param[in]  none  
   * @return     ERROR or NO_ERROR
   *
   * This function establishes a socket connection to the daemon
   * using the Network::TcpSocket class.
   *
   */
  long Daemon::is_connected( std::string &reply ) {
    std::string function = "Sequencer::Daemon::is_connected["+this->name+"]";

    reply = ( this->socket.isconnected() ? "true" : "false" );

    logwrite( function, reply );

    return( NO_ERROR );
  }
  /** Sequencer::Daemon::is_connected *****************************************/

}
