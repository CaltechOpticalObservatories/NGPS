/**
 * @file    generic.cpp
 * @brief   instrument-specific class definitions for generic instruments
 * @details This file defines functions for any generic instrument
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * Any functions defined here must have (generic) function definitions in archon.h.
 * This file contains the specific code for those function definitions which are
 * unique to the particular instrument.
 *
 * More than likely, if a function is here then it is not supported, otherwise it
 * would be defined for a particular instrument. This file is here so that the main
 * daemon will compile.
 *
 */

#include <string>
#include <sstream>
#include <vector>

#include "archon.h"
#include "common.h"
#include "utilities.h"
#include "logentry.h"

namespace Archon {

      /***** Archon::Interface::make_camera_header ****************************/
      /**
       * @brief      adds header keywords to the systemkeys database
       * @param[in]  
       * @param[out] 
       * @return     
       *
       */
      void Interface::make_camera_header( ) {
        std::stringstream keystr;

        keystr.str("");
        keystr << "EXPTIME=" << this->camera_info.exposure_time << " // exposure time in " << ( this->is_longexposure ? "sec" : "msec" );
        this->systemkeys.addkey( keystr.str() );

        keystr.str("");
        keystr << "NSEQ=" << this->camera_info.nseq << " // number of exposure sequences";
        this->systemkeys.addkey( keystr.str() );

        return;
      }
      /***** Archon::Interface::make_camera_header ****************************/


      /**************** Archon::Interface::region_of_interest *****************/
      /**
       * @brief      define a region of interest
       * @param[in]  args
       * @param[out] retstring
       * @return     ERROR
       *
       * ROI is not currently supported for generic instruments.
       *
       */
      long Interface::region_of_interest( std::string args, std::string &retstring ) {
        std::string function = "Archon::Interface::region_of_interest";
        std::stringstream message;
        this->camera.log_error( function, "ROI not supported" );
        return( ERROR );
      }
      /**************** Archon::Interface::region_of_interest *****************/


      /**************** Archon::Interface::multi_cds **************************/
      /**
       * @brief      
       * @param[in]  args
       * @param[out] retstring
       * @return     
       *
       * MCDS is not supported for generic instruments.
       *
       */
      long Interface::multi_cds( std::string args, std::string &retstring ) {
        std::string function = "Archon::Interface::multi_cds";
        std::stringstream message;
        this->camera.log_error( function, "MCDS not supported" );
        return( ERROR );
      }
      /**************** Archon::Interface::multi_cds **************************/


      /**************** Archon::Interface::sample_mode ************************/
      /**
       * @brief      
       * @param[in]  args
       * @param[out] retstring
       * @return     
       *
       * sample_mode is not supported for generic instruments.
       *
       */
      long Interface::sample_mode( std::string args, std::string &retstring ) {
        std::string function = "Archon::Interface::sample_mode";
        std::stringstream message;
        this->camera.log_error( function, "sample_mode command not supported" );
        return( ERROR );
      }
      /**************** Archon::Interface::sample_mode ************************/


      /**************** Archon::Interface::deinterlace ************************/
      /**
       * @brief      deinterlace
       * @param[in]  
       * @param[out] 
       * @return     
       *
       * Deinterlacing is not supported for generic instruments.
       *
       */
      template <class T>
      T* Interface::deinterlace(T* imbuf) {
        std::string function = "Archon::Instrument::deinterlace";
#ifdef LOGLEVEL_DEBUG
        logwrite( function, "deinterlacing not supported" );
#endif
        return( (T*)NULL );
      }
      /**************** Archon::Interface::deinterlace ************************/


}
