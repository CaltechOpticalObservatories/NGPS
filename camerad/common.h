/**
 * @file    common.h
 * @brief   common interface functions header file
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef COMMON_H
#define COMMON_H

#include <CCfits/CCfits>           //!< needed here for types in set_axes()
#include <dirent.h>                //!< for imdir()
#include <map>
#include <vector>
#include <mutex>
#include <atomic>

#include <queue>
#include <condition_variable>

#include "logentry.h"
#include "utilities.h"

const long NOTHING = -1;
const long NO_ERROR = 0;
const long ERROR = 1;
const long BUSY = 2;
const long TIMEOUT = 3;

// handy snprintf shortcut
#define SNPRINTF(VAR, ...) { snprintf(VAR, sizeof(VAR), __VA_ARGS__); }

namespace Common {

  /**************** Common::FitsKeys ******************************************/
  //
  // This class provides a user-defined keyword database,
  // and the tools to access that database.
  //
  class FitsKeys {
    private:
    public:
      FitsKeys() {}
      ~FitsKeys() {}

      std::string get_keytype(std::string keyvalue);         //!< return type of keyword based on value
      long listkeys();                                       //!< list FITS keys in the internal database
      long addkey(std::string arg);                          //!< add FITS key to the internal database

      typedef struct {                                       //!< structure of FITS keyword internal database
        std::string keyword;
        std::string keytype;
        std::string keyvalue;
        std::string keycomment;
      } user_key_t;

      typedef std::map<std::string, user_key_t> fits_key_t;  //!< STL map for the actual keyword database

      fits_key_t keydb;                                      //!< keyword database

      // Find all entries in the keyword database which start with the search_for string,
      // return a vector of iterators.
      //
      std::vector< fits_key_t::const_iterator > FindKeys( std::string search_for ) {
        std::vector< fits_key_t::const_iterator > vec;
        for ( auto it  = this->keydb.lower_bound( search_for ); 
                   it != std::end( this->keydb ) && it->first.compare( 0, search_for.size(), search_for ) == 0; 
                 ++it ) {
          vec.push_back( it );
        }
        return( vec );
      }

      // Find and remove all entries in the keyword database which match the search_for string.
      //
      void EraseKeys( std::string search_for ) {
        std::string function = "FitsKeys::EraseKeys";
        std::stringstream message;
        std::vector< fits_key_t::const_iterator > erasevec;

        // get a vector of iterators for all the keys matching the search string
        //
        erasevec = this->FindKeys( search_for );
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] found " << erasevec.size() << " entries matching \"" << search_for << "*\"";
        logwrite( function, message.str() );
#endif

        // loop through that vector and erase the selected iterators from the database
        //
        for ( auto vec : erasevec ) {
#ifdef LOGLEVEL_DEBUG
	  message.str(""); message << "[DEBUG] erasing " << vec->first; logwrite( function, message.str() );
#endif
	  this->keydb.erase( vec );
	}
        return;
      }
  };
  /**************** Common::FitsKeys ******************************************/


  /**************** Common::Queue *********************************************/
  //
  // This class provides a thread-safe messaging queue.
  //
  class Queue {
    private:
      std::queue<std::string> message_queue;
      mutable std::mutex queue_mutex;
      std::condition_variable notifier;
      bool is_running;
    public:
      Queue(void) : message_queue() , queue_mutex() , notifier() { this->is_running = false; };
      ~Queue(void) {}

      void service_running(bool state) { this->is_running = state; };
      bool service_running() { return this->is_running; };

      void enqueue(std::string message);         //!< push an element into the queue.
      std::string dequeue(void);                 //!< pop an element from the queue
  };
  /**************** Common::Queue *********************************************/


  /**************** Common::Common ********************************************/
  //
  class Common {
    private:
      std::string image_dir;
      std::string base_name;
      std::string fits_naming;
      std::string fitstime;                  //!< "YYYYMMDDHHMMSS" uesd for filename, set by get_fitsname()
      int image_num;
      bool is_datacube;
      bool is_longerror;                     //!< set to return error message on command port
      bool is_cubeamps;                      //!< should amplifiers be written as multi-extension data cubes?
      std::atomic<bool> _abortstate;;
      std::mutex abort_mutex;
      std::stringstream lasterrorstring;     //!< a place to preserve an error message

    public:
      Common();
      ~Common() {}

      bool          autodir_state;           //!< if true then images are saved in a date subdir below image_dir, i.e. image_dir/YYYYMMDD/
      bool          abortstate;              //!< set true to abort the current operation (exposure, readout, etc.)
      std::string   writekeys_when;          //!< when to write fits keys "before" or "after" exposure
      Queue message;                         //!< message queue object

      void set_abortstate(bool state);
      bool get_abortstate();

      std::map<int, std::string> firmware;   //!< firmware file for given controller device number, read from .cfg file
      std::map<int, int> readout_time;       //!< readout time in msec for given controller device number, read from .cfg file

      void log_error( std::string function, std::string message );

      std::string get_longerror();

      long imdir(std::string dir_in);
      long imdir(std::string dir_in, std::string& dir_out);
      long autodir(std::string state_in, std::string& state_out);
      long basename(std::string name_in);
      long basename(std::string name_in, std::string& name_out);
      long imnum(std::string num_in, std::string& num_out);
      long writekeys(std::string writekeys_in, std::string &writekeys_out);
      long fitsnaming(std::string naming_in, std::string& naming_out);
      void increment_imnum() { if (this->fits_naming.compare("number")==0) this->image_num++; };
      void set_fitstime(std::string time_in);
      long get_fitsname(std::string &name_out);
      long get_fitsname(std::string controllerid, std::string &name_out);
      void abort();
      void datacube(bool state_in);
      bool datacube();
      long datacube(std::string state_in, std::string &state_out);
      void longerror(bool state_in);
      bool longerror();
      long longerror(std::string state_in, std::string &state_out);
      void cubeamps(bool state_in);
      bool cubeamps();
      long cubeamps(std::string state_in, std::string &state_out);
  };
  /**************** Common::Common ********************************************/

  typedef enum {
    FRAME_IMAGE,
    FRAME_RAW,
    NUM_FRAME_TYPES
  } frame_type_t;

  const char * const frame_type_str[NUM_FRAME_TYPES] = {
    "IMAGE",
    "RAW"
  };

  /**************** Common::Information ***************************************/
  //
  class Information {
    private:
    public:
      std::string   hostname;                //!< Archon controller hostname
      int           port;                    //!< Archon controller TPC/IP port number
      int           activebufs;              //!< Archon controller number of active frame buffers
      int           bitpix;                  //!< Archon bits per pixel based on SAMPLEMODE
      int           datatype;                //!< FITS data type (corresponding to bitpix) used in set_axes()
      bool          type_set;                //!< set when FITS data type has been defined
      frame_type_t  frame_type;              //!< frame_type is IMAGE or RAW
      long          detector_pixels[2];      //!< element 0=cols (pixels), 1=rows (lines)
      long          section_size;            //!< pixels to write for this section (could be less than full sensor size)
      long          image_memory;            //!< bytes per image sensor
      std::string   current_observing_mode;  //!< the current mode
      std::string   readout_name;            //!< name of the readout source
      int           readout_type;            //!< type of the readout source is an enum
      long          naxis;
      long          axes[2];
      int           binning[2];
      long          axis_pixels[2];
      long          region_of_interest[4];
      long          image_center[2];
      bool          abortexposure;
      bool          iscube;                  //!< the info object given to the FITS writer will need to know cube status
      int           extension;               //!< extension number for data cubes
      bool          shutterenable;           //!< set true to allow the controller to open the shutter on expose, false to disable it
      std::string   shutteractivate;         //!< shutter activation state
      int32_t       exposure_time;           //!< exposure time in exposure_unit
      std::string   exposure_unit;           //!< exposure time unit
      int           exposure_factor;         //!< multiplier for exposure_unit relative to 1 sec (=1 for sec, =1000 for msec, etc.)
      double        exposure_progress;       //!< exposure progress (fraction)
      int           num_pre_exposures;       //!< pre-exposures are exposures taken but not saved
      std::string   fits_name;               //!< contatenation of Common's image_dir + image_name + image_num
      std::string   start_time;              //!< system time when the exposure started (YYYY-MM-DDTHH:MM:SS.sss)

      std::vector< std::vector<long> > amp_section;

      FitsKeys userkeys;             //!< create a FitsKeys object for FITS keys specified by the user
      FitsKeys systemkeys;           //!< create a FitsKeys object for FITS keys imposed by the software


  Information() {
        this->axes[0] = 1;
        this->axes[1] = 1;
        this->binning[0] = 1;
        this->binning[1] = 1;
        this->region_of_interest[0] = 1;
        this->region_of_interest[1] = 1;
        this->region_of_interest[2] = 1;
        this->region_of_interest[3] = 1;
        this->image_center[0] = 1;
        this->image_center[1] = 1;
        this->iscube = false;
        this->datatype = -1;
        this->type_set = false;              //!< set true when datatype has been defined
        this->exposure_time = -1;            //!< default exposure time is undefined
        this->exposure_unit = "";            //!< default exposure unit is undefined
        this->exposure_factor = -1;          //!< default factor is undefined
        this->shutteractivate = "";
        this->num_pre_exposures = 0;         //!< default is no pre-exposures
      }

      long pre_exposures( std::string num_in, std::string &num_out );

      long set_axes() {
        std::string function = "Common::Information::set_axes";
        std::stringstream message;
        long bytes_per_pixel;

        if ( this->frame_type == FRAME_RAW ) {
          bytes_per_pixel = 2;
          this->datatype = USHORT_IMG;
        }
        else {
          switch ( this->bitpix ) {
            case 16:
              bytes_per_pixel = 2;
              this->datatype = SHORT_IMG;
              break;
            case 32:
              bytes_per_pixel = 4;
              this->datatype = FLOAT_IMG;
              break;
          default:
            message << "ERROR: unknown bitpix " << this->bitpix << ": expected {16,32}";
            logwrite( function, message.str() );
            return (ERROR);
          }
        }
        this->type_set = true;         // datatype has been set

        this->naxis = 2;

        this->axis_pixels[0] = this->region_of_interest[1] -
                               this->region_of_interest[0] + 1;
        this->axis_pixels[1] = this->region_of_interest[3] -
                               this->region_of_interest[2] + 1;

        this->axes[0] = this->axis_pixels[0] / this->binning[0];
        this->axes[1] = this->axis_pixels[1] / this->binning[1];

        this->section_size = this->axes[0] * this->axes[1];                    // Pixels to write for this image section
        this->image_memory = this->detector_pixels[0] 
                           * this->detector_pixels[1] * bytes_per_pixel;       // Bytes per detector

#ifdef LOGLEVEL_DEBUG
        message << "[DEBUG] region_of_interest[1]=" << this->region_of_interest[1]
                << " region_of_interest[0]=" << this->region_of_interest[0]
                << " region_of_interest[3]=" << this->region_of_interest[3]
                << " region_of_interest[2]=" << this->region_of_interest[2]
                << " axes[0]=" << this->axes[0]
                << " axes[1]=" << this->axes[1];
        logwrite( function, message.str() );
#endif

        return (NO_ERROR);
      }
  };
  /**************** Common::Information ***************************************/

}
#endif
