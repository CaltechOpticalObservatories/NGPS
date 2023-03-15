/**
 * @file    camera.h
 * @brief   camera interface functions header file for the ACAM camera
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */
#ifndef ACAM_CAMERA_H
#define ACAM_CAMERA_H

#include <CCfits/CCfits>           // needed here for types in set_axes()
#include <dirent.h>                // for imdir()
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

#include "common.h"
#include "logentry.h"
#include "utilities.h"


/***** Camera *****************************************************************/
/**
 * @namespace Camera
 *
 */
namespace Camera {

  /***** Camera::Information **************************************************/
  /**
   * @class  Information
   * @brief  
   *
   */
  class Information {
    private:
    public:
      long          axes[2];
      bool          iscube;                  //!< the info object given to the FITS writer will need to know cube status
      std::string   fits_name;               //!< contatenation of Camera's image_dir + image_name + image_num
      bool          type_set;                //!< set when FITS data type has been defined
      int           datatype;                //!< FITS data type (corresponding to bitpix) used in set_axes()
      long          section_size;            //!< pixels to write for this section (could be less than full sensor size)
      int           extension;               //!< extension number for data cubes

      Common::FitsKeys userkeys;     ///< create a FitsKeys object for FITS keys specified by the user
      Common::FitsKeys systemkeys;   ///< create a FitsKeys object for FITS keys imposed by the software

  };
  /***** Camera::Information **************************************************/

}
/***** Camera *****************************************************************/
#endif

