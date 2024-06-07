/**
 * @file     sequencer_interface.cpp
 * @brief    these are the functions for the classes that the sequencer uses to interface to other subsystems
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#include "sequencer_interface.h"

namespace Sequencer {

  /***** Sequencer::TargetInfo::TargetInfo ************************************/
  /**
   * @brief  class constructor
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
    this->db_sets       = "";
    this->db_configured = false;

    this->init_record();

    // These are the fields (or columns) to be accessed in the DB_ACTIVE table.
    // They can be listed in any order.
    //
    this->targetlist_cols = { "OBSERVATION_ID",
                              "OBS_ORDER",
                              "STATE",
                              "NAME",
                              "RA",
                              "DECL",
                              "OTMexpt",
                              "TARGET_NUMBER",
                              "SEQUENCE_NUMBER",
                              "OTMslitwidth",
                              "OTMslitangle",
                              "SLITOFFSET",
                              "BINSPECT",
                              "BINSPAT",
                              "OTMcass"
                            };           /// initialize the target list fields for accessing the active target table

    // These are the fields (or columns) to be accessed in the DB_SETS table.
    //
    this->targetset_cols  = { "SET_ID",
                              "SET_NAME"
                            };           /// initialize the target set fields for accessing the table of target sets

    // These wiil be read from the .cfg file
    //
    this->offset_threshold = 0;
    this->max_tcs_offset = 0;
  }
  /***** Sequencer::TargetInfo::TargetInfo ************************************/


  /***** Sequencer::TargetInfo::colnum ****************************************/
  /**
   * @brief      get column number of requested field from this->targetlist_cols
   * @param[in]  field  string to search for
   * @return     integer, -1 on error
   *
   * This function is used to return the location of the field name in the targetlist_cols
   * vector. This is needed because the mysqlx Connector/C++ X DEV API does not have
   * its own capability for this. If the field is not found then return -1. This should
   * cause a std::exception row column to be thrown if you try to access a row[ -1 ].
   *
   */
  int TargetInfo::colnum( std::string field, std::vector<std::string> vec ) {
    std::string function = "Sequencer::TargetInfo::colnum";
    std::stringstream message;
    std::vector<std::string>::iterator it;
    it = std::find( vec.begin(), vec.end(), field );
    if ( it != vec.end() ) {
      return ( it - vec.begin() );
    }
    else {
      message.str(""); message << "ERROR: requested field " << field << " not found";
      logwrite( function, message.str() );
      return -1;
    }
  }
  /***** Sequencer::TargetInfo::colnum ****************************************/


  /***** Sequencer::TargetInfo::init_record ***********************************/
  /**
   * @brief      initialize current target record variables
   *
   */
  void TargetInfo::init_record() {
    std::string function = "Sequencer::TargetInfo::init_record";
    std::stringstream message;
    this->setid=-1;
    this->setname="(UNDEFINED)";
    this->obsid=-1;
    this->obsorder=-1;
    this->name="";
    this->ra_hms="";
    this->dec_dms="";
    this->slitangle=-1;
    this->casangle=-1;
    this->slitwidth=-1;
    this->slitoffset=-1;
    this->exptime=-1;
    this->targetnum=-1;
    this->sequencenum=-1;
    this->obsplan="";
    this->binspect=-1;
    this->binspat=-1;
    this->acquired=false;

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
   * @param[in]  number  int used for both OBSERVATION_ID and OBS_ORDER
   * @param[in]  name    string target name
   * @param[in]  ra      string RA
   * @param[in]  dec     string DECL
   * @param[in]  slita   double slit angle
   * @param[in]  slitw   double slit width
   * @param[in]  etime   double exposure time
   *
   * This is for testing purposes. Adds a row to the database using the passed-in
   * parameters which set the ID, ORDER, NAME, RA, DECL. Everything else is fixed.
   *
   */
  long TargetInfo::add_row( int number, std::string name, std::string ra, std::string dec,
                            double slita, double slitw, double etime ) {
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
                          "BINSPAT"
                        )
                .values( number,                     /* OBSERVATION_ID  */
                         number,                     /* OBS_ORDER       */
                         this->setid,                /* SET_ID          */
                         Sequencer::TARGET_PENDING,  /* STATE           */
                         name,                       /* NAME            */
                         ra,                         /* RA              */
                         dec,                        /* DECL            */
                         "J2000",                    /* EPOCH           */
                         etime,                      /* EXPTIME         */
                         etime,                      /* OTMexpt         */
                         1,                          /* TARGET_NUMBER   */
                         1,                          /* SEQUENCE_NUMBER */
                         0.,                         /* CASANGLE        */
                         0.,                         /* OTMcass         */
                         slita,                      /* OTMslitangle    */
                         slitw,                      /* OTMslitwidth    */
                         slitw,                      /* SLITWIDTH       */
                         1,                          /* SLITOFFSET      */
                         1,                          /* BINSPECT        */
                         1                           /* BINSPAT         */
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
        message.str(""); message << "[DEBUG] " << cc << ": " << this->targetlist_cols.at(cc) << " = " << row[cc];
        logwrite( function, message.str() );
#endif
        if ( row[cc].isNull() ) {
          message.str(""); message << "ERROR " << this->targetlist_cols.at(cc) << " cannot be empty!";
          status = message.str();
          logwrite( function, message.str() );
          error = ERROR;
        }
      }
      if ( error == ERROR ) { return TARGET_ERROR; }

      // Connector/C++ does not support referring to row columns by their name yet (!)
      // so you have to get them by the order requested. The colnum() function returns
      // the correct column number.
      //
      col = this->colnum( "OBSERVATION_ID", this->targetlist_cols );  this->obsid       = row.get( col );
      col = this->colnum( "OBS_ORDER", this->targetlist_cols );       this->obsorder    = row.get( col );
      col = this->colnum( "NAME", this->targetlist_cols );            this->name        = row.get( col );
      col = this->colnum( "STATE", this->targetlist_cols );           this->state       = row.get( col );
      col = this->colnum( "RA", this->targetlist_cols );              this->ra_hms      = row.get( col );
      col = this->colnum( "DECL", this->targetlist_cols );            this->dec_dms     = row.get( col );
      col = this->colnum( "OTMcass", this->targetlist_cols );         this->casangle    = row.get( col );
      col = this->colnum( "OTMslitangle", this->targetlist_cols );    this->slitangle   = row.get( col );
      col = this->colnum( "OTMslitwidth", this->targetlist_cols );    this->slitwidth   = row.get( col );
      col = this->colnum( "SLITOFFSET", this->targetlist_cols );      this->slitoffset  = row.get( col );
      col = this->colnum( "OTMexpt", this->targetlist_cols );         this->exptime     = row.get( col );
      col = this->colnum( "TARGET_NUMBER", this->targetlist_cols );   this->targetnum   = row.get( col );
      col = this->colnum( "SEQUENCE_NUMBER", this->targetlist_cols ); this->sequencenum = row.get( col );
      col = this->colnum( "BINSPECT", this->targetlist_cols );        this->binspect    = row.get( col );
      col = this->colnum( "BINSPAT", this->targetlist_cols );         this->binspat     = row.get( col );
    }
    catch ( const mysqlx::Error &err ) {  /// catch errors thrown from mysqlx connector/C++ X DEV API
      message.str(""); message << "EXCEPTION from mySQL ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << err;
      status = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }
    catch ( std::exception &ex ) {        /// catch std::exceptions. This could be if this->colnum() returns a -1
      message.str(""); message << "EXCEPTION ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex.what();
      status = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }
    catch ( const char *ex ) {            /// catch everything else
      message.str(""); message << "EXCEPTION ";
      if ( col >= 0 && col < colcount ) { message << "(reading " << this->targetlist_cols.at(col) << ")"; }
      else { message << "( col = " << col << " )"; }
      message << ": " << ex;
      status = message.str();
      logwrite( function, message.str() );
      init_record();    // ensures that any previous record's info is not mistaken for this one
      return( TARGET_ERROR );
    }

    message.str(""); message << "retrieved target id " << this->obsid << " " << this->name << " " << this->ra_hms << " " << this->dec_dms
                             << " from set " << this->setid << " " << this->setname;
    status = message.str();
    logwrite( function, message.str() );

    // If we got to here then target is found, but do one last quality-control check before returning success
    //
    error = target_qc( status );

    return ( error == NO_ERROR ? TARGET_FOUND : TARGET_ERROR );
  }
  /***** Sequencer::TargetInfo::get_next **************************************/


  /***** Sequencer::TargetInfo::target_qc *************************************/
  /**
   * @brief      target info quality control
   * @details    applies a limited set of formatting rules and range checks on certain values
   * @param[out] status    reference to string to return status message
   * @return     ERROR or NO_ERROR
   *
   */
  long TargetInfo::target_qc( std::string &status ) {
    std::string function = "Sequencer::TargetInfo::target_qc";
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
      double _rah = this->radec_to_decimal( this->ra_hms );  // convert RA from HH:MM:SS.s to decimal hours
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
      double _dec = this->radec_to_decimal( this->dec_dms );  // convert DEC from DD:MM:SS.s to decimal degrees
      if ( _dec < -90.0 || _dec > 90.0 ) {
        message.str(""); message << "ERROR declination " << this->dec_dms << " outside range {-90:+90}";
        status = message.str();
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    return NO_ERROR;
  }
  /***** Sequencer::TargetInfo::target_qc *************************************/


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
      // which matches the current observation ID.
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
//                        "STATE",
                          "NAME",
                          "RA",
                          "DECL",
                          "EXPTIME",
                          "TARGET_NUMBER",
                          "SEQUENCE_NUMBER",
                          "SLITWIDTH",
                          "SLITOFFSET",
                          "BINSPECT",
                          "BINSPAT",
                          "CASANGLE"
                        )
                .values( this->obsid,
//                       Sequencer::TARGET_COMPLETE,
                         this->name,
                         this->ra_hms,
                         this->dec_dms,
                         this->exptime,
                         this->targetnum,
                         this->sequencenum,
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


  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/
  /**
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in  input string to convert
   * @return     double representation of string, or NaN on error
   *
   * Input string could be  HH:MM:SS.sss
   *                        HH MM SS.sss
   *                       ±DD:MM:SS.sss
   *                       ±DD MM SS.sss
   *                       ± D:MM:SS.sss
   *                       ± D MM SS.sss
   *
   * Convert the input string into a decimal (double) number, HH.hhh or ±DD.dddd
   *
   * If the string is empty or otherwise cannot be converted then return NaN.
   *
   */
  double TargetInfo::radec_to_decimal( std::string str_in ) {
    std::string dontcare;
    return( this->radec_to_decimal( str_in, dontcare ) );
  }
  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/


  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/
  /**
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in     input string to convert
   * @param[out] retstring  reference to string representation of return value
   * @return     double representation of string, or NaN on error
   *
   * This function is overloaded.
   * This version accepts a reference to a return string, to return a string
   * version of the decimal (double) return value.
   *
   */
  double TargetInfo::radec_to_decimal( std::string str_in, std::string &retstring ) {
    std::string function = "Sequencer::TargetInfo::radec_to_decimal";
    std::stringstream message;
    std::vector<std::string> tokens;
    double sign=1.0;

    // can't convert an empty string to a value other than NaN
    //
    if ( str_in.empty() ) {
      logwrite( function, "ERROR: empty input string returns NaN" );
      return( NAN );
    }

    // If there's a minus sign (-) in the input string then set the sign
    // multiplier negative, then remove the sign.
    //
    // This is done because tokenizing on space or colon would result in three separate
    // tokens (HH MM SS or DD MM SS) except for the case where the degree is a single 
    // digit, then it's possible that tokenizing " + D MM SS.sss" it could result in four 
    // tokens, "+", "D", "MM", "SS.sss" so determine the sign then get rid of it.
    //
    if ( str_in.find( '-' ) != std::string::npos ) sign = -1.0;
    str_in.erase( std::remove( str_in.begin(), str_in.end(), '-' ), str_in.end() );
    str_in.erase( std::remove( str_in.begin(), str_in.end(), '+' ), str_in.end() );

    Tokenize( str_in, tokens, " :" );  // tokenize on space or colon

    if ( tokens.size() != 3 ) {
      message.str(""); message << "ERROR: expected 3 tokens but received " << tokens.size() << " from str_in \"" << str_in << "\"";
      logwrite( function, message.str() );
      return( NAN );
    }

    double hh, mm, ss, dec;
    std::stringstream ret;
    try {
      hh = std::stod( tokens.at(0) );
      mm = std::stod( tokens.at(1) ) / 60.0;
      ss = std::stod( tokens.at(2) ) / 3600.0;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR: out of range parsing input string \"" << str_in << "\": " << e.what();
      logwrite( function, message.str() );
      return( NAN );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR: invalid argument parsing input string \"" << str_in << "\": " << e.what();
      logwrite( function, message.str() );
      return( NAN );
    }

    dec = sign * ( hh + mm + ss );
    ret << std::fixed << std::setprecision(6) << dec;
    retstring = ret.str();

    return( dec );
  }
  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/


  /***** Sequencer::TargetInfo::decimal_to_sexa *******************************/
  /**
   * @brief      convert decimal number to sexagesimal
   * @param[in]  str_in     input string to convert
   * @param[out] retstring  reference to string representation of return value
   * @return     double representation of string, or NaN on error
   *
   * This function is overloaded.
   * This version accepts a reference to a return string, to return a string
   * version of the decimal (double) return value.
   *
   */
  void TargetInfo::decimal_to_sexa( double dec_in, std::string &retstring ) {
    std::string function = "Sequencer::TargetInfo::decimal_to_sexa";
    std::stringstream message;
    std::string sign = ( dec_in < 0 ? "-" : "+" );

    double hh, mm, ss, dec;
    double fractpart, intpart;

    fractpart = std::modf( std::abs(dec_in), &intpart );
    hh = intpart;
    dec = fractpart * 60.0;

    fractpart = std::modf( dec, &intpart );
    mm = intpart;
    ss = fractpart * 60;

    std::stringstream ret;

    ret << sign << std::setw(2) << std::setfill('0') << hh << ":"
                << std::setw(2) << std::setfill('0') << mm << ":"
                << std::fixed << std::setprecision(2) << ss;
    retstring = ret.str();

    return;
  }
  /***** Sequencer::TargetInfo::decimal_to_sexa *******************************/


  /***** Sequencer::PowerSwitch::configure ************************************/
  /**
   * @brief      
   * @param[in]  arglist
   * @return     ERROR or NO_ERROR
   *
   */
  long PowerSwitch::configure( std::string arglist ) {
    std::string function = "Sequencer::PowerSwitch::PowerSwitch";
    std::stringstream message;
    std::vector<std::string> tokens;

    int size = Tokenize( arglist, tokens, " \t" );

    for ( const auto &tok : tokens ) {
      this->plugname.push_back( tok );
    }

    return( size < 1 ? ERROR : NO_ERROR );
  }
  /***** Sequencer::PowerSwitch::configure ************************************/


  /***** Sequencer::FPOffsets::FPOffsets **************************************/
  /**
   * @brief      class constructor
   *
   */
  FPOffsets::FPOffsets() {
    std::string function = "FPOffsets::FPOffsets";
    std::stringstream message;

    if ( !this->py_instance.is_initialized() ) {
      logwrite( function, "ERROR could not initialize Python" );
      this->python_initialized = false;
      return;
    }

    this->pModuleName = PyUnicode_FromString( PYTHON_FPOFFSETS_MODULE );
    this->pModule     = PyImport_Import( this->pModuleName );
    this->python_initialized = true;
  }
  /***** Sequencer::FPOffsets::FPOffsets **************************************/


  /***** Sequencer::FPOffsets::compute_offset *********************************/
  /**
   * @brief      calculate focal plane offsets of one component from another
   * @param[in]  from       component to convert from
   * @param[in]  ra_in      ra in
   * @param[in]  dec_in     dec in
   * @param[in]  angle_in   angle in
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version is passed in all outputs. Outputs are stored in the class.
   * All units are in degrees.
   *
   */
  long FPOffsets::compute_offset( std::string from, std::string to, double ra_in, double dec_in, double angle_in ) {
    this->coords_in.ra    = ra_in;
    this->coords_in.dec   = dec_in;
    this->coords_in.angle = angle_in;
    return compute_offset( from, to );
  }
  /***** Sequencer::FPOffsets::compute_offset *********************************/


  /***** Sequencer::FPOffsets::compute_offset *********************************/
  /**
   * @brief      calculate focal plane offsets of one component from another
   * @param[in]  from       component to convert from
   * @param[in]  to         component to convert to
   * @param[in]  ra_in      ra in
   * @param[in]  dec_in     dec in
   * @param[in]  angle_in   angle in
   * @param[out] ra_out     computed ra
   * @param[out] dec_out    computed dec
   * @param[out] angle_out  computed angle
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version is passed all inputs and returns all outputs by reference.
   * All units are in degrees.
   *
   */
  long FPOffsets::compute_offset( std::string from, std::string to, 
                                  double ra_in, double dec_in, double angle_in,
                                  double &ra_out, double &dec_out, double &angle_out ) {
    this->coords_in.ra    = ra_in;
    this->coords_in.dec   = dec_in;
    this->coords_in.angle = angle_in;

    long error = compute_offset( from, to );

    ra_out    = this->coords_out.ra;
    dec_out   = this->coords_out.dec;
    angle_out = this->coords_out.angle;

    return error;
  }
  /***** Sequencer::FPOffsets::compute_offset *********************************/


  /***** Sequencer::FPOffsets::compute_offset *********************************/
  /**
   * @brief      calculate focal plane offsets of one component from another
   * @param[in]  from       component to convert from
   * @param[in]  to         component to convert to
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded. This is the version that does the work;
   * the other versions call this one. This takes the inputs from class
   * members and outputs to class members.
   *
   */
  long FPOffsets::compute_offset( std::string from, std::string to ) {
    std::string function = "FPOffsets::compute_offset";
    std::stringstream message;

//#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[ACQUIRE] from=" << from << " to=" << to
                             << " coords_in.ra=" << this->coords_in.ra
                             << " .dec=" << this->coords_in.dec
                             << " .angle=" << this->coords_in.angle << " deg";
    logwrite( function, message.str() );
//#endif

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return( ERROR );
    }

    if ( this->pModule==NULL ) {
      logwrite( function, "ERROR: Python module not imported" );
      return( ERROR );
    }

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_FPOFFSETS_FUNCTION );

    const char* _fromc = from.c_str();
    const char* _toc   = to.c_str();

    // Build up the PyObject argument list that will be passed to the function
    //
    PyObject* pArgList = Py_BuildValue( "(ssddd)", _fromc, _toc, 
                                        this->coords_in.ra, 
                                        this->coords_in.dec,
                                        this->coords_in.angle
                                      );

    // Call the Python function here
    //
    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python function not callable" );
      return( ERROR );
    }

    PyObject* pReturn = PyObject_CallObject( pFunction, pArgList );

    // Expected back a tuple
    //
    if ( !PyTuple_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not receive a tuple" );
      return( ERROR );
    }

    int tuple_size = PyTuple_Size( pReturn );

    // Put each tuple item in its place
    //
    for ( int tuplen = 0; tuplen < tuple_size; tuplen++ ) {
      PyObject* pItem = PyTuple_GetItem( pReturn, tuplen );  // grab an item
      if ( PyFloat_Check( pItem ) ) {
        switch ( tuplen ) {
          case 0: this->coords_out.ra    = PyFloat_AsDouble( pItem ); break;
          case 1: this->coords_out.dec   = PyFloat_AsDouble( pItem ); break;
          case 2: this->coords_out.angle = PyFloat_AsDouble( pItem ); break;
          default:
            message.str(""); message << "ERROR unexpected tuple item " << tuplen << ": expected {0,1,2}";
            logwrite( function, message.str() );
            return( ERROR );
            break;
        }
      }
    }

    // Checking after extracting, because it may allow for partial extraction
    //
    if ( tuple_size != 3 ) {
      message.str(""); message << "ERROR unexpected 3 tuple items but received " << tuple_size;
      logwrite( function, message.str() );
      return( ERROR );
    }

//#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[ACQUIRE] coords_out.ra=" << this->coords_out.ra
                             << " .dec=" << this->coords_out.dec
                             << " .angle=" << this->coords_out.angle << " deg";
    logwrite( function, message.str() );
//#endif

    return NO_ERROR;
  }
  /***** Sequencer::FPOffsets::compute_offset *********************************/


  /***** Sequencer::FPOffsets::solve_offset ***********************************/
  /**
   * @brief      calculate offsets between ACAM and goal
   * @param[in]  ra_acam   current solution for ACAM RA
   * @param[in]  dec_acam  current solution for ACAM DEC
   * @param[in]  ra_goal   RA goal
   * @param[in]  dec_goal  DEC goal
   * @param[in]  ra_off    reference to computed offset in RA to reach goal
   * @param[in]  dec_off   reference to computed offset in DEC to reach goal
   * @return     ERROR or NO_ERROR
   *
   * @details    The TCS "PT" command moves the telescope along a great circle.
   *             Given a current position and a goal, this calls a Python
   *             function which computes the offsets needed to move the ACAM
   *             to the goal via the PT command.
   *
   */
  long FPOffsets::solve_offset( double ra_acam, double dec_acam, double ra_goal, double dec_goal,
                                double &ra_off, double &dec_off ) {
    std::string function = "Sequencer::FPOffsets::solve_offset";
    std::stringstream message;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] ra_acam=" << ra_acam << " dec_acam=" << dec_acam << " ra_goal=" << ra_goal << " dec_goal=" << dec_goal;
    logwrite( function, message.str() );
#endif

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return( ERROR );
    }

    if ( this->pModule==NULL ) {
      logwrite( function, "ERROR: Python module not imported" );
      return( ERROR );
    }

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_SOLVEOFFSETDEG_FUNCTION );

    // Build up the PyObject argument list that will be passed to the function
    //
    PyObject* pArgList = Py_BuildValue( "(dddd)", ra_acam, dec_acam, ra_goal, dec_goal );

    // Call the Python function here
    //
    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python function not callable" );
      return( ERROR );
    }

    PyObject* pReturn = PyObject_CallObject( pFunction, pArgList );

    // Expected back a tuple
    //
    if ( !PyTuple_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not receive a tuple" );
      return( ERROR );
    }

    int tuple_size = PyTuple_Size( pReturn );

    // Put each tuple item in its place
    //
    for ( int tuplen = 0; tuplen < tuple_size; tuplen++ ) {
      PyObject* pItem = PyTuple_GetItem( pReturn, tuplen );  // grab an item
      if ( PyFloat_Check( pItem ) ) {
        switch ( tuplen ) {
          case 0: ra_off  = PyFloat_AsDouble( pItem ); break;
          case 1: dec_off = PyFloat_AsDouble( pItem ); break;
          default:
            message.str(""); message << "ERROR unexpected tuple item " << tuplen << ": expected {0,1}";
            logwrite( function, message.str() );
            return( ERROR );
            break;
        }
      }
    }

    // Checking after extracting, because it may allow for partial extraction
    //
    if ( tuple_size != 2 ) {
      message.str(""); message << "ERROR expected 2 tuple items but received " << tuple_size;
      logwrite( function, message.str() );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] ra_off=" << ra_off << " dec_off=" << dec_off;
    logwrite( function, message.str() );
#endif

    return NO_ERROR;
  }
  /***** Sequencer::FPOffsets::solve_offset ***********************************/

}
