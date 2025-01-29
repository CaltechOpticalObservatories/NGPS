/**
 * @file     sequencer_interface.cpp
 * @brief    these are the functions for the classes that the sequencer uses to interface to other subsystems
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#include "sequencer_interface.h"

namespace Sequencer {


  /***** Sequencer::TargetInfo::colnum ****************************************/
  /**
   * @brief      get column number of requested field from this->targetlist_cols
   * @details    This can throw a std::runtime_error exception
   * @param[in]  field  string to search for
   * @return     col_count_t index
   *
   * This function is used to return the location of the field name in the targetlist_cols
   * vector. This is needed because the mysqlx Connector/C++ X DEV API does not have
   * its own capability for this. If the field is not found then throw an exception.
   *
   */
  mysqlx::col_count_t TargetInfo::colnum( std::string field, std::vector<std::string> vec ) {
    auto it = std::find( vec.begin(), vec.end(), field );
    if ( it != vec.end() ) {
      return ( it - vec.begin() );
    }
    else {
      std::string function = "Sequencer::TargetInfo::colnum";
      std::stringstream message;
      message.str(""); message << "ERROR: requested field " << field << " not found";
      logwrite( function, message.str() );
      throw std::runtime_error(message.str());
    }
  }
  /***** Sequencer::TargetInfo::colnum ****************************************/


  /***** Sequencer::TargetInfo::init_record ***********************************/
  /**
   * @brief      initialize current target record variables
   * @details    This is called by the TargetInfo class constructor, and
   *
   */
  void TargetInfo::init_record() {
    std::string function = "Sequencer::TargetInfo::init_record";
    std::stringstream message;
    this->setid=-1;
    this->setname="(UNDEFINED)";
    this->obsid=-1;
    this->obsorder=-1;
    this->name.clear();
    this->ra_hms.clear();
    this->dec_dms.clear();
    this->slitangle=NAN;
    this->casangle=NAN;
    this->pointmode.clear();
    this->slitwidth=NAN;
    this->slitwidth_req=NAN;
    this->slitoffset=NAN;
    this->slitoffset_req=NAN;
    this->exptime_act=-1;
    this->exptime_req=-1;
    this->targetnum=-1;
    this->sequencenum=-1;
    this->obsmode.clear();
    this->binspect=-1;
    this->binspat=-1;
    this->nexp=1;
    this->airmasslimit=99.;
    this->acquired=false;
    this->notbefore="1901-01-01 00:00:00.000";
    this->slewstart="1901-01-01 00:00:00.000";
    this->slewend="1901-01-01 00:00:00.000";
    this->expstart="1901-01-01 00:00:00.000";
    this->expend="1901-01-01 00:00:00.000";

  }
  /***** Sequencer::TargetInfo::init_record ***********************************/


  /***** Sequencer::TargetInfo::configure_db **********************************/
  /**
   * @brief      configure database connection parameters
   * @param[in]  param  name of parameter
   * @param[in]  value  value of parameter
   * @return     ERROR or NO_ERROR
   *
   */
  long TargetInfo::configure_db( std::string param, std::string value ) {
    std::string function = "Sequencer::TargetInfo::configure_db";
    std::stringstream message;
    long error = NO_ERROR;

    if ( this->db_configured ) {
      logwrite( function, "database already configured" );
      return NO_ERROR;
    }

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
    if ( param == Sequencer::DB_SETS ) {
      this->db_sets = value;
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
         !this->db_sets.empty()       &&
         this->db_port  != -1         ) {
      this->dbManager = std::make_unique<DatabaseManager>(this->db_host, this->db_port,
                                                          this->db_user, this->db_pass,
                                                          this->db_schema, this->db_active);
      this->db_configured = true;
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] host=" << this->db_host << " port=" << this->db_port
                               << " user=" << this->db_user << " pass=" << this->db_pass
                               << " schema=" << this->db_schema << " active table=" << this->db_active
                               << " completed table=" << this->db_completed << " target sets table=" << this->db_sets;
      logwrite( function, message.str() );
#endif
    }
    else { this->db_configured = false; }

    return( error );
  }
  /***** Sequencer::TargetInfo::configure_db **********************************/


  /***** Sequencer::TargetInfo::targetset *************************************/
  /**
   * @brief      set or get the target set to read from the targets table
   * @param[in]  args       string can contain either a set ID or a set name
   * @param[out] retstring  reference to a return string to carry the current ID and NAME
   *
   * If args is empty then return the current set ID and NAME. The set ID is stored in the
   * class object this->setid. Even in the case of an empty args to read the current set,
   * a database query of set_id is still made, to ensure that the set (still) exists.
   *
   */
  long TargetInfo::targetset( std::string args, std::string &retstring ) {
    std::string function = "Sequencer::TargetInfo::targetset";
    std::stringstream message;
    std::stringstream retstream;      // used for building the return string
    int setid_in=-1;                  // holds the set ID from args (if args contains a number)
    std::string setname_in="";        // holds the set name from args (if args contains a string)
    mysqlx::row_count_t rowcount=-1;  // number of rows that match the select criteria
    mysqlx::col_count_t col=-1;       // column number returned from this->colnum( FIELD )
    mysqlx::col_count_t colcount=-1;  // the number of columns in the currently read row
    bool use_id = false;              // search the DB by set ID (true) or by set name (false)

    // Initialize the return string now.
    // In case there are any errors and the function returns early, it will return the current settings.
    //
    retstream.str(""); retstream << this->setid << " " << this->setname;
    retstring = retstream.str();

    if ( args.empty() && this->setid < 0 ) {
      logwrite( function, "ERROR: set ID has not been set and no ID or name provided" );
      return( ERROR );
    }

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      return( ERROR );
    }

    try {
      // If no args were provided then this is a read-only request,
      // so use the class object's setid value.
      //
      if ( args.empty() ) {
        setid_in = this->setid;
        use_id = true;
      }

      // otherwise either a set ID or a set name was provided
      //
      else {
        use_id     = ( args.find_first_not_of("0123456789") == std::string::npos );  // set use_id true if only a number was provided
        setid_in   = ( use_id ? std::stoi( args ) : -1 );                            // get the provided setid, or
        setname_in = ( use_id ? "" : args );                                         // get the provided setname
      }

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
      mysqlx::Table targetsets = db.getTable( this->db_sets );

      // Find a row in the SQL table where either SET_ID is setid_in or SET_NAME is setname_in,
      // depending on if a number or a string was provided above.
      //
      mysqlx::RowResult result;
      if ( use_id ) {
        result = targetsets.select( this->targetset_cols )
                           .where( "SET_ID like :setid" )
                           .bind( "setid", setid_in )
                           .execute();
      }
      else {
        result = targetsets.select( this->targetset_cols )
                           .where( "SET_NAME like :setname" )
                           .bind( "setname", setname_in )
                           .execute();
      }

      rowcount = result.count();
      colcount = result.getColumnCount();

      // Nothing found
      //
      if ( rowcount < 1 ) {
        message.str(""); message << "ERROR no target sets found with requested ";
        if ( use_id ) message << "ID = " << setid_in; else message << "name = " << setname_in;
        logwrite( function, message.str() );
        return( ERROR );
      }

      // fetch the current row
      //
      mysqlx::Row row = result.fetchOne();

      // Connector/C++ does not support referring to row columns by their name yet (!)
      // so you have to get them by the order requested. The colnum() function returns
      // the correct column number.
      //
      col = this->colnum( "SET_ID", this->targetset_cols );     this->setid       = row.get( col );
      col = this->colnum( "SET_NAME", this->targetset_cols );   this->setname     = row.get( col );

      // Reset the return string now that the set ID and name have been read from the database
      // and set in the class object.
      //
      retstream.str(""); retstream << this->setid << " " << this->setname;
      retstring = retstream.str();
    }
    catch ( std::invalid_argument & ) {
      message.str(""); message << "ERROR invalid argument: args=" << args << " col=" << col;
      logwrite(function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range & ) {
      message.str(""); message << "ERROR out of range: args=" << args << " col=" << col;
      logwrite(function, message.str() );
      return( ERROR );
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "ERROR from mySQL ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetset_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << err;
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( ERROR );
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "ERROR std exception ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetset_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex.what();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( ERROR );
    }
    catch ( const char *ex ) {            /// catch everything else
      message.str(""); message << "ERROR other exception ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetset_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex;
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( ERROR );
    }

    message.str(""); message << "current target set " << retstream.str();
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Sequencer::TargetInfo::targetset *************************************/


  /***** Sequencer::TargetInfo::add_row ***************************************/
  /**
   * @brief      adds a row to the database (non-production)
   * @param[in]  number_in     used for both OBSERVATION_ID and OBS_ORDER
   * @param[in]  name_in       target name
   * @param[in]  ra_hms_in     RA in HH:MM:SS
   * @param[in]  dec_dms_in    DECL in DD:MM:SS
   * @param[in]  slita_in      slit angle
   * @param[in]  slitw_in      slit width
   * @param[in]  exptime_in    exposure time
   * @param[in]  pointmode_in  point mode
   *
   * This is for testing purposes. Adds a row to the database using the passed-in
   * parameters which set the ID, ORDER, NAME, RA, DECL. Everything else is fixed.
   *
   */
  long TargetInfo::add_row( int number_in, std::string name_in, std::string ra_hms_in, std::string dec_dms_in,
                            double slita_in, double slitw_in, double exptime_in, std::string pointmode_in ) {
    std::string function = "Sequencer::TargetInfo::add_row";
    std::stringstream message;

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR: database not configured (check .cfg file)" );
      return( ERROR );
    }

    if ( this->setid < 0 ) {
      logwrite( function, "ERROR: targetset has not been provided" );
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
                          "OBS_ORDER",
                          "SET_ID",
                          "STATE",
                          "NAME",
                          "RA",
                          "DECL",
                          "EPOCH",
                          "EXPTIME",
                          "OTMexpt",
                          "TARGET_NUMBER",
                          "SEQUENCE_NUMBER",
                          "CASANGLE",
                          "OTMcass",
                          "OTMslitangle",
                          "OTMslitwidth",
                          "SLITWIDTH",
                          "SLITOFFSET",
                          "BINSPECT",
                          "BINSPAT",
                          "POINTMODE"
                        )
                .values( number_in,                  // OBSERVATION_ID
                         number_in,                  // OBS_ORDER
                         this->setid,                // SET_ID
                         Sequencer::TARGET_PENDING,  // STATE
                         name_in,                    // NAME
                         ra_hms_in,                  // RA
                         dec_dms_in,                 // DECL
                         "J2000",                    // EPOCH
                         exptime_in,                 // EXPTIME
                         exptime_in,                 // OTMexpt
                         1,                          // TARGET_NUMBER
                         1,                          // SEQUENCE_NUMBER
                         0.,                         // CASANGLE
                         0.,                         // OTMcass
                         slita_in,                   // OTMslitangle
                         slitw_in,                   // OTMslitwidth
                         slitw_in,                   // SLITWIDTH
                         1,                          // SLITOFFSET
                         1,                          // BINSPECT
                         1,                          // BINSPAT
                         pointmode_in                // POINTMODE
                       )
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
  /***** Sequencer::TargetInfo::add_row ***************************************/


  TargetInfo::TargetState TargetInfo::get_specified_target( std::string args, std::string &retstring ) {
    const std::string function="Sequencer::TargetInfo::get_specified_target";
    std::stringstream message;

    if ( !is_db_configured() ) {
      message.str(""); message << "ERROR database not configured (check .cfg file)";
      retstring = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return TARGET_ERROR;
    }

    try {
      int _obsid = std::stoi(args);

      std::string condition = "OBSERVATION_ID like :obsid";
      std::string order = "OBSERVATION_ID";
      std::map<std::string, std::string> bindings = { {"obsid", args} };

      mysqlx::RowResult result = dbManager->do_query(condition, order, bindings, this->targetlist_cols);

      mysqlx::row_count_t rowcount = result.count();
      mysqlx::col_count_t colcount = result.getColumnCount();

      message.str(""); message << "rowcount=" << rowcount << " colcount=" << colcount;
      logwrite( function, message.str() );

      if ( result.count() < 1 ) {
        message.str(""); message << "no matching target found for obsid " << _obsid;
        retstring = message.str();
        logwrite( function, message.str() );
        init_record();    // ensures that any previous record's info is not mistaken for this one
        return TARGET_NOT_FOUND;
      }

      // Fetch the current row,
      //
      mysqlx::Row row = result.fetchOne();

      // which should not be empty.
      //
      if ( ! row ) {
        logwrite( function, "ERROR no row read from database" );
        return TARGET_ERROR;
      }
      else logwrite( function, "read target row from database" );

      if ( this->parse_target_from_row( row ) != NO_ERROR ) return TARGET_ERROR;
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "ERROR mySQL exception: " << err;
      retstring = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return TARGET_ERROR;
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "ERROR exception: " << ex.what();
      retstring = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return TARGET_ERROR;
    }

    logwrite( function, "loaded target "+std::string(this->name) );

    return TARGET_FOUND;
  }


  /***** Sequencer::TargetInfo::get_next **************************************/
  /**
   * @brief      get next target from DB whose state is Sequencer::TARGET_PENDING
   * @return     ERROR, NO_ERROR, TARGET_FOUND, TARGET_NOT_FOUND
   *
   * This function is overloaded.
   *
   * This version has no parameters so it does not return the status message
   * but only returns the TargetState.
   *
   */
  TargetInfo::TargetState TargetInfo::get_next() {
    std::string dontcare;
    return( this->get_next( Sequencer::TARGET_PENDING, dontcare ) );
  }
  /***** Sequencer::TargetInfo::get_next **************************************/


  /***** Sequencer::TargetInfo::get_next **************************************/
  /**
   * @brief      get next target from DB whose state is Sequencer::TARGET_PENDING
   * @param[out] status    reference to string to return status message
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
  TargetInfo::TargetState TargetInfo::get_next( std::string &status ) {
    return( this->get_next( Sequencer::TARGET_PENDING, status ) );
  }
  /***** Sequencer::TargetInfo::get_next **************************************/


  /***** Sequencer::TargetInfo::get_next **************************************/
  /**
   * @brief      get next target from DB whose state is state_in
   * @param[in]  state_in  the state to search for
   * @param[out] status    reference to string to return status message
   * @return     ERROR, NO_ERROR, TARGET_FOUND, TARGET_NOT_FOUND
   *
   * This function is overloaded.
   *
   * This version accepts (and requires) a state to look for.
   *
   */
  TargetInfo::TargetState TargetInfo::get_next( std::string state_in, std::string &status ) {
    std::string function = "Sequencer::TargetInfo::get_next";
    std::stringstream message;
    long error = NO_ERROR;
    mysqlx::row_count_t rowcount=-1;  /// number of rows that match the select criteria
    mysqlx::col_count_t col=-1;       /// column number returned from this->colnum( FIELD )
    mysqlx::col_count_t colcount=-1;  /// the number of columns in the currently read row

    if ( !is_db_configured() ) {
      message.str(""); message << "ERROR database not configured (check .cfg file)";
      status = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }

    if ( this->setid < 0 ) {
      message.str(""); message << "ERROR invalid target set ID " << this->setid << " " << this->setname;
      status = message.str();
      logwrite( function, message.str() );
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
      mysqlx::RowResult result = targettable.select( this->targetlist_cols )
                                           .where( "SET_ID like :setid && STATE like :state" )
                                           .orderBy( "OBS_ORDER" )
                                           .bind( "state", state_in )
                                           .bind( "setid", this->setid )
                                           .execute();

      rowcount = result.count();
      colcount = result.getColumnCount();

      if ( rowcount < 1 ) {
        message.str(""); message << "no targets found in set " << this->setid << " " << this->setname << " with state \"" << state_in << "\"";
        status = message.str();
        logwrite( function, message.str() );
        init_record();    // ensures that any previous record's info is not mistaken for this one
        return( TARGET_NOT_FOUND );
      }

#ifdef LOGLEVEL_DEBUG
      logwrite( function, "[DEBUG] retrieved "+std::to_string(colcount)+" columns" );
#endif

      // Fetch the current row,
      //
      mysqlx::Row row = result.fetchOne();

      // which should not be empty.
      //
      if ( ! row ) {
        logwrite( function, "ERROR no row read from database" );
        return TARGET_ERROR;
      }

      error = this->parse_target_from_row( row );
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "ERROR mySQL exception ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << err;
      status = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return TARGET_ERROR;
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "ERROR exception ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex.what();
      status = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return TARGET_ERROR;
    }

    message.str(""); message << "retrieved target id " << this->obsid << " " << this->name << " " << this->ra_hms << " " << this->dec_dms
                             << " from set " << this->setid << " " << this->setname;
    status = message.str();
    logwrite( function, message.str() );

    // If we got to here then target is found, but do one last quality-control check before returning success
    //
    error = target_qc_check( status );

    return ( error == NO_ERROR ? TARGET_FOUND : TARGET_ERROR );
  }
  /***** Sequencer::TargetInfo::get_next **************************************/


  /***** Sequencer::TargetInfo::parse_target_from_row *************************/
  /**
   * @brief      
   * @param[in]  
   * @param[out] 
   * @return     
   *
   */
  long TargetInfo::parse_target_from_row( const mysqlx::Row &row ) {
    try {
      // To get the value of a field, call extract_column_from_row<T>( FIELD, row, [default] )
      // Call it with the explicit template instantiation type <T> to match the data type of
      // the value, pass it the name of the field and the mysqlx::Row.
      //
      // The fields here must be defined in the TargetInfo::targetlist_cols vector.
      //
      // If a third (optional) argument is specified then that value will be used if the
      // field is empty, otherwise the field is required and will throw an exception if
      // empty.
      //

      // If the name starts with "CAL_" then it is a calibration target. This
      // flag will tell the sequencer how to select the calibration settings,
      // which are stored in the config file indexed by name.
      //
      this->name           = extract_column_from_row<mysqlx::string>( "NAME", row );

      std::string _name(this->name);
      this->iscal = ( _name.compare(0, 4, "CAL_")==0 );

      this->state          = extract_column_from_row<mysqlx::string>( "STATE", row );
      this->owner          = extract_column_from_row<mysqlx::string>( "OWNER", row );
      this->obsid          = extract_column_from_row<int>( "OBSERVATION_ID", row );
      this->obsorder       = extract_column_from_row<int>( "OBS_ORDER", row );
      this->targetnum      = extract_column_from_row<long>( "TARGET_NUMBER", row );
      this->sequencenum    = extract_column_from_row<long>( "SEQUENCE_NUMBER", row );
      this->ra_hms         = extract_column_from_row<mysqlx::string>( "RA", row );
      this->dec_dms        = extract_column_from_row<mysqlx::string>( "DECL", row );
      this->offset_ra      = extract_column_from_row<double>( "OFFSET_RA", row );
      this->offset_dec     = extract_column_from_row<double>( "OFFSET_DEC", row );

      this->casangle       = extract_column_from_row<double>( "OTMcass", row );
      this->slitangle      = extract_column_from_row<double>( "OTMslitangle", row );
      this->slitwidth_req  = extract_column_from_row<double>( "OTMslitwidth", row );
      this->slitoffset_req = extract_column_from_row<double>( "SLITOFFSET", row );

      // The database stores a field "EXPTIME_REQ" which contains the command to the OTM,
      // then the field "OTMexpt" is the calculated exposure time from the OTM, which I
      // define here as my exptime_req because that is the exposure time the sequencer is
      // going to request of the cameras. After the exposure the sequencer will ask the
      // camera for the actual exposure time, which I call exptime_act.
      //
      this->exptime_req    = extract_column_from_row<double>( "OTMexpt", row );

      this->nexp           = extract_column_from_row<int>( "NEXP", row );

      this->binspect       = extract_column_from_row<int>( "BINSPECT", row );
      this->binspat        = extract_column_from_row<int>( "BINSPAT", row );

      this->pointmode      = extract_column_from_row<mysqlx::string>( "POINTMODE", row );
      this->note           = extract_column_from_row<mysqlx::string>( "NOTE", row );
      this->otmflag        = extract_column_from_row<mysqlx::string>( "OTMFLAG", row );
      this->notbefore      = extract_column_from_row<mysqlx::string>( "NOTBEFORE", row, "1901-01-01 00:00:00.000" );

//    this->airmasslimit = extract_column_from_row<double>( "AIRMASS_MAX", row, 99.0 );
    }
    catch ( const std::runtime_error &e ) {
      logwrite( "Sequencer::TargetInfo::parse_target_from_row", "ERROR: "+std::string(e.what()) );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::TargetInfo::parse_target_from_row *************************/


  /***** Sequencer::TargetInfo::target_qc_check *******************************/
  /**
   * @brief      target info quality control check
   * @details    applies a limited set of formatting rules and range checks on certain values
   * @param[out] status    reference to string to return status message
   * @return     ERROR or NO_ERROR
   *
   */
  long TargetInfo::target_qc_check( std::string &status ) {
    std::string function = "Sequencer::TargetInfo::target_qc_check";
    std::stringstream message;

    // You can have both RA and DEC empty (which means don't point the telescope)
    // but you can't have only one of them empty (which is considered undefined).
    //
    if ( (   this->ra_hms.empty() && ! this->dec_dms.empty() ) ||
         ( ! this->ra_hms.empty() &&   this->dec_dms.empty() ) ) {
      message.str(""); message << "ERROR cannot have only RA or only DEC empty. both must be empty or filled";
      status = message.str();
      logwrite( function, message.str() );
      return ERROR;
    }

    // Cannot have negative RA
    //
    if ( ! this->ra_hms.empty() ) {
      double _rah = radec_to_decimal( this->ra_hms );  // convert RA from HH:MM:SS.s to decimal hours
      if ( _rah < 0 ) {
        message.str(""); message << "ERROR cannot have negative RA " << this->ra_hms;
        status = message.str();
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    // Check Declination in range {-90:+90}
    //
    if ( ! this->dec_dms.empty() ) {
      double _dec = radec_to_decimal( this->dec_dms );  // convert DEC from DD:MM:SS.s to decimal degrees
      if ( _dec < -90.0 || _dec > 90.0 ) {
        message.str(""); message << "ERROR declination " << this->dec_dms << " outside range {-90:+90}";
        status = message.str();
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    // If pointmode not provided (this is OK) then the default is SLIT
    //
    if ( this->pointmode.empty() ) {
      this->pointmode = Acam::POINTMODE_SLIT;
    }
    else {
      if ( ! caseCompareString( this->pointmode, Acam::POINTMODE_ACAM ) &&
           ! caseCompareString( this->pointmode, Acam::POINTMODE_SLIT ) ) {
        message.str(""); message << "ERROR invalid pointmode \"" << this->pointmode << "\": must be { <empty> "
                                 << Acam::POINTMODE_ACAM << " " << Acam::POINTMODE_SLIT << " }";
        status = message.str();
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    return NO_ERROR;
  }
  /***** Sequencer::TargetInfo::target_qc_check *******************************/


  /***** Sequencer::TargetInfo::update_state **********************************/
  /**
   * @brief      update the target state in the database DB_ACTIVE table
   * @param[in]  newstate  string to update the STATE column
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
      // which matches the current observation ID. The targetlist_cols
      // vector identifies the fields to retrieve and is initialized
      // by the TargetInfo() class initializer list.
      //
      mysqlx::RowResult result = targettable.select( this->targetlist_cols )
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

      this->state = newstate;  // on success, save the new state to the class so that it can be accessed by enqueue
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
  /***** Sequencer::TargetInfo::update_state **********************************/


  /***** Sequencer::TargetInfo::insert_completed ******************************/
  /**
   * @brief      insert current target record into completed observations table
   *
   */
  long TargetInfo::insert_completed() {
    std::string function = "Sequencer::TargetInfo::insert_completed";
    std::stringstream message;

    if ( !is_db_configured() ) {
      logwrite( function, "ERROR database not configured (check .cfg file)" );
      return ERROR;
    }

    if ( this->name.empty() ) {
      logwrite( function, "ERROR no record selected" );
      return ERROR;
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

      // create a vector of column names
      //
      std::vector<std::string> columns;
      columns = { "OWNER",            // target table
                  "OBSERVATION_ID",   // target table
                  "SET_ID",           // target table
                  "TARGET_NUMBER",    // target table
                  "SEQUENCE_NUMBER",  // target table
                  "NAME",             // target table
                  "FITSFILE",         // internal from ___
                  "RA",               // target table
                  "DECL",             // target table
                  "SLITANGLE_REQ",    // target table col = SLITANGLE
                  "POINTMODE",        // target table
                  "NOTBEFORE",        // target table
                  "SLEW_START",       // from Sequence::dothread_move_to_target()
                  "SLEW_END",         // from Sequence::dothread_move_to_target()
                  "EXPTIME_REQ",      // target table col = OTMexpt
                  "EXP_START",        // from Sequence::dothread_trigger_exposure()
                  "EXP_END",          // from Sequence::dothread_sequencer_async_listener() ?
                  "SLITWIDTH_REQ",    // target table col = SLITWIDTH
                  "BINSPECT",         // target table
                  "BINSPAT",          // target table
                  "OBSMODE",          // target table
                  "NOTE",             // target table
                  "OTMFLAG"           // target table
                };

      // and a vector of corresponding values for those columns
      //
      std::vector<mysqlx::Value> values;
      values = { this->owner,         // OWNER
                 this->obsid,         // OBSERVATION_ID
                 this->setid,         // SET_ID
                 this->targetnum,     // TARGET_NUMBER
                 this->sequencenum,   // SEQUENCER_NUMBER
                 this->name,          // NAME
                 this->fitsfile,      // FITSFILE
                 this->ra_hms,        // RA
                 this->dec_dms,       // DECL
                 this->slitangle_req, // SLITANGLE_REQ
                 this->pointmode,     // POINTMODE
                 this->notbefore,     // NOTBEFORE
                 this->slewstart,     // SLEW_START
                 this->slewend,       // SLEW_END
                 this->exptime_req,   // EXPTIME_REQ <-- came in as OTMexpt
                 this->expstart,      // EXP_START
                 this->expend,        // EXP_END
                 this->slitwidth_req, // SLITWIDTH_REQ
                 this->binspect,      // BINSPECT
                 this->binspat,       // BINSPAT
                 this->obsmode,       // OBSMODE
                 this->note,          // NOTE
                 this->otmflag        // OTMFLAG
               };

      // The entries in the above "columns" and "values" vectors are fixed.
      // Now add additional columns/values to each which come from external
      // telemetry sources, but only add them if they have a valid value.
      //
      for ( const auto &[name, data] : this->external_telemetry ) {
        if ( data.valid ) {
          columns.push_back( name );
          values.push_back( data.value );
        }
      }

      // add the row --
      //
      targettable.insert( columns).values( values ).execute();
    }
    catch ( const mysqlx::Error &err ) {
      message.str(""); message << "ERROR mySQL exception: " << err;
      logwrite( function, message.str() );
      return ERROR;
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR std exception: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    message.str(""); message << "target " << this->name << " inserted into completed table";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Sequencer::TargetInfo::insert_completed ******************************/


  /***** Sequencer::TargetInfo::get_table_names *******************************/
  /**
   * @brief      utility to print all of the table names from the database
   *
   */
  long TargetInfo::get_table_names() {
    std::string function = "Sequencer::TargetInfo::get_table_names";
    std::stringstream message;
    std::list<std::string> tablenames;

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

      tablenames  = db.getTableNames();
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

    message.str(""); message << "TableNames: ";
    for ( const auto &name : tablenames ) message << name << " ";
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** Sequencer::TargetInfo::get_table_names *******************************/


  /***** Sequencer::PowerSwitch::configure ************************************/
  /**
   * @brief      parses POWER_* entry from configuration file
   * @param[in]  arglist
   * @return     ERROR or NO_ERROR
   *
   */
  long PowerSwitch::configure( std::string arglist ) {
    std::vector<std::string> tokens;

    int size = Tokenize( arglist, tokens, " \t" );

    for ( const auto &tok : tokens ) {
      this->plugname.push_back( tok );
    }

    return( size < 1 ? ERROR : NO_ERROR );
  }
  /***** Sequencer::PowerSwitch::configure ************************************/


  /***** Sequencer::CalibrationTarget::configure ******************************/
  /**
   * @brief      parses CAL_TARGET entry from configuration file
   * @param[in]  args  args for CAL_TARGET key
   * @return     ERROR | NO_ERROR
   *
   */
  long CalibrationTarget::configure( const std::string &args ) {
    const std::string function("Sequencer::CalibrationTarget::configure");
    std::stringstream message;
    std::vector<std::string> tokens;

    auto size = Tokenize( args, tokens, " \t" );

    // there must be 15 args. see cfg file for complete description
    if ( size != 15 ) {
      message << "ERROR expected 15 but received " << size << " parameters";
      logwrite( function, message.str() );
      return ERROR;
    }

    // token[0] = name
    // name can't be empty, must be "SCIENCE" or start with "CAL_"
    std::string name(tokens[0]);
    if ( name.empty() || ( name != "SCIENCE" &&
                           name.compare(0, 4, "CAL_") !=0 ) ) {
      message << "ERROR invalid calibration target name \"" << name << "\": must be \"SCIENCE\" or start with \"CAL_\" ";
      return ERROR;
    }
    this->calmap[name].name = name;

    // token[1] = caldoor
    if ( tokens[1].empty() ||
         ( tokens[1].find("open")==std::string::npos && tokens[1].find("close") ) ) {
      message << "ERROR invalid caldoor \"" << tokens[1] << "\": expected {open|close}";
      return ERROR;
    }
    this->calmap[name].caldoor = (tokens.at(1).find("open")==0);

    // token[2] = calcover
    if ( tokens[2].empty() ||
         ( tokens[2].find("open")==std::string::npos && tokens[2].find("close") ) ) {
      message << "ERROR invalid calcover \"" << tokens[2] << "\": expected {open|close}";
      return ERROR;
    }
    this->calmap[name].calcover = (tokens.at(2).find("open")==0);

    // tokens[3:6] = LAMPTHAR, LAMPFEAR, LAMPBLUC, LAMPREDC
    int n=3;  // incremental token counter used for the following groups
    for ( const auto &lamp : this->lampnames ) {
      if ( tokens[n].empty() ||
           ( tokens[n].find("on")==std::string::npos && tokens[n].find("off")==std::string::npos ) ) {
        message << "ERROR invalid state \"" << tokens[n] << "\" for " << lamp << ": expected {on|off}";
        logwrite( function, message.str() );
        return ERROR;
      }
      this->calmap[name].lamp[lamp] = (tokens[n].find("on")==0);
      n++;
    }

    // tokens[7:8] = domelamps
    // i indexes domelampnames vector {0,1}
    // j indexes domelamp map {1,2}
    for ( int i=0,j=1; j<=2; i++,j++ ) {
      if ( tokens[n].empty() ||
           ( tokens[n].find("on")==std::string::npos && tokens[n].find("off")==std::string::npos ) ) {
        message << "ERROR invalid state \"" << tokens[n] << "\" for " << domelampnames[i] << ": expected {on|off}";
        logwrite( function, message.str() );
        return ERROR;
      }
      this->calmap[name].domelamp[j] = (tokens[n].find("on")==0);
      n++;
    }

    // tokens[0:14] = lampmod{1:6}
    for ( int i=1; i<=6; i++ ) {
      if ( tokens[n].empty() ||
           ( tokens[n].find("on")==std::string::npos && tokens[n].find("off")==std::string::npos ) ) {
        message << "ERROR invalid state \"" << tokens[n] << "\" for lampmod" << n << ": expected {on|off}";
        logwrite( function, message.str() );
        return ERROR;
      }
      this->calmap[name].lampmod[i] = (tokens[n].find("on")==0);
      n++;
    }

    return NO_ERROR;
  }
  /***** Sequencer::CalibrationTarget::configure ******************************/

}
