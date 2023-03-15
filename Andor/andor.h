/**
 * @file    andor.h
 * @brief   this file contains the definitions for the Andor interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef ANDOR_H
#define ANDOR_H

#include <CCfits/CCfits>           //!< needed here for types in set_axes()
#include <type_traits>
#include "common.h"
#include "network.h"
#include "utilities.h"
#include "atmcdLXd.h"

#define ANDOR_SDK "/usr/local/etc/andor"

/***** Andor ******************************************************************/
/**
 * @namespace Andor
 * @brief     namespace for Andor cameras
 *
 */
namespace Andor {

  /***** Information **********************************************************/
  /**
   * @class   Information
   * @brief   information class for the Andor CCD
   * @details contains information about the Andor camera
   *
   */
  class Information {
    private:
    public:
      int serial_number;
      int mode;
      int rows;
      int cols;
      unsigned long npix;
      double exposure_time;      ///< exposure time in seconds

      Information();
  };
  /***** Information **********************************************************/


  /***** Interface ************************************************************/
  /**
   * @class   Interface
   * @brief   class for control of the Andor CCD
   * @details Control software for ths CCD, does all of the driver control
   *          functions for the Andor library (open, close CCD driver),
   *          communicates with the CCD, etc.
   *
   */
  class Interface {
    private:
      char* sdk;
      bool initialized;
    public:

      Interface();
      ~Interface();

      Andor::Information camera_info;

      bool is_initialized() { return this->initialized; };
      long open();
      long close();
      long test();
      long exptime( std::string exptime_in, std::string &retstring );
      long acquire_one();
      unsigned int start_acquisition();
      std::string status_string( int status );
      long get_status( int &status );
      unsigned int get_status();

      template <typename T>
      unsigned int get_last_frame( T* buf ) {
        std::string function = "Andor::Interface::get_last_frame";
        std::stringstream message;
        unsigned long ret;

        GetDetector( &this->camera_info.cols, &this->camera_info.rows );
        this->camera_info.npix = this->camera_info.cols * this->camera_info.rows;

        // Use the appropriate API call to get the acquired data
        // according to the type of pointer passed in.
        //
        if constexpr ( std::is_same_v<T, int32_t*> ) {
          ret = GetAcquiredData( buf, this->camera_info.npix );
        }
        else
        if constexpr ( std::is_same_v<T, uint16_t*> ) {
std::cerr << "ok\n";
          ret = GetAcquiredData16( buf, this->camera_info.npix );
        }
        else {
std::cerr << "nope\n";
          ret = DRV_DATATYPE;
        }
        switch ( ret ) {
          case DRV_SUCCESS:
            message << "data acquired";
            break;
          case DRV_NOT_INITIALIZED:
            message << "ERROR not initialized";
            break;
          case DRV_ACQUIRING:
            message << "ERROR acquisition in progress";
            break;
          case DRV_ERROR_ACK:
            message << "ERROR unable to communicate with device";
            break;
          case DRV_P1INVALID:
            message << "ERROR invalid buffer pointer";
            break;
          case DRV_P2INVALID:
            message << "ERROR buffer size incorrect";
            break;
          case DRV_NO_NEW_DATA:
            message << "ERROR no acquisition";
            break;
          case DRV_DATATYPE:
            message << "ERROR unrecognized datatype";
            break;
          default:
            message << "ERROR unrecognized return code " << ret;
        }

        logwrite( function, message.str() );
        return ret;
      }




  };
  /***** Interface ************************************************************/

}
/***** Andor ******************************************************************/
#endif
