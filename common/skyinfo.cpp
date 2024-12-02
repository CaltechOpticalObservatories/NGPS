#include "skyinfo.h"

namespace SkyInfo {

  /***** SkyInfo::FPOffsets::FPOffsets ****************************************/
  /**
   * @brief      class constructor
   *
   */
  FPOffsets::FPOffsets() {
    std::string function = "SkyInfo::FPOffsets::FPOffsets";
    std::stringstream message;

    if ( !this->py_instance.is_initialized() ) {
      logwrite( function, "ERROR could not initialize Python" );
      this->python_initialized = false;
      return;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();   // Acquire the GIL

    this->pModuleName = PyUnicode_FromString( PYTHON_FPOFFSETS_MODULE );
    this->pModule     = PyImport_Import( this->pModuleName );

    PyGILState_Release( gstate );                    // Release the GIL

    this->python_initialized = true;
  }
  /***** SkyInfo::FPOffsets::FPOffsets ****************************************/


  /***** SkyInfo::FPOffsets::recompute_offset *********************************/
  long FPOffsets::recompute_offset( const std::string &from, const std::string &to,
                                    double &ra_out, double &dec_out, double &angle_out ) {
    long error = compute_offset( from, to );

    ra_out    = this->coords_out.ra;
    dec_out   = this->coords_out.dec;
    angle_out = this->coords_out.angle;

    return error;
  }
  /***** SkyInfo::FPOffsets::recompute_offset *********************************/


  long FPOffsets::compute_offset_last_angle( const std::string &from, const std::string &to,
                                             const double ra_in, const double dec_in,
                                             double &ra_out, double &dec_out, double &angle_out ) {
    this->coords_in.ra    = ra_in;
    this->coords_in.dec   = dec_in;
    this->coords_in.angle = this->coords_out.angle;  // use the last calculated angle as input

    long error = compute_offset( from, to );

    ra_out    = this->coords_out.ra;
    dec_out   = this->coords_out.dec;
    angle_out = this->coords_out.angle;

    return error;
  }
  /***** SkyInfo::FPOffsets::compute_offset ***********************************/
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
  long FPOffsets::compute_offset( const std::string &from, const std::string &to,
                                  const double ra_in, const double dec_in, const double angle_in ) {
    this->coords_in.ra    = ra_in;
    this->coords_in.dec   = dec_in;
    this->coords_in.angle = angle_in;
    return compute_offset( from, to );
  }
  /***** SkyInfo::FPOffsets::compute_offset ***********************************/


  /***** SkyInfo::FPOffsets::compute_offset ***********************************/
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
  long FPOffsets::compute_offset( const std::string &from, const std::string &to,
                                  const double ra_in, const double dec_in, const double angle_in,
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
  /***** SkyInfo::FPOffsets::compute_offset ***********************************/


  /***** SkyInfo::FPOffsets::compute_offset ***********************************/
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
  long FPOffsets::compute_offset( const std::string &from, const std::string &to ) {
    std::string function = "SkyInfo::FPOffsets::compute_offset";
    std::stringstream message;

    // Check for valid inputs before passing something to Python that
    // it won't like.
    //
    if ( from.empty() || to.empty()        ||
         std::isnan( this->coords_in.ra )  ||
         std::isnan( this->coords_in.dec ) ||
         std::isnan( this->coords_in.angle ) ) {
      logwrite( function, "ERROR empty from/to string(s) or one or more input values is NaN" );
      message.str(""); message << "from=" << from << " to=" << to
                             << " ra=" << this->coords_in.ra
                             << " dec=" << this->coords_in.dec
                             << " angle=" << this->coords_in.angle << " deg";
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pModule == nullptr ) {
      logwrite( function, "ERROR: Python module not imported" );
      return ERROR;
    }

    // Acquire the GIL
    //
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_FPOFFSETS_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python function not callable" );
      Py_XDECREF( pFunction );
      PyGILState_Release(gstate);
      return ERROR;
    }

    // Build up the PyObject argument list that will be passed to the function
    //
    PyObject* pArgList = Py_BuildValue( "(ssddd)", from.c_str(), to.c_str(),
                                        this->coords_in.ra,
                                        this->coords_in.dec,
                                        this->coords_in.angle
                                      );

    if ( !pArgList ) {
      logwrite( function, "ERROR: failed to build argument list" );
      Py_XDECREF( pFunction );
      PyGILState_Release(gstate);
      return ERROR;
    }

    // Call the Python function here
    //
    PyObject* pReturn = PyObject_CallObject( pFunction, pArgList );

    Py_XDECREF( pFunction );
    Py_DECREF( pArgList );

    if ( !pReturn || PyErr_Occurred() ) {
      logwrite( function, "ERROR calling Python function: " PYTHON_FPOFFSETS_FUNCTION );
      PyErr_Print();
      PyGILState_Release(gstate);
      return ERROR;
    }

    // Expected back a tuple
    //
    if ( !PyTuple_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not receive a tuple" );
      Py_DECREF( pReturn );
      PyGILState_Release(gstate);
      return ERROR;
    }

    int tuple_size = PyTuple_Size( pReturn );

    // Put each tuple item in its place
    //
    for ( int tuplen = 0; tuplen < tuple_size; tuplen++ ) {
      PyObject* pItem = PyTuple_GetItem( pReturn, tuplen );  // grab an item
      if ( PyFloat_Check( pItem ) ) {
        switch ( tuplen ) {
          case 0: this->coords_out.ra    = PyFloat_AsDouble( pItem );
                  break;
          case 1: this->coords_out.dec   = PyFloat_AsDouble( pItem );
                  break;
          case 2: this->coords_out.angle = PyFloat_AsDouble( pItem );
                  break;
          default:
            message.str(""); message << "ERROR unexpected tuple item " << tuplen << ": expected {0,1,2}";
            logwrite( function, message.str() );
            Py_DECREF( pReturn );
            PyGILState_Release( gstate );
            return ERROR;
        }
      }
      else {
        message.str(""); message << "ERROR tuple item " << tuplen << " not a float";
        logwrite( function, message.str() );
        Py_DECREF( pReturn );
        PyGILState_Release( gstate );
        return ERROR;
      }
    }

    Py_DECREF( pReturn );

    // Checking after extracting, because it may allow for partial extraction
    //
    if ( tuple_size != 3 ) {
      message.str(""); message << "ERROR unexpected 3 tuple items but received " << tuple_size;
      logwrite( function, message.str() );
      Py_DECREF( pReturn );
      PyGILState_Release( gstate );
      return ERROR;
    }

#ifdef LOGLEVEL_DEBUG
//  message.str(""); message << "[ACQUIRE] coords_out.ra=" << std::fixed << std::setprecision(6)
//                           << this->coords_out.ra << " .dec="
//                           << this->coords_out.dec << " .angle="
//                           << this->coords_out.angle << " deg";
//  logwrite( function, message.str() );
#endif

    PyGILState_Release( gstate );

    return NO_ERROR;
  }
  /***** SkyInfo::FPOffsets::compute_offset ***********************************/


  /***** SkyInfo::FPOffsets::solve_offset *************************************/
  /**
   * @brief      calculate offsets between ACAM and goal
   * @param[in]  ra_acam   current solution for ACAM RA
   * @param[in]  dec_acam  current solution for ACAM DEC
   * @param[in]  ra_goal   RA goal
   * @param[in]  dec_goal  DEC goal
   * @param[out] ra_off    reference to computed offset in RA to reach goal
   * @param[out] dec_off   reference to computed offset in DEC to reach goal
   * @return     ERROR or NO_ERROR
   *
   * @details    The TCS "PT" command moves the telescope along a great circle.
   *             Given a current position and a goal, this calls a Python
   *             function which computes the offsets needed to move the ACAM
   *             to the goal via the PT command.
   *
   */
  long FPOffsets::solve_offset( const double ra_acam, const double dec_acam, const double ra_goal, const double dec_goal,
                                double &ra_off, double &dec_off ) {
    std::string function = "SkyInfo::FPOffsets::solve_offset";
    std::stringstream message;

    // Check for valid inputs before passing something to Python that
    // it won't like.
    //
    if ( std::isnan( ra_acam )  ||
         std::isnan( dec_acam ) ||
         std::isnan( ra_goal )  ||
         std::isnan( dec_goal ) ) {
      logwrite( function, "ERROR one or more input values is NaN" );
      message.str(""); message << "ra_acam=" << ra_acam << " dec_acam=" << dec_acam
                               << " ra_goal=" << ra_goal << " dec_goal=" << dec_goal;
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pModule == nullptr ) {
      logwrite( function, "ERROR: Python module not imported" );
      return ERROR;
    }

    // Acquire the GIL
    //
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_SOLVEOFFSETDEG_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      message.str(""); message << "ERROR Python function " << PYTHON_SOLVEOFFSETDEG_FUNCTION << " not callable";;
      logwrite( function, message.str() );
      Py_XDECREF( pFunction );
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Build up the PyObject argument list that will be passed to the function
    //
    PyObject* pArgList = Py_BuildValue( "(dddd)", ra_acam, dec_acam, ra_goal, dec_goal );

    // Call the Python function here
    //
    PyObject* pReturn = PyObject_CallObject( pFunction, pArgList );

    Py_XDECREF( pFunction );
    Py_DECREF( pArgList );

    if ( !pReturn || PyErr_Occurred() ) {
      message.str(""); message << "ERROR calling Python function: " << PYTHON_SOLVEOFFSETDEG_FUNCTION;
      logwrite( function, message.str() );
      PyErr_Print();
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Expected back a tuple
    //
    if ( !PyTuple_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not receive a tuple" );
      Py_DECREF( pReturn );
      PyGILState_Release( gstate );
      return ERROR;
    }

    int tuple_size = PyTuple_Size( pReturn );

    // Put each tuple item in its place
    //
    for ( int tuplen = 0; tuplen < tuple_size; tuplen++ ) {
      PyObject* pItem = PyTuple_GetItem( pReturn, tuplen );  // grab an item
      if ( PyFloat_Check( pItem ) ) {
        switch ( tuplen ) {
          case 0: ra_off  = PyFloat_AsDouble( pItem );
                  break;
          case 1: dec_off = PyFloat_AsDouble( pItem );
                  break;
          default:
            message.str(""); message << "ERROR unexpected tuple item " << tuplen << ": expected {0,1}";
            logwrite( function, message.str() );
            Py_DECREF( pReturn );
            PyGILState_Release( gstate );
            return ERROR;
        }
      }
      else {
        message.str(""); message << "ERROR tuple item " << tuplen << " not a float";
        logwrite( function, message.str() );
        Py_DECREF( pReturn );
        PyGILState_Release( gstate );
        return ERROR;
      }
    }

    Py_DECREF( pReturn );
    PyGILState_Release( gstate );

    // Checking after extracting, because it may allow for partial extraction
    //
    if ( tuple_size != 2 ) {
      message.str(""); message << "ERROR expected 2 tuple items but received " << tuple_size;
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** SkyInfo::FPOffsets::solve_offset *************************************/


  /***** SkyInfo::FPOffsets::apply_offset *************************************/
  /**
   * @brief      calculate offsets to apply to the telescope
   * @details    The ra, dec arguments are modified, so they are also the outputs.
   *             All units are in degrees.
   * @param[out] ra       reference to current RA is modified by ra_off
   * @param[in]  ra_off   RA offset in deg
   * @param[out] dec      reference to current DEC is modified by dec_off
   * @param[in]  dec_off  DEC offset in deg
   * @return     ERROR | NO_ERROR
   *
   */
  long FPOffsets::apply_offset( double &ra,  const double ra_off,
                                double &dec, const double dec_off ) {
    std::string function = "SkyInfo::FPOffsets::apply_offset";
    std::stringstream message;

    // Check for valid inputs before passing something to Python that
    // it won't like.
    //
    if ( std::isnan( ra    )   ||
         std::isnan( dec    )  ||
         std::isnan( ra_off )  ||
         std::isnan( dec_off ) ) {
      logwrite( function, "ERROR one or more input values is NaN" );
      message.str(""); message << "ra=" << ra << " dec=" << dec
                               << " ra_off=" << ra_off << " dec_off=" << dec_off;
      logwrite( function, message.str() );
      return ERROR;
    }

    // Nothing to do if both offsets are zero so get out now.
    //
    if ( ra_off == 0.0 && dec_off == 0.0 ) return NO_ERROR;

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pModule==NULL ) {
      logwrite( function, "ERROR: Python module not imported" );
      return ERROR;
    }

    // Acquire the GIL
    //
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_APPLYOFFSETDEG_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      message.str(""); message << "ERROR Python function " << PYTHON_APPLYOFFSETDEG_FUNCTION << " not callable";;
      logwrite( function, message.str() );
      Py_XDECREF( pFunction );
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Build up the PyObject argument list that will be passed to the function
    //
    PyObject* pArgList = Py_BuildValue( "(dddd)", ra, dec, ra_off, dec_off );

    // Call the Python function here
    //
    PyObject* pReturn = PyObject_CallObject( pFunction, pArgList );

    Py_XDECREF( pFunction );
    Py_DECREF( pArgList );

    if ( !pReturn || PyErr_Occurred() ) {
      message.str(""); message << "ERROR calling Python function: " << PYTHON_APPLYOFFSETDEG_FUNCTION;
      logwrite( function, message.str() );
      PyErr_Print();
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Expected back a tuple
    //
    if ( !PyTuple_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not receive a tuple" );
      Py_DECREF( pReturn );
      PyGILState_Release( gstate );
      return ERROR;
    }

    int tuple_size = PyTuple_Size( pReturn );

    // Put each tuple item in its place
    //
    for ( int tuplen = 0; tuplen < tuple_size; tuplen++ ) {
      PyObject* pItem = PyTuple_GetItem( pReturn, tuplen );  // grab an item
      if ( PyFloat_Check( pItem ) ) {
        switch ( tuplen ) {
          case 0: ra  = PyFloat_AsDouble( pItem );
                  break;
          case 1: dec = PyFloat_AsDouble( pItem );
                  break;
          default:
            message.str(""); message << "ERROR unexpected tuple item " << tuplen << ": expected {0,1}";
            logwrite( function, message.str() );
            Py_DECREF( pReturn );
            PyGILState_Release( gstate );
            return ERROR;
        }
      }
    }

    Py_DECREF( pReturn );
    PyGILState_Release( gstate );

    // Checking after extracting, because it may allow for partial extraction
    //
    if ( tuple_size != 2 ) {
      message.str(""); message << "ERROR expected 2 tuple items but received " << tuple_size;
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** SkyInfo::FPOffsets::apply_offset *************************************/


  /***** SkyInfo::FPOffsets::get_acam_params **********************************/
  /**
   * @brief      get parameters needed for acam image headers
   * @details    This calls Python function getiAcamParams in the FPOffsets
   *             module and stores the results in the class. The call needs no
   *             arguments and returns a dictionary of the form:
   *'''
   *             { "PIXSCALE"  : pixscale,       # arcsec/px
   *               "CDELT1"    : pixscale/3600., # deg/px
   *               "CDELT2"    : pixscale/3600., # deg/px
   *               "CRPIX1"    : 512,            # ACAM center
   *               "CRPIX2"    : 512 }
   *'''
   * @return     ERROR | NO_ERROR
   *
   */
  long FPOffsets::get_acam_params() {
    std::string function = "SkyInfo::FPOffsets::get_acam_params";

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pModule==NULL ) {
      logwrite( function, "ERROR: Python module not imported" );
      return ERROR;
    }

    // Acquire the GIL
    //
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_GETACAMPARAMS_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR Python function "+std::string(PYTHON_GETACAMPARAMS_FUNCTION)+" not callable" );
      Py_XDECREF( pFunction );
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Call the Python function here
    //
    PyObject* pReturn = PyObject_CallObject( pFunction, NULL );
    Py_XDECREF( pFunction );

    if ( !pReturn || PyErr_Occurred() ) {
      logwrite( function, "ERROR calling Python function: "+std::string(PYTHON_GETACAMPARAMS_FUNCTION) );
      PyErr_Print();
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Expect a dictionary
    //
    if ( !PyDict_Check( pReturn ) ) {
      logwrite( function, "ERROR Python function "+std::string(PYTHON_GETACAMPARAMS_FUNCTION)+" did not return expected dictionary" );
      Py_DECREF( pReturn );
      PyGILState_Release(gstate);
      return ERROR;
    }

    // Now extract the named values from this sub-dictionary,
    // and store them in a class map indexed by camera name.
    //
    // extract the values
    PyObject* pPIXSCALE = PyDict_GetItemString( pReturn, "PIXSCALE" );
    PyObject* pCDELT1   = PyDict_GetItemString( pReturn, "CDELT1" );
    PyObject* pCDELT2   = PyDict_GetItemString( pReturn, "CDELT2" );
    PyObject* pCRPIX1   = PyDict_GetItemString( pReturn, "CRPIX1" );
    PyObject* pCRPIX2   = PyDict_GetItemString( pReturn, "CRPIX2" );

    // store them in acamparams struct
    if (pPIXSCALE && PyFloat_Check(pPIXSCALE)) this->acamparams.pixscale = PyFloat_AsDouble( pPIXSCALE );
    if (pCDELT1   && PyFloat_Check(pCDELT1))   this->acamparams.cdelt1   = PyFloat_AsDouble( pCDELT1 );
    if (pCDELT2   && PyFloat_Check(pCDELT2))   this->acamparams.cdelt2   = PyFloat_AsDouble( pCDELT2 );
    if (pCRPIX1   && PyFloat_Check(pCRPIX1))   this->acamparams.crpix1   = PyFloat_AsDouble( pCRPIX1 );
    if (pCRPIX2   && PyFloat_Check(pCRPIX2))   this->acamparams.crpix2   = PyFloat_AsDouble( pCRPIX2 );

    Py_DECREF( pReturn );
    PyGILState_Release( gstate );

    return NO_ERROR;
  }
  /***** SkyInfo::FPOffsets::get_acam_params **********************************/


  /***** SkyInfo::FPOffsets::get_slicecam_params ******************************/
  /**
   * @brief      get parameters needed for slicecam image headers
   * @details    This calls Python function getSlicevParams in the FPOffsets
   *             module and stores the results in the class. The call needs no
   *             arguments and returns a nested dictionary of the form:
   *'''
   *             "L":{ "CDELT1"  : 1.667E-05,
   *                   "CDELT2"  : 1.667E-05,
   *                   "CRPIX1"  : 1025,
   *                   "CRPIX2"  : 512,
   *                   "THETADEG" : 0 },
   *             "R":{ "CDELT1"  : 1.667E-05,
   *                   "CDELT2"  : 1.667E-05,
   *                   "CRPIX1"  : -1,
   *                   "CRPIX2"  : 512,
   *                   "THETADEG" : 0 }
   *'''
   * @return     ERROR | NO_ERROR
   *
   */
  long FPOffsets::get_slicecam_params() {
    std::string function = "SkyInfo::FPOffsets::get_slicecam_params";
    std::stringstream message;

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pModule==NULL ) {
      logwrite( function, "ERROR: Python module not imported" );
      return ERROR;
    }

    // Acquire the GIL
    //
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* pFunction = PyObject_GetAttrString( this->pModule, PYTHON_GETSLICEPARAMS_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      message.str(""); message << "ERROR Python function " << PYTHON_GETSLICEPARAMS_FUNCTION << " not callable";;
      logwrite( function, message.str() );
      Py_XDECREF( pFunction );
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Call the Python function here
    //
    PyObject* pReturn = PyObject_CallObject( pFunction, NULL );
    Py_XDECREF( pFunction );

    if ( !pReturn || PyErr_Occurred() ) {
      message.str(""); message << "ERROR calling Python function: " << PYTHON_GETSLICEPARAMS_FUNCTION;
      logwrite( function, message.str() );
      PyErr_Print();
      PyGILState_Release( gstate );
      return ERROR;
    }

    // Expect a dictionary
    //
    if ( !PyDict_Check( pReturn ) ) {
      logwrite( function, "ERROR Python function did not return expected dictionary" );
      Py_DECREF( pReturn );
      PyGILState_Release(gstate);
      return ERROR;
    }

    // vector of camera names, suitable for PyDict_GetItemString
    //
    std::vector<const char*> camera_names = { "L", "R" };

    // Get the values out of the nested dictionary
    //
    for ( const auto &cam : camera_names ) {                   // loop over the camera names

      PyObject* pDict = PyDict_GetItemString( pReturn, cam );  // dictionary for this camera

      // Now extract the named values from this sub-dictionary,
      // and store them in a class map indexed by camera name.
      //
      if ( pDict && PyDict_Check(pDict) ) {
        // extract the values
        PyObject* pPIXSCALE = PyDict_GetItemString( pDict, "PIXSCALE" );
        PyObject* pCDELT1   = PyDict_GetItemString( pDict, "CDELT1" );
        PyObject* pCDELT2   = PyDict_GetItemString( pDict, "CDELT2" );
        PyObject* pCRPIX1   = PyDict_GetItemString( pDict, "CRPIX1" );
        PyObject* pCRPIX2   = PyDict_GetItemString( pDict, "CRPIX2" );
        PyObject* pTHETADEG = PyDict_GetItemString( pDict, "THETADEG" );
        PyObject* pDATASEC  = PyDict_GetItemString( pDict, "DATASEC" );

        // store them in sliceparams map
        if ( pPIXSCALE && PyFloat_Check(pPIXSCALE))  this->sliceparams[std::string(cam)].pixscale = PyFloat_AsDouble( pPIXSCALE );
        if ( pCDELT1   && PyFloat_Check(pCDELT1) )   this->sliceparams[std::string(cam)].cdelt1   = PyFloat_AsDouble( pCDELT1 );
        if ( pCDELT2   && PyFloat_Check(pCDELT2) )   this->sliceparams[std::string(cam)].cdelt2   = PyFloat_AsDouble( pCDELT2 );
        if ( pCRPIX1   && PyFloat_Check(pCRPIX1) )   this->sliceparams[std::string(cam)].crpix1   = PyFloat_AsDouble( pCRPIX1 );
        if ( pCRPIX2   && PyFloat_Check(pCRPIX2) )   this->sliceparams[std::string(cam)].crpix2   = PyFloat_AsDouble( pCRPIX2 );
        if ( pTHETADEG && PyFloat_Check(pTHETADEG) ) this->sliceparams[std::string(cam)].thetadeg = PyFloat_AsDouble( pTHETADEG );
        if ( pDATASEC  ) {
          const char* cDATASEC = PyUnicode_AsUTF8( pDATASEC );
          this->sliceparams[std::string(cam)].datasec.assign( cDATASEC );
        }
      }
    }

    Py_DECREF( pReturn );
    PyGILState_Release( gstate );

    return NO_ERROR;
  }
  /***** SkyInfo::FPOffsets::get_slicecam_params ******************************/

}
