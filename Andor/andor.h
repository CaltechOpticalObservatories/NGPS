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
      int temp;
      std::string temp_status;
      std::string modestr;
      int rows;
      int cols;
      int axes[2];
      int datatype;
      unsigned long npix;
      unsigned long section_size;
      double exposure_time;      ///< exposure time in seconds
      std::string fits_name;

      Information();
  };
  /***** Information **********************************************************/


  /***** FITS_file ************************************************************/
  /**
   * @class   FITS_file
   * @brief   class for FITS I/O using CCfits
   *
   */
  class FITS_file {
    private:
      std::unique_ptr<CCfits::FITS> pFits;          ///< pointer to FITS data container
      std::string fits_name;

    public:
      FITS_file();
      FITS_file(Information info);
      ~FITS_file();

      Information info;

      long open_file();
      void close_file();
      long copy_header( std::string wcs_in );
      template <typename T> long write_image( T* data );
  };
  /***** FITS_file ************************************************************/


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

      FITS_file fits_file;

      uint16_t* image_data;

      // The following functions wrap Andor SDK functions,
      // indicated by "_" prefix. What the wrappers provide is a check
      // of the return value. If not DRV_SUCCESS then the return value
      // is logged (and the wrappper returns ERROR). On DRV_SUCCESS the
      // wrapper returns NO_ERROR.
      //
      long _GetAcquiredData16( uint16_t* buf, unsigned long bufsize );
      long _GetAvailableCameras( int* number );
      long _GetCameraHandle( int index, int* handle );
      long _GetCameraSerialNumber( int* number );
      long _GetDetector( int* xpix, int* ypix );
      long _GetStatus( int* status );
      long _GetStatus( int* status, std::stringstream &status_msg );
      long _GetTemperature();
      long _Initialize( );
      long _SetAcquisitionMode( int mode );
      long _SetCurrentCamera( int handle );
      long _SetExposureTime( double exptime );
      long _SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend );
      long _SetReadMode( int mode );
      long _SetShutter( int type, int mode, int closetime, int opentime );
      long _StartAcquisition( );

      inline bool is_initialized() { return this->initialized; };

      long open( std::string args );
      long close();
      long test();
      long shutter();
      long exptime( std::string exptime_in, std::string &retstring );
      long acquire_one();
      long save_acquired( std::string wcs_in, std::string &imgname );
      unsigned int start_acquisition();
      long log_status();

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
          ret = GetAcquiredData16( buf, this->camera_info.npix );
        }
        else {
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
