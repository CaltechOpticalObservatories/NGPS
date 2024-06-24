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

    this->pModuleName = PyUnicode_FromString( PYTHON_FPOFFSETS_MODULE );
    this->pModule     = PyImport_Import( this->pModuleName );
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

//#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[ACQUIRE] from=" << from << " to=" << to
                             << " coords_in.ra=" << this->coords_in.ra
                             << " .dec=" << this->coords_in.dec
                             << " .angle=" << this->coords_in.angle << " deg";
    logwrite( function, message.str() );
//#endif

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
    PyObject* pReturn = PyObject_CallObject( pFunction, pArgList );

    Py_XDECREF( pFunction );
    Py_DECREF( pArgList );

    if ( !pReturn || PyErr_Occurred() ) {
      message.str(""); message << "ERROR calling Python function: " << PYTHON_FPOFFSETS_FUNCTION;
      logwrite( function, message.str() );
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
      PyGILState_Release( gstate );
      return ERROR;
    }

//#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[ACQUIRE] coords_out.ra=" << this->coords_out.ra
                             << " .dec=" << this->coords_out.dec
                             << " .angle=" << this->coords_out.angle << " deg";
    logwrite( function, message.str() );
//#endif

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
  long FPOffsets::solve_offset( const double ra_acam, const double dec_acam, const double ra_goal, const double dec_goal,
                                double &ra_off, double &dec_off ) {
    std::string function = "SkyInfo::FPOffsets::solve_offset";
    std::stringstream message;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] ra_acam=" << ra_acam << " dec_acam=" << dec_acam << " ra_goal=" << ra_goal << " dec_goal=" << dec_goal;
    logwrite( function, message.str() );
#endif

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

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] ra_off=" << ra_off << " dec_off=" << dec_off;
    logwrite( function, message.str() );
#endif

    return NO_ERROR;
  }
  /***** SkyInfo::FPOffsets::solve_offset *************************************/

}
