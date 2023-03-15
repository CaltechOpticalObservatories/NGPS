/**
 * @file    acam_interface.cpp
 * @brief   this contains the acam interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Acam namespace,
 * and is how the acam daemon interfaces to the acam hardware.
 *
 */

#include "acam_interface.h"

namespace Acam {


  /***** Acam::Camera::Camera *************************************************/
  /**
   * @brief      class constructor
   *
   */
  Camera::Camera() {
  }
  /***** Acam::Camera::Camera *************************************************/


  /***** Acam::Camera::~Camera ************************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Camera::~Camera() {
  }
  /***** Acam::Camera::~Camera ************************************************/


  /***** Acam::Camera::open ***************************************************/
  /**
   * @brief      open connection to Andor and initialize SDK
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::open() {
    std::string function = "Acam::Camera::open";
    std::stringstream message;
    long error=NO_ERROR;

    if ( this->andor.is_initialized() ) {
      logwrite( function, "already initialized" );
    }
    else {
      error = this->andor.open();
    }
    return error;
  }
  /***** Acam::Camera::open ***************************************************/


  /***** Acam::Camera::close **************************************************/
  /**
   * @brief      close connection to Andor
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::close() {
    return this->andor.close();
  }
  /***** Acam::Camera::close **************************************************/


  /***** Acam::Camera::start_acquisition **************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::start_acquisition() {
    return ERROR;
  }
  /***** Acam::Camera::start_acquisition **************************************/


  /***** Acam::Camera::get_status *********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::get_status() {
    return ERROR;
  }
  /***** Acam::Camera::get_status *********************************************/


  /***** Acam::Camera::get_frame **********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::get_frame() {
    unsigned long framepix = this->andor.camera_info.cols * this->andor.camera_info.rows;

    // Allocate storage if needed
    //
    if ( this->image_data != NULL &&
         this->andor.camera_info.npix == framepix ) {
    }
    else
    if ( this->image_data != NULL &&
         this->andor.camera_info.npix != framepix ) {
      delete [] (uint16_t*)this->image_data;
      this->image_data = NULL;
    }

    if ( this->image_data == NULL ) {
      this->image_data = (uint16_t*)new int16_t[ framepix ];
    }

    unsigned int ret = this->andor.get_last_frame( (uint16_t*)this->image_data );

    if ( ret == DRV_SUCCESS ) return NO_ERROR; else return ERROR;
  }
  /***** Acam::Camera::get_frame **********************************************/


  /***** Acam::Camera::test_image *********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::test_image( ) {
    std::string function = "Acam::Interface::test_image";
    std::stringstream message;
    long error = NO_ERROR;

    error = this->camera.andor.test();

    return( error );
  }
  /***** Acam::Camera::test_image *********************************************/


  /***** Acam::CameraServer::CameraServer *************************************/
  /**
   * @brief      default constructor
   *
   */
  CameraServer::CameraServer() {
    this->name = "AndorCameraServer";
    this->host = "";
    this->port = -1;
  }
  /***** Acam::CameraServer::CameraServer *************************************/


  /***** Acam::CameraServer::CameraServer *************************************/
  /**
   * @brief      default constructor
   * @param[in]  host
   * @param[in]  port
   *
   */
  CameraServer::CameraServer( std::string host, int port ) {
    this->name = "AndorCameraServer";
    this->host = host;
    this->port = port;
  }
  /***** Acam::CameraServer::CameraServer *************************************/


  /***** Acam::CameraServer::CameraServer *************************************/
  /**
   * @brief      default constructor
   * @param[in]  name  name for this component (info only)
   * @param[in]  host  host for this component
   * @param[in]  port  port number for this component
   *
   */
  CameraServer::CameraServer( std::string name, std::string host, int port ) {
    this->name = name;
    this->host = host;
    this->port = port;
  }
  /***** Acam::CameraServer::CameraServer *************************************/


  /***** Acam::CameraServer::~CameraServer ************************************/
  /**
   * @brief      default deconstructor
   *
   */
  CameraServer::~CameraServer( ) {
  }
  /***** Acam::CameraServer::~CameraServer ************************************/


  /***** Acam::CameraServer::open *********************************************/
  /**
   * @brief      open a connection to the acam server
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::open() {
    std::string function = "Acam::CameraServer::open";
    std::stringstream message;

    if ( this->server.isconnected() ) {
      logwrite( function, "connection already open" );
      return( NO_ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->server = s;

    // Initialize connection to the acam camera server
    //
    logwrite( function, "opening connection to external camera server" );

    if ( this->server.Connect() != 0 ) {
      logwrite( function, "ERROR connecting to external camera server" );
      return( ERROR );
    }

    message.str(""); message << "socket connection to "
                             << this->server.gethost() << ":" << this->server.getport()
                             << " established on fd " << this->server.getfd();
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Acam::CameraServer::open *********************************************/


  /***** Acam::CameraServer::close ********************************************/
  /**
   * @brief      close the connection to the acam camera server
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::close() {
    std::string function = "Acam::CameraServer::close";
    std::stringstream message;

    if ( !this->server.isconnected() ) {
      logwrite( function, "connection already closed" );
      return( NO_ERROR );
    }

    long error = this->server.Close();

    if ( error == NO_ERROR ) {
      logwrite( function, "connection to acam camera server closed" );
    }
    else {
      logwrite( function, "ERROR closing acam camera server connection" );
    }

    return( error );
  }
  /***** Acam::CameraServer::close ********************************************/


  /***** Acam::CameraServer::acquire ******************************************/
  /**
   * @brief      acquire an image from the acam camera server
   * @param[in]  wcsname    if provided, the name of a previous image with WCS info
   * @param[out] imagename  filename of the new image taken by the server
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::acquire( std::string wcsname, std::string &imagename ) {
    std::string function = "Acam::CameraServer::acquire";
    std::stringstream message;
    std::string cmd = "acquire";

    // Nothing to do if not connected
    //
    if ( !this->server.isconnected() ) {
      logwrite( function, "ERROR: no connection to the camera server" );
      return( ERROR );
    }

    // Include the filename of a WCS-corrected file, if provided
    //
    if ( ! wcsname.empty() ) { cmd.append( " " ); cmd.append( wcsname ); }

    // Send command to the server here
    //
    long error = this->send_command( cmd, imagename );

    message.str(""); message << "[DEBUG] wcsname=" << wcsname << " imagename=" << imagename;
    logwrite( function, message.str() );

    return( error );
  }
  /***** Acam::CameraServer::acquire ******************************************/


  /***** Acam::CameraServer::coords *******************************************/
  /**
   * @brief      send coords to the camera server
   * @param[in]  coords_in  string containing coords to write
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::coords( std::string coords_in ) {
    std::string function = "Acam::CameraServer::coords";
    std::stringstream message;

    // Nothing to do if not connected
    //
    if ( !this->server.isconnected() ) {
      logwrite( function, "ERROR: no connection to the camera server" );
      return( ERROR );
    }

    // Otherwise send the command to the camera server
    //
    std::stringstream cmd;
    std::string reply;      // reply not used
    cmd << ACAMD_CAMERASERVER_COORDS << " " << coords_in;
    return this->send_command( cmd.str(), reply );
  }
  /***** Acam::CameraServer::coords *******************************************/


  /***** Acam::CameraServer::send_command *************************************/
  /**
   * @brief      send a command string to the camera server
   * @param[in]  cmd  string command
   * @return     ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded with a version that accepts a return string.
   * This version sends a command only and does not read back any reply.
   *
   */
  long CameraServer::send_command( std::string cmd ) {
    std::string function = "Acam::CameraServer::send_command";
    std::stringstream message;

    logwrite( function, cmd );

    cmd.append( "\n" );                        // add the newline character

    int written = this->server.Write( cmd );   // write the command

    if ( written <= 0 ) return( ERROR );       // return error if error writing to socket

    return( NO_ERROR );
  }
  /***** Acam::CameraServer::send_command *************************************/


  /***** Acam::CameraServer::send_command *************************************/
  /**
   * @brief      send a command string to the camera server
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply
   * @return     ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded.
   *
   * This version is called with a reference to return string, in which case 
   * after writing the command the reply is read and placed into the return
   * string.
   *
   */
  long CameraServer::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Acam::CameraServer::send_command";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    // send the command
    //
    error = this->send_command( cmd );

    if ( error == ERROR ) {
      message.str(""); message << "ERROR sending command: " << cmd;
      logwrite( function, message.str() );
    }

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=this->server.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << this->server.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << this->server.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->server.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "error reading from camera server: " << strerror( errno );
        logwrite( function, message.str() );
        break;
      }

      // remove any newline characters and get out
      //
      reply.erase(std::remove(reply.begin(), reply.end(), '\r' ), reply.end());
      reply.erase(std::remove(reply.begin(), reply.end(), '\n' ), reply.end());
      break;

    }

message.str(""); message << "[DEBUG] got back reply=" << reply; logwrite( function, message.str() );
    retstring = reply;

    return( error );
  }
  /***** Acam::CameraServer::send_command *************************************/


  /***** Acam::Astrometry::Astrometry *****************************************/
  /**
   * @brief      class constructor
   *
   */
  Astrometry::Astrometry() {
    std::string function = "Astrometry::Astrometry";
    std::stringstream message;

    if ( !this->py_instance.is_initialized() ) {
      logwrite( function, "ERROR could not initialize Python" );
      return;
    }

    this->pModuleName = PyUnicode_FromString( PYTHON_ASTROMETRY_MODULE );
    this->pModule     = PyImport_Import( this->pModuleName );

    if ( this->pModule == NULL ) {
      PyErr_Print();
      this->python_initialized = false;
      return;
    }
    else this->python_initialized = true;

    this->isacquire=true;
    this->solver_arglist="";

    logwrite( "Astrometry::Astrometry", "initialized Python astrometry module" );
    return;
  }
  /***** Acam::Astrometry::Astrometry *****************************************/


  /***** Acam::Astrometry::~Astrometry ****************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Astrometry::~Astrometry() {
  }
  /***** Acam::Astrometry::~Astrometry ****************************************/


  /***** Acam::Astrometry::solve **********************************************/
  /**
   * @brief      call the Python astrometry solver
   * @param[in]  imagename_in  fits filename to give to the solver
   *
   */
  long Astrometry::solve( std::string imagename_in ) {
    std::string function = "Acam::Astrometry::solve";
    std::stringstream message;

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return( ERROR );
    }

    if ( this->pModule==NULL ) {
      logwrite( function, "ERROR: Python astrometry module not imported" );
      return( ERROR );
    }

    // Can't proceed unless there is an imagename
    //
    if ( imagename_in.empty() ) {
      logwrite( function, "ERROR: imagename cannot be empty" );
      return ERROR;
    }

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_ASTROMETRY_FUNCTION );

    const char* imagename = imagename_in.c_str();

    PyObject* pArgList = Py_BuildValue( "(s)", imagename );

    // There is always at minimum acquire={true|false}
    // and other optional key=val pairs can be added to a space-delimited string.
    //
    PyObject* pKeywords = PyDict_New();
    PyDict_SetItemString( pKeywords, "acquire", this->isacquire ? Py_True : Py_False );

    // Build up a list of key=val arguments to send to the solver from a
    // space-delimted string, which is disected here.
    //
#ifdef NOT_YET_IMPLEMENTED
    if ( !this->solver_arglist.empty() ) {  // only do this if the arglist is not empty

      // Tokenize on space " " to get the pairs "key=val"
      //
      std::vector<std::string> argpair;
      Tokenize( this->solver_arglist, argpair, " " );

      // Loop through each "key=val" pair
      //
      try {
        for ( int pairn=0; pairn < argpair.size(); pairn++ ) {

          // Tokenize each argpair on "=" to separate the key and value
          //
          std::vector<std::string> keyval;
          Tokenize( argpair.at(pairn), keyval, "=" );

          // Add the arg pair to the keywords list
          //
          const char* keyname = keyval.at(0);
          PyDict_SetItemString( pKeywords, keyname, this->isacquire ? Py_True : Py_False );
        }
      }
      catch( std::out_of_range const& ) {
        message.str(""); message << "ERROR: out of range parsing key=value pair from arglist \"" << this->solver_arglist << "\"";
        logwrite( function, message.str() );
        return ERROR;
      }
      catch( ... ) {
        message.str(""); message << "ERROR unknown exception parsing key=value pair from arglist \"" << this->solver_arglist << "\"";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
#endif

    double t0=0, t1=0;

    // Call the Python astrometry function here
    //
    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python astrometry function not callable" );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] calling Python function: " << PYTHON_ASTROMETRY_FUNCTION;
    logwrite( function, message.str() );
#endif

    t0=get_clock_time();
    PyObject* pReturn = PyObject_Call( pFunction, pArgList, pKeywords );
    t1=get_clock_time();

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] Python call time " << (t1-t0) << " sec";
    logwrite( function, message.str() );
#endif

    // Check the return values from Python here
    //
    if ( ! pReturn ) {
      logwrite( function, "ERROR calling Python astrometry solver" );
      return( ERROR );
    }

    // Expected a List: [ string, float, float, float ]
    //
    if ( PyList_Check( pReturn ) ) {

      int list_size = PyList_Size( pReturn );

      for ( int listn = 0; listn < list_size; ++listn ) {    // loop through the List to get each item
        PyObject* pItem = PyList_GetItem( pReturn, listn );  // grab an item

        // The first item is a string
        //
        if ( listn == 0 ) {
          if ( !PyUnicode_Check( pItem ) ) {
            logwrite( function, "ERROR expected a string for item 0" );
            return( ERROR );
          }
          int itemlen    = PyUnicode_GetLength( pItem );
          int itemkind   = PyUnicode_KIND( pItem );
          void* itemdata = PyUnicode_DATA( pItem );
          PyUnicode_READ( itemkind, itemdata, itemlen );
          this->result.assign( (char*)itemdata );
        }

        // and the subsequent items are floats
        //
        else {
          if ( PyFloat_Check( pItem ) ) {
            switch ( listn ) {
              case 1: this->ra  = PyFloat_AsDouble( pItem ); break;
              case 2: this->dec = PyFloat_AsDouble( pItem ); break;
              case 3: this->pa  = PyFloat_AsDouble( pItem ); break;
              default:
                message.str(""); message << "ERROR unexpected list item " << listn << ": expected {0,1,2,3}";
                logwrite( function, message.str() );
                return( ERROR );
                break;
            }
          }
          else {  // if ! PyFloat_Check(pItem)
            message.str(""); message << "ERROR expected float for item " << listn;
            logwrite( function, message.str() );
            return( ERROR );
          }
        }
      }

    }
    else {
      logwrite( function, "ERROR Python astrometry function did not return expected list" );
      return( ERROR );
    }

    message.str(""); message << "result=" << this->result
                             << " RA="    << this->ra
                             << " DEC="   << this->dec
                             << " PA="    << this->pa
                             << " solve time=" << t1-t0 << " sec";
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** Acam::Astrometry::solve **********************************************/


  /***** Acam::Interface::Interface *******************************************/
  /**
   * @brief      class constructor
   * @details    The andor camera will be initialized upon construction
   */
  Interface::Interface() {
    this->cameraserver_host="";
    this->motion_host="";
    this->cameraserver_port=-1;
    this->motion_port=-1;
    this->imagename="";
    this->wcsname="";
  }
  /***** Acam::Interface::Interface *******************************************/


  /***** Acam::Interface::~Interface ******************************************/
  /**
   * @brief      class deconstructor
   * @details    The andor camera will be closed upon de-construction
   *
   */
  Interface::~Interface() {
  }
  /***** Acam::Interface::~Interface ******************************************/


  /***** Acam::Interface::initialize_class ************************************/
  /**
   * @brief      initializes the class from configure_acam()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Acam::Server::configure_acam() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Acam::Interface::initialize_class";
    std::stringstream message;

#ifdef ACAM_ANDOR_SOURCE_SERVER
    if ( this->cameraserver_port < 0 || this->cameraserver_host.empty() ) {
      message.str(""); message << "ERROR: external camera server (" << this->cameraserver_host << ") or port (" 
                               << this->cameraserver_port << ") invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Initialize the network interface to the external camera server
    //
    CameraServer s( this->cameraserver_host, this->cameraserver_port );
    this->camera_server = s;
#endif

    return( NO_ERROR );
  }
  /***** Acam::Interface::initialize_class ************************************/


  /***** Acam::Interface::open ************************************************/
  /**
   * @brief      wrapper to open all acam hardware components
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "Acam::Interface::open";
    std::stringstream message;
    long error = NO_ERROR;

//  error |= this->motion.open();

#ifdef ACAM_ANDOR_SOURCE_SERVER
    error |= this->camera_server.open();  // get images from external server
#endif

#ifdef ACAM_ANDOR_SOURCE_ANDOR
    error |= this->camera.open();         // get images from Andor directly
#endif

    if ( error != NO_ERROR ) logwrite( function, "ERROR: one or more components failed to open" );

    return( error );
  }
  /***** Acam::Interface::open ************************************************/


  /***** Acam::Interface::isopen **********************************************/
  /**
   * @brief      wrapper for all acam hardware components
   * @return     ERROR or NO_ERROR
   *
   */
  bool Interface::isopen() {
    std::string function = "Acam::Interface::isopen";
    std::stringstream message;
    bool _is_open = true;  // default True, then "and" will make false if any one is false

//  _is_open &= this->motion.isopen();

#ifdef ACAM_ANDOR_SOURCE_SERVER
    _is_open &= this->camera_server.isopen();
#endif

#ifdef ACAM_ANDOR_SOURCE_ANDOR
//  _is_open &= this->camera.isopen();
    _is_open &= this->andor.is_initialized();
#endif
    return( _is_open );
  }
  /***** Acam::Interface::isopen **********************************************/


  /***** Acam::Interface::close ***********************************************/
  /**
   * @brief      wrapper for all acam hardware components
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "Acam::Interface::close";
    std::stringstream message;
    long error = NO_ERROR;

//  error |= this->motion.close();
//  error |= this->motion.send_command( "close" );  // just needed for the emulator

#ifdef ACAM_ANDOR_SOURCE_SERVER
    error |= this->camera_server.close();
#endif

#ifdef ACAM_ANDOR_SOURCE_ANDOR
    error |= this->camera.close();
#endif

    return( error );
  }
  /***** Acam::Interface::close ***********************************************/


  /***** Acam::Interface::acquire *********************************************/
  /**
   * @brief      wrapper to acquire an Andor image
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::acquire() {
    std::string function = "Acam::Interface::acquire";
    std::stringstream message;
    long error = NO_ERROR;

#ifdef ACAM_ANDOR_SOURCE_SERVER
    std::string _imagename = this->imagename;
    this->camera_server.acquire( this->wcsname, _imagename );
    this->imagename = _imagename;
#endif

#ifdef ACAM_ANDOR_SOURCE_ANDOR
#endif

    message.str(""); message << "[DEBUG] this->wcsname=" << this->wcsname << " this->imagename=" << this->imagename;
    logwrite( function, message.str() );

    return( error );
  }
  /***** Acam::Interface::acquire *********************************************/


  /***** Acam::Interface::exptime *********************************************/
  /**
   * @brief      wrapper to set exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "Acam::Interface::exptime";
    std::stringstream message;
#ifdef ACAM_ANDOR_SOURCE_SERVER
    logwrite( function, "ERROR: not yet implemented" );
    return( ERROR );
#endif

#ifdef ACAM_ANDOR_SOURCE_ANDOR
    return this->andor.exptime( exptime_in, retstring );
#endif
  }
  /***** Acam::Interface::exptime *********************************************/


  /***** Acam::Interface::solve ***********************************************/
  /**
   * @brief      wrapper for Astrometry::solve()
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::solve( std::string imagename_in, std::string &retstring ) {
    std::string function = "Acam::Interface::solve";
    std::stringstream message;
    long error = NO_ERROR;
    std::string _imagename;
    std::string _wcsname;

    message.str(""); message << "[DEBUG] this->wcsname=" << this->wcsname << " this->imagename=" << this->imagename;
    logwrite( function, message.str() );

    // If no image name was provided to this function then get it from the class
    //
    _imagename = ( imagename_in.empty() ? this->get_imagename() : imagename_in );

    // They can't both be empty, need an image name from somewhere
    //
    if ( _imagename.empty() ) {
      logwrite( function, "ERROR: imagename not provided and no name from previous solve" );
      return ERROR;
    }

    // Call the astrometry solver
    //
    error = this->astrometry.solve( _imagename );

    // Put the results into the return string
    //
    retstring = this->astrometry.get_result();

    // Astrometry::solve() will create a new file with "_WCSfix" appended to
    // the filename (before the .fits extension) but it doesn't return that,
    // you just have to know. So that extension is added here and the new
    // name is stored in the class.
    //
    std::string suffix = "_WCSfix";
    
    try {
      size_t dotfits = _imagename.find( ".fits" );
      _wcsname = _imagename;
      _wcsname.insert( dotfits, suffix );
    }
    catch( std::out_of_range const& ) {
      message.str(""); message << "ERROR: malformed filename? Could not find \".fits\" in imagename \"" << _imagename << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( ... ) {
      message.str(""); message << "ERROR unknown exception inserting suffix into \"" << _wcsname << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Save the wcsname to the class
    //
    this->set_wcsname( _wcsname );

    return error;
  }
  /***** Acam::Interface::solve ***********************************************/


}
