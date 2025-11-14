/** ---------------------------------------------------------------------------
 * @file    slicecam_camera.h
 * @brief   slicecam camera include
 * @details defines the Slicecam Camera class
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "common.h"
#include "logentry.h"
#include "slicecam_fits.h"
#include "slicecamd_commands.h"

/***** Slicecam ***************************************************************/
/**
 * @namespace Slicecam
 * @brief     namespace for slicer cameras
 *
 */
namespace Slicecam {

  /***** Slicecam::Camera *****************************************************/
  /**
   * @class  Camera
   * @brief  Camera class
   *
   * This class is used for communicating with the slicecam camera directly (which is an Andor)
   *
   */
  class Camera {
    private:
      uint16_t* image_data;
      int simsize;      /// for the sky simulator
      std::map<at_32, at_32> handlemap;

    public:
      Camera() : image_data( nullptr ), simsize(1024) { };

      FITS_file fits_file;        /// instantiate a FITS container object
      FitsInfo  fitsinfo;

      std::mutex framegrab_mutex;

      std::map<std::string, std::unique_ptr<Andor::Interface>> andor;     ///< container for Andor::Interface objects

      std::map<std::string, Andor::DefaultValues> default_config;         ///< container to hold defaults for each camera

      inline void copy_info() { fits_file.copy_info( fitsinfo ); }
      inline void set_simsize( int val )     { if ( val > 0 ) this->simsize = val;  else throw std::out_of_range("simsize must be greater than 0");  }

      inline long init_handlemap() {
        this->handlemap.clear();
        return this->andor.begin()->second->get_handlemap( this->handlemap );
      }

      long emulator( std::string args, std::string &retstring );
      long open( std::string which, std::string args );
      long close();
      long get_frame();
      long write_frame( std::string source_file, std::string &outfile, const bool _tcs_online );
      long bin( const int hbin, const int vbin );
      long set_fan( std::string which, int mode );
      long imflip( std::string args, std::string &retstring );
      long imrot( std::string args, std::string &retstring );
      long set_gain( int &gain );
      long set_gain( std::string which, int &gain );
      long set_gain( int &&gain );
      long set_gain( std::string which, int &&gain );
      long set_exptime( float &val );
      long set_exptime( std::string which, float &val );
      long set_exptime( float &&val );
      long set_exptime( std::string which, float &&val );
      long speed( std::string args, std::string &retstring );
      long temperature( std::string args, std::string &retstring );
  };
  /***** Slicecam::Camera *****************************************************/
}
