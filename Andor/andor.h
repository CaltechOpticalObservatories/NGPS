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
#include <string>
#include "common.h"
#include "network.h"
#include "utilities.h"
#include "atmcdLXd.h"
#include "andor_emulator.h"

#define ANDOR_SDK "/usr/local/etc/andor"

/***** Andor ******************************************************************/
/**
 * @namespace  Andor
 * @brief      namespace for Andor cameras
 *
 */
namespace Andor {

  const int AMPTYPE_EMCCD = 0;
  const int AMPTYPE_CONV  = 1;

  /***** Andor::Information ***************************************************/
  /**
   * @class     Information
   * @brief     information class for the Andor CCD
   * @details   contains information about the Andor camera
   *
   */
  class Information {
    private:
    public:
      int serial_number;
      int acqmode;                                 ///< acquisition mode
      std::string acqmodestr;
      int readmode;                                ///< read mode
      std::string readmodestr;
      int adchan;                                  ///< selected AD channel
      int adchans;                                 ///< number of AD channels available
      int setpoint;                                ///< temperature setpoint
      int ccdtemp;                                 ///< ccd temperature
      int temp_min;                                ///< minimum allowed temperature
      int temp_max;                                ///< maximum allowed temperature
      int amptype;                                 ///< amp type 0=EM 1=conventional
      std::string amptypestr;                      ///< amp type string
      std::map<int, std::vector<float>> hsspeeds;  ///< vector of hori shift speeds for each amp type
      std::vector<float> vsspeeds;                 ///< vector of vert shift speeds
      float hspeed;                                ///< horizontal pixel speed
      float vspeed;                                ///< vertical pixel speed
      int hbin;                                    ///< horizontal binning
      int vbin;                                    ///< vertical binning
      int hstart;                                  ///< horizontal start pixel
      int vstart;                                  ///< vertical start pixel
      int hend;                                    ///< horizontal end pixel
      int vend;                                    ///< vertical end pixel
      int gain;                                    ///< currently selected gain
      int emgain;                                  ///< current EM gain (doesn't mean EM is selected)
      int emgain_low;                              ///< EM gain low
      int emgain_high;                             ///< EM gain high
      int hflip;                                   ///< flipped horizontal? 1=yes 0=no
      int vflip;                                   ///< flipped vertical? 1=yes 0=no
      std::string temp_status;
      int rows;
      int cols;
      int axes[2];
      int datatype;
      unsigned long npix;
      unsigned long section_size;
      double pixel_scale;                          ///< pixel scale
      double exposure_time;                        ///< exposure time in seconds
      std::string fits_name;
      std::string sdk_version;                     ///< SDK version
      std::string driver_version;                  ///< device driver version
      std::string timestring;
      double mjd0;

      Information();
  };
  /***** Andor::Information ***************************************************/


  /***** Andor::FITS_file *****************************************************/
  /**
   * @class     FITS_file
   * @brief     class for FITS I/O using CCfits
   *
   */
  class FITS_file {
    private:
      std::unique_ptr<CCfits::FITS> pFits;          ///< pointer to FITS data container
      std::string fits_name;

    public:
      FITS_file() { };
      FITS_file(Information info);                  ///< instantiated with existing info object

      Information info;

      long open_file();
      void close_file();
      long create_header();
      long copy_header( std::string wcs_in );
      template <typename T> long write_image( T* data );
      template <typename T> long swrite_image( std::unique_ptr<T[]> &data ) {
        std::string function = "Andor::FITS_file::write_image";
        std::stringstream message;

        // Set the FITS system to verbose mode so it writes error messages
        //
        CCfits::FITS::setVerboseMode( true );

        // write the primary image into the FITS file
        //
        try {
          std::valarray<T> array( this->info.section_size );
          for ( unsigned long i=0; i < this->info.section_size; i++ ) array[i] = data[i];
          long fpixel(1);        // start with the first pixel always
          this->pFits->pHDU().write( fpixel, this->info.section_size, array );
          this->pFits->flush();  // make sure the image is written to disk
        }
        catch ( CCfits::FitsError& error ) {
          message.str(""); message << "FITS file error thrown: " << error.message();
          logwrite(function, message.str());
          return ERROR;
        }

        return NO_ERROR;
      }

  };
  /***** Andor::FITS_file *****************************************************/


  /***** Andor::AndorBase *****************************************************/
  /**
   * @class     AndorBase
   * @brief     Base class for the Andor CCD Software Development Kit
   * @details   This class contains wrappers for all the SDK functions,
   *            indicated by "_" prefix. What the wrappers provide is a check
   *            of the return value. If not DRV_SUCCESS then the return value
   *            is logged (and the wrappper returns ERROR). On DRV_SUCCESS the
   *            wrapper returns NO_ERROR.
   */
  class AndorBase {
    public:
      virtual ~AndorBase() {}
      virtual long _GetAcquiredData16( uint16_t* buf, unsigned long bufsize ) = 0;
      virtual long _GetAvailableCameras( int &number ) = 0;
      virtual long _GetCameraHandle( int index, int* handle ) = 0;
      virtual long _GetCameraSerialNumber( int &number ) = 0;
      virtual long _GetDetector( int &xpix, int &ypix ) = 0;
      virtual long _GetStatus( std::string &status ) = 0;
      virtual long _GetStatus( int &status_id ) = 0;
      virtual long _GetStatus( int &status_id, std::string &status_msg ) = 0;
      virtual long _GetNumberADChannels( int &channels ) = 0;
      virtual long _GetNumberHSSpeeds( int chan, int type, int &speeds ) = 0;
      virtual long _GetNumberVSSpeeds( int &speeds ) = 0;
      virtual long _GetHSSpeed( int chan, int type, int index, float &speed ) = 0;
      virtual long _SetHSSpeed( int type, int index ) = 0;
      virtual long _GetVSSpeed( int index, float &speed ) = 0;
      virtual long _SetVSSpeed( int index ) = 0;
      virtual long _SetEMCCDGain( int gain ) = 0;
      virtual long _GetEMCCDGain( int &gain ) = 0;
      virtual long _GetEMGainRange( int &low, int &high ) = 0;
      virtual long _SetOutputAmplifier( int type ) = 0;
      virtual long _GetTemperature( int &temp, std::string_view &status ) = 0;
      virtual long _GetTemperatureRange( int &min, int &max ) = 0;
      virtual long _CoolerON() = 0;
      virtual long _CoolerOFF() = 0;
      virtual long _SetTemperature( int temp ) = 0;
      virtual long _GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) = 0;
      virtual long _Initialize( ) = 0;
      virtual long _SetAcquisitionMode( int mode ) = 0;
      virtual long _SetCurrentCamera( int handle ) = 0;
      virtual long _SetExposureTime( double exptime ) = 0;
      virtual long _SetImageFlip( int hflip, int vflip ) = 0;
      virtual long _SetImageRotate( int rotdir ) = 0;
      virtual long _SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) = 0;
      virtual long _SetReadMode( int mode ) = 0;
      virtual long _SetShutter( int type, int mode, int closetime, int opentime ) = 0;
      virtual long _StartAcquisition( ) = 0;
  };
  /***** Andor::AndorBase *****************************************************/


  /***** Andor::SDK ***********************************************************/
  /**
   * @class     SDK
   * @brief     Derived class for operating the Andor CCD Software Development Kit
   * @details   This class inherits from AndorBase.
   *
   */
  class SDK : public AndorBase {
    private:
      bool initialized;
    public:
      long _GetAcquiredData16( uint16_t* buf, unsigned long bufsize ) override;
      long _GetAvailableCameras( int &number ) override;
      long _GetCameraHandle( int index, int* handle ) override;
      long _GetCameraSerialNumber( int &number ) override;
      long _GetDetector( int &xpix, int &ypix ) override;
      long _GetStatus( std::string &status ) override;
      long _GetStatus( int &status_id ) override;
      long _GetStatus( int &status_id, std::string &status_msg ) override;
      long _GetNumberADChannels( int &channels ) override;
      long _GetNumberHSSpeeds( int chan, int type, int &speeds ) override;
      long _GetNumberVSSpeeds( int &speeds ) override;
      long _GetHSSpeed( int chan, int type, int index, float &speed ) override;
      long _SetHSSpeed( int type, int index ) override;
      long _GetVSSpeed( int index, float &speed ) override;
      long _SetVSSpeed( int index ) override;
      long _SetEMCCDGain( int gain ) override;
      long _GetEMCCDGain( int &gain ) override;
      long _GetEMGainRange( int &low, int &high ) override;
      long _SetOutputAmplifier( int type ) override;
      long _GetTemperature( int &temp, std::string_view &status ) override;
      long _GetTemperatureRange( int &min, int &max ) override;
      long _CoolerON() override;
      long _CoolerOFF() override;
      long _SetTemperature( int temp ) override;
      long _GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) override;
      long _Initialize( ) override;
      long _SetAcquisitionMode( int mode ) override;
      long _SetCurrentCamera( int handle ) override;
      long _SetExposureTime( double exptime ) override;
      long _SetImageFlip( int hflip, int vflip ) override;
      long _SetImageRotate( int rotdir ) override;
      long _SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) override;
      long _SetReadMode( int mode ) override;
      long _SetShutter( int type, int mode, int closetime, int opentime ) override;
      long _StartAcquisition( ) override;
  };
  /***** Andor::SDK ***********************************************************/


  /***** Andor::Emulator ******************************************************/
  /**
   * @class     Emulator
   * @brief     Derived class for emulating the Andor CCD Software Development Kit
   * @details   This class inherits from AndorBase.
   *
   */
  class Emulator : public AndorBase {
    private:
      bool initialized;
      double exptime;

    public:
      Emulator() : initialized(false), exptime(0) { }

      inline double get_exptime() { return this->exptime; }

      long _GetAcquiredData16( uint16_t* buf, unsigned long bufsize ) override;
      long _GetAvailableCameras( int &number ) override;
      long _GetCameraHandle( int index, int* handle ) override;
      long _GetCameraSerialNumber( int &number ) override;
      long _GetDetector( int &xpix, int &ypix ) override;
      long _GetStatus( std::string &status ) override;
      long _GetStatus( int &status_id ) override;
      long _GetStatus( int &status_id, std::string &status_msg ) override;
      long _GetNumberADChannels( int &channels ) override;
      long _GetNumberHSSpeeds( int chan, int type, int &speeds ) override;
      long _GetNumberVSSpeeds( int &speeds ) override;
      long _GetHSSpeed( int chan, int type, int index, float &speed ) override;
      long _SetHSSpeed( int type, int index ) override;
      long _GetVSSpeed( int index, float &speed ) override;
      long _SetVSSpeed( int index ) override;
      long _SetEMCCDGain( int gain ) override;
      long _GetEMCCDGain( int &gain ) override;
      long _GetEMGainRange( int &low, int &high ) override;
      long _SetOutputAmplifier( int type ) override;
      long _GetTemperature( int &temp, std::string_view &status ) override;
      long _GetTemperatureRange( int &min, int &max ) override;
      long _CoolerON() override;
      long _CoolerOFF() override;
      long _SetTemperature( int temp ) override;
      long _GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) override;
      long _Initialize( ) override;
      long _SetAcquisitionMode( int mode ) override;
      long _SetCurrentCamera( int handle ) override;
      long _SetExposureTime( double exptime ) override;
      long _SetImageFlip( int hflip, int vflip ) override;
      long _SetImageRotate( int rotdir ) override;
      long _SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) override;
      long _SetReadMode( int mode ) override;
      long _SetShutter( int type, int mode, int closetime, int opentime ) override;
      long _StartAcquisition( ) override;

      SkySim skysim;
  };
  /***** Andor::Emulator ******************************************************/


  /***** Andor::Interface *****************************************************/
  /**
   * @class     Interface
   * @brief     class for control of the Andor CCD using the SDK wrappers
   * @details   Control software for ths CCD, does all of the driver control
   *            functions for the Andor library (open, close CCD driver),
   *            communicates with the CCD, etc.
   *
   */
  class Interface {
    private:
      bool initialized;           ///< is the Andor SDK initialized?
      int  serial;                ///< serial number to use
      bool andor_emulated;        ///< is the Andor emulator in use?
      Andor::AndorBase* andor;    ///< pointer to the Andor class to use
//    std::unique_ptr<uint16_t[]> image_data;
      uint16_t* image_data;

    public:
      /***** Andor::Interface::Interface **************************************/
      /**
       * @brief      default Interface constructor
       *
       */
      Interface() : initialized( false ), serial( -1 ), andor_emulated( false ), andor( &sdk ), image_data( nullptr ) { }
      /***** Andor::Interface::Interface **************************************/

      /***** Andor::Interface::Interface **************************************/
      /**
       * @brief      Interface constructor accepts serial number
       *
       */
      Interface( int sn ) : initialized( false ), serial( sn ), andor_emulated( false ), andor( &sdk ), image_data( nullptr ) { }
      /***** Andor::Interface::Interface **************************************/

      Andor::SDK sdk;             ///< object for the real Andor SDK

      Andor::Emulator emulator;   ///< object for the Andor emulator

      /***** Andor::Interface::andor_emulator *********************************/
      /**
       * @brief      enable/disable Andor emulator
       * @param[in]  emulate  true=use emulator, false=use real Andor SDK
       *
       */
      inline void andor_emulator( bool emulate ) {
        this->andor = emulate ? static_cast<Andor::AndorBase*>(&emulator) : static_cast<Andor::AndorBase*>(&sdk);
        this->andor_emulated = emulate;
        return;
      }
      /***** Andor::Interface::andor_emulator *********************************/

      /***** Andor::Interface::get_andor_object *******************************/
      /**
       * @brief      returns string indicating which object is in use
       * @return     "emulator" or "sdk" or "null"
       *
       */
      inline std::string_view get_andor_object() {
        if ( this->andor == &emulator ) return "emulator";
        else
        if ( this->andor == &sdk ) return "sdk";
        else
        return "null";
      }
      /***** Andor::Interface::get_andor_object *******************************/

      Andor::Information camera_info;

      FITS_file fits_file;

//    std::unique_ptr<uint16_t[]> get_image_data() { return std::move( this->image_data ); }
//    std::unique_ptr<uint16_t[]>& get_image_data() { return this->image_data; }
      inline uint16_t* get_image_data() { return this->image_data; }

      inline bool is_emulated() { return this->andor_emulated; }

      inline bool is_initialized() { return this->initialized; };

      long simulate_frame( std::string name_in );

      long simandor( std::string args, std::string &retstring );
      long open( std::string args );
      long close();
      long test();
      long shutter();
      long set_exptime( int exptime );
      long set_exptime( std::string exptime, std::string &retstring );
      long acquire_one();
      long save_acquired( std::string wcs_in, std::string &imgname );
      unsigned int start_acquisition();
      long get_detector( int &x, int &y );
      long get_status();
      long get_speeds();
      long set_temperature( int temp );
      long get_temperature( int &temp );
      long get_temperature();
      long set_read_mode( int mode );
      long set_acquisition_mode( int mode );
      long set_output_amplifier( int type );
      long set_emgain( int gain );
      long get_emgain_range( int &low, int &high );
      long get_emgain( int &gain );
      long set_hsspeed( float speed );
      long set_vsspeed( float speed );
      long set_image( int hbin, int vbin, int hstart, int hend, int vstart, int vend );
      long set_binning( int hbin, int vbin );
      long set_imflip( int hflip, int vflip );
      long set_imrot( int rotdir );

      template <typename T>
      unsigned int get_last_frame( T* buf ) {
        std::string function = "Andor::Interface::get_last_frame";
        std::stringstream message;
        unsigned long ret;

        this->get_detector( this->camera_info.cols, this->camera_info.rows );
        this->camera_info.npix = this->camera_info.cols * this->camera_info.rows;

        // Use the appropriate API call to get the acquired data
        // according to the type of pointer passed in.
        //
        if constexpr ( std::is_same_v<T, int32_t> ) {
logwrite(function, "int32_t");
          ret = GetAcquiredData( buf, this->camera_info.npix );
        }
        else
        if constexpr ( std::is_same_v<T, uint16_t> ) {
logwrite(function, "uint16_t");
          ret = GetAcquiredData16( buf, this->camera_info.npix );
        }
        else {
          message.str(""); message << "Unknown type: " << typeid(T).name();
          logwrite(function, message.str());
          message.str("");
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
  /***** Andor::Interface *****************************************************/

}
/***** Andor ******************************************************************/
#endif
