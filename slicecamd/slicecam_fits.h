#pragma once

#include <CCfits/CCfits>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include "utilities.h"
#include "common.h"
#ifdef ANDORSIM
#include "andorsim.h"
#else
#include "andor.h"
#endif

namespace Slicecam {

  /***** FitsInfo *************************************************************/
  /**
   * @class   FitsInfo
   * @brief   information class for the slicecam camera
   * @details contains information about the Andor camera
   *
   */
  class FitsInfo {
    private:
    public:
      int axes[2];
      int datatype;
      unsigned long section_size;
      std::string fits_name;

      Common::FitsKeys fitskeys;   ///< create a FitsKeys object for FITS keys imposed by the software
  };
  /***** FitsInfo *************************************************************/


  /***** FITS_file ************************************************************/
  /**
   * @class     FITS_file
   * @brief     class for FITS I/O using CCfits
   *
   */
  class FITS_file {
    private:
      std::mutex fits_mutex;                        ///< used to block writing_file semaphore in multiple threads
      std::unique_ptr<CCfits::FITS> pFits;          ///< pointer to FITS data container
      std::string fits_name;
      std::atomic<bool> file_open;                  ///< semaphore indicates file is open

    public:
      std::atomic<int> extension;

      FITS_file() : pFits( nullptr ), file_open( false ), extension( 0 ) { };

      FITS_file( Slicecam::FitsInfo info ) : pFits( nullptr ), file_open( false ), extension( 0 ), info( info ) { } ///< instantiated with existing info object

      Slicecam::FitsInfo info;

      inline void copy_info( const FitsInfo &newinfo ) { info = newinfo; }

      long open_file();
      void close_file();
      long copy_header_from( std::string file_in );
      long create_header();
      long copy_header( std::string wcs_in );
      long add_key( std::string keyword, std::string type, std::string value, std::string comment );

      long write_image( std::shared_ptr<Andor::Interface> slicecam );
//    long write_image( uint16_t* data, Slicecam::FitsInfo &imageinfo );
/*** 4/3/24
      template <typename T> long write_image( std::unique_ptr<T[]> &data ) {
        std::string function = "Slicecam::FITS_file::write_image";
        std::stringstream message;

message << "[DEBUG] section_size=" << this->info.section_size; logwrite( function, message.str() );

        if ( this->info.section_size == 0 ) {
          logwrite( function, "ERROR size is 0" );
          return( ERROR );
        }

        // copy the data into an array of the appropriate type
        //
        std::valarray<T> array( this->info.section_size );
        for ( unsigned long i = 0; i < this->info.section_size; i++ ) {
          array[i] = data[i];
        }


        // Set the FITS system to verbose mode so it writes error messages
        //
        CCfits::FITS::setVerboseMode( true );

        // write the primary image into the FITS file
        //
        try {
          long fpixel(1);        // start with the first pixel always
          this->pFits->pHDU().write( fpixel, this->info.section_size, array );
          this->pFits->flush();  // make sure the image is written to disk
        }
        catch ( CCfits::FitsError& error ) {
          message.str(""); message << "FITS file error thrown: " << error.message();
          logwrite(function, message.str());
          return( ERROR );
        }

        return( NO_ERROR );
      }
4/3/24 ***/

  };
  /***** FITS_file ************************************************************/

}
