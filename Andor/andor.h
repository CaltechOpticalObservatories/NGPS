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
 * @namespace  Andor
 * @brief      namespace for Andor cameras
 *
 */
namespace Andor {

  const int AMPTYPE_EMCCD = 0;
  const int AMPTYPE_CONV  = 1;

  /***** Information **********************************************************/
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
      double exposure_time;      ///< exposure time in seconds
      std::string fits_name;
      std::string sdk_version;     ///< SDK version
      std::string driver_version;  ///< device driver version

      Information();
  };
  /***** Information **********************************************************/


  /***** FITS_file ************************************************************/
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
      FITS_file();
      FITS_file(Information info);
      ~FITS_file();

      Information info;

      long open_file();
      void close_file();
      long create_header();
      long copy_header( std::string wcs_in );
      template <typename T> long write_image( T* data );
  };
  /***** FITS_file ************************************************************/


  /***** AndorBase ************************************************************/
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
  /***** AndorBase ************************************************************/


  /***** SDK ******************************************************************/
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
  /***** SDK ******************************************************************/


  /***** Sim ******************************************************************/
  /**
   * @class     Sim
   * @brief     Derived class for simulating the Andor CCD Software Development Kit
   * @details   This class inherits from AndorBase.
   *
   */
  class Sim : public AndorBase {
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
  /***** Sim ******************************************************************/


  /***** Interface ************************************************************/
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
      bool andor_simulated;       ///< is the Andor simulated?
      Andor::AndorBase* andor;    ///< pointer to the Andor class to use

    public:
      /***** Andor::Interface::Interface **************************************/
      /**
       * @brief      Interface constructor, defaults to Andor not simulated
       *
       */
      Interface() : initialized( false ), andor_simulated( false ), andor( &sdk ) {
        image_data = nullptr;
      }
      /***** Andor::Interface::Interface **************************************/

      Andor::SDK sdk;             ///< object for the real Andor SDK

      Andor::Sim sim;             ///< object for the simulated Andor

      /***** Andor::Interface::sim_andor **************************************/
      /**
       * @brief      enable/disable simulating the Andor
       * @param[in]  simandor  true=use simulator, false=use real Andor SDK
       * @details    "sim" or "sdk" or "null"
       *
       */
      inline void sim_andor( bool simandor ) {
        this->andor = simandor ? static_cast<Andor::AndorBase*>(&sim) : static_cast<Andor::AndorBase*>(&sdk);
        this->andor_simulated = simandor;
        return;
      }
      /***** Andor::Interface::sim_andor **************************************/

      /***** Andor::Interface::get_andor_object *******************************/
      /**
       * @brief      returns string indicating which object is in use
       * @return     "sim" or "sdk" or "null"
       *
       */
      inline std::string_view get_andor_object() {
        if ( this->andor == &sim ) return "sim";
        else
        if ( this->andor == &sdk ) return "sdk";
        else
        return "null";
      }
      /***** Andor::Interface::get_andor_object *******************************/

      Andor::Information camera_info;

      FITS_file fits_file;

      uint16_t* image_data;

      inline bool is_initialized() { return this->initialized; };

      long simandor( std::string args, std::string &retstring );
      long open( std::string args );
      long close();
      long test();
      long shutter();
      long exptime( int exptime_in );
      long exptime( std::string exptime_in, std::string &retstring );
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
