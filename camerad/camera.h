/**
 * @file    camera.h
 * @brief   camera interface functions header file common to all camera interfaces (astrocam, archon)
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#pragma once

#include <CCfits/CCfits>           // needed here for types in set_axes()
#include <dirent.h>                // for imdir()
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <json.hpp>

#include "common.h"
#include "logentry.h"
#include "utilities.h"

// handy snprintf shortcut
#define SNPRINTF(VAR, ...) { snprintf(VAR, sizeof(VAR), __VA_ARGS__); }
#define _COL_ 0
#define _ROW_ 1

/***** Camera *****************************************************************/
/**
 * @namespace Camera
 *
 */
namespace Camera {

  constexpr long MAX_SHUTTER_DELAY = 3000;  // maximum shutter delay in ms

  class InterfaceBase {
    public:
      InterfaceBase() = default;
  };

  template <typename T>
  class NewInterface : public T {
    public:
      NewInterface() = default;

      long new_expose( std::string nseq_in ) { return T::new_expose( nseq_in ); }
  };


  /***** Camera::Shutter ******************************************************/
  /**
   * @class    Camera
   * @brief    This class interfaces the Bonn shutter control port to a serial port.
   * @details  This class uses ioctl calls to read and write RS232 signals, normally
   *           used for handshaking. RTS(7) drives the shutter open pin 7, where
   *           pin 8 is tied to GND(5). Blade A (1) and B (2) status pins are open
   *           collector outputs that are pulled up to +5V (6) through 1k and are
   *           read by DSR(6) and CTS(8), respectively. Similarly, the error pin (4)
   *           is read by DCD(1). Consequently, these are active-LO outputs.
   *           Since the computer has no RS232 port, a USB-RS232 converter is used.
   *
   */
  class Shutter {
    private:
      int state;    //!< shutter open state: 1=open, 0=closed, -1=uninitialized
      int RTS_bit;  //!< bitmask representing RTS line
      int fd;
      std::chrono::time_point<std::chrono::high_resolution_clock> open_time, close_time;
      double duration_sec;
    public:
      std::condition_variable condition;
      std::mutex lock;

      inline bool isopen()   const { return ( this->state==1 ? true : false ); }
      inline bool isclosed() const { return ( this->state==0 ? true : false ); }
      inline void arm()            { this->state = -1; }

      /***** Camera::Shutter:init *********************************************/
      /*
       * @brief      initialize the Shutter class
       * @details    Shutter class must be initialized before use.
       *             This opens a connection to the USB device for the USB-RS232
       *             serial converter. This requires a proper udev rule to assign
       *             the appropriate USB device to /dev/shutter
       * @return     ERROR or NO_ERROR
       *
       */
      long init()  {
        std::string function = "Camera::Shutter::init";
        long error = ERROR;
        int state = -1;

        // close any open fd
        //
        if ( this->fd>=0 ) { close( this->fd ); this->fd=-1; }

        // open the USB device
        //
        this->fd = open( "/dev/shutter", O_RDWR | O_NOCTTY );

        // If USB device opened then make sure the shutter is actually okay.
        // Check this by closing it, waiting to make sure it has closed,
        // then reading the state.
        //
        if ( this->fd>=0 ) {
          error  = this->set_close();
          if ( error != NO_ERROR ) {
            std::stringstream message;
            message << "ERROR closing shutter: " << std::strerror(errno);
            logwrite( function, message.str() );
          }
          std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
          if ( error==NO_ERROR ) error = this->get_state( state );
        }
        else logwrite( function, "ERROR: failed to open /dev/shutter USB device (check udev)" );

        if ( error==NO_ERROR && state==0) logwrite( function, "shutter initialized OK" );
        else logwrite( function, "ERROR: failed to initialize shutter" );

        return ( error );
      }
      /***** Camera::Shutter:init *********************************************/


      /***** Camera::Shutter:shutdown *****************************************/
      /*
       * @brief      closes the USB connection
       *
       */
      void shutdown() {
        if ( this->fd>=0 ) { close( this->fd ); logwrite( "Camera::Shutter::shutdown", "USB device closed" ); }
        this->fd=-1;
        return;
      }
      /***** Camera::Shutter:shutdown *****************************************/


      /**
       * @brief      properly sets duration_sec when taking a 0s exposure
       */
      inline void zero_exposure() { this->duration_sec = 0.0; }


      /***** Camera::Shutter:set_open *****************************************/
      /*
       * @brief      opens the shutter
       * @details    Uses iotcl( TIOCMBIS ) to set modem control register RTS bit.
       *             Sets the class variable open_time to record the time it opened.
       * @return     ERROR or NO_ERROR
       *
       * If this returns ERROR then it means the shutter class has not been initialized.
       *
       */
      inline long set_open() {
        this->state=1;
        this->duration_sec=NAN;  // reset duration, set on shutter close
        this->open_time = std::chrono::high_resolution_clock::now();
        return( ioctl( this->fd, TIOCMBIS, &this->RTS_bit ) < 0 ? ERROR : NO_ERROR );
      }
      /***** Camera::Shutter:set_open *****************************************/


      /***** Camera::Shutter:set_close ****************************************/
      /*
       * @brief      closes the shutter and calculates shutter open time in sec
       * @details    Uses iotcl( TIOCMBIC ) to clear modem control register RTS bit.
       *             Sets the class variable close_time to record the time it closed.
       * @return     ERROR or NO_ERROR
       *
       * If this returns ERROR then it means the shutter class has not been initialized.
       *
       */
      inline long set_close() {
        this->state=0;
        this->close_time = std::chrono::high_resolution_clock::now();
        // close shutter here
        long ret = ( ioctl( this->fd, TIOCMBIC, &this->RTS_bit ) < 0 ? ERROR : NO_ERROR );
        // set shutter duration now
        this->duration_sec = std::chrono::duration_cast<std::chrono::nanoseconds>(this->close_time
                                                                                - this->open_time).count() / 1000000000.;
        return ret;
      }
      /***** Camera::Shutter:set_close ****************************************/


      /***** Camera::Shutter:get_duration *************************************/
      /*
       * @brief      returns the shutter open/close time duration in seconds
       * @details    This was calculated by Shutter::set_close()
       * @return     double precision duration in sec
       *
       */
      inline double get_duration() const {
        return this->duration_sec;
      }
      /***** Camera::Shutter:get_duration *************************************/


      /***** Camera::Shutter:get_state ****************************************/
      /*
       * @brief      reads the shutter state
       * @details    Uses ioctl( TIOCMGET ) to read modem control status bits,
       *             but since the Bonn signals are active low, a loss of power
       *             could trigger an active state.
       * @param[out] state  reference to int to contain state, where {-1,0,1}={error,closed,open}
       * @return     ERROR or NO_ERROR
       *
       * Note there is a discrepancy among the various Bonn documentation pages
       * as to which pins are blade A and B. I am using Table 5 of the user manual,
       * which agrees with the schematic title "Interface to Bonn-Shutter" REV 2.0,
       * dated Sep 5, 2013.
       *
       */
      long get_state( int &state ) {
        std::string function = "Camera::Shutter::get_state";
        std::stringstream message;
        int serial;

        // get all modem status bits (TIOCMGET) and store in serial
        //
        int err = ioctl( this->fd, TIOCMGET, &serial );

        if ( err ) {
          message.str(""); message << "ERROR: ioctl system call: " << std::strerror(errno);
          logwrite( function, message.str() );
          state = -1;
          return ERROR;
        }

#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] serial=0x" << std::hex << serial
                                 << " CAR=0x" << TIOCM_CAR 
                                 << " DSR=0x" << TIOCM_DSR 
                                 << " CTS=0x" << TIOCM_CTS;
        logwrite( function, message.str() );
        message.str(""); message << "[DEBUG] serial & CAR=0x" << (serial & TIOCM_CAR)
                                 << ( (serial&TIOCM_CAR)==0 ? " <-- Bonn error":"" );
        logwrite( function, message.str() );
        message.str(""); message << "[DEBUG] serial & DSR=0x" << (serial & TIOCM_DSR)
                                 << ( (serial&TIOCM_DSR)==0 ? " <-- Blade A closed":"" );
        logwrite( function, message.str() );
        message.str(""); message << "[DEBUG] serial & CTS=0x" << (serial & TIOCM_CTS)
                                 << ( (serial&TIOCM_CTS)==0 ? " <-- Blade B closed":"" );
        logwrite( function, message.str() );
#endif

        // Check if all bits are low, which is an indicator of a power or connection problem,
        // since both blades can't be closed at the same time (but they can both be open).
        //
        if ( (serial & TIOCM_CAR)==0 &&
             (serial & TIOCM_DSR)==0 &&
             (serial & TIOCM_CTS)==0 ) {
          logwrite( function, "ERROR: all Bonn status bits are LO, indicating possible power loss or connection fault" );
          state = -1;
          return ERROR;
        }

        // If the CAR bit is not set in serial then this is a Bonn error (active low)
        //
        if ( (serial & TIOCM_CAR) == 0 ) {
          logwrite( function, "ERROR: Bonn shutter fatal error" );
          state = -1;
          return ERROR;
        }

        // If either DSR or CTS are not set then then shutter is closed
        //
        if ( ( (serial & TIOCM_DSR) == 0 ) || ( (serial & TIOCM_CTS) == 0 ) ) {  // shutter closed
          state = 0;
          return NO_ERROR;
        }
        else
        // If both DSR and CTS are set then the shutter is open
        //
        if ( ( serial & TIOCM_DSR ) && ( serial & TIOCM_CTS ) ) {                // shutter open
          state = 1;
          return NO_ERROR;
        }
        else {
          message.str(""); message << "ERROR: unknown state 0x" << std::hex << serial;
          logwrite( function, message.str() );
          state = -1;
          return ERROR;
        }
      }
      /***** Camera::Shutter:get_state ****************************************/


      Shutter() : state(-1), RTS_bit(TIOCM_RTS), fd(-1), duration_sec(NAN) {
        this->open_time = this->close_time = std::chrono::high_resolution_clock::now();
      }

      Shutter(const Shutter&) = delete;

      ~Shutter() { this->shutdown(); }
  };
  /***** Camera::Shutter ******************************************************/


  /***** Camera::Camera *******************************************************/
  /**
   * @class   Camera
   * @brief   This class describes the overall instrument camera system
   * @details 
   *
   */
  class Camera {
    private:
      std::string image_dir;
      std::string base_name;
      std::string fits_naming;
      std::string fitstime;                  //!< "YYYYMMDDHHMMSS" uesd for filename, set by get_fitsname()
      mode_t dirmode;                        //!< user specified mode to OR with 0700 for imdir creation
      int image_num;
      bool is_mex;
      bool is_longerror;                     //!< set to return error message on command port
      bool is_mexamps;                       //!< should amplifiers be written as multi-extension?
      std::atomic<bool> _abortstate;
      std::mutex abort_mutex;
      std::stringstream lasterrorstring;     //!< a place to preserve an error message
      long shutter_delay;                    /// shutter delay in milliseconds

    public:
      Camera();

      std::string   axis_x, axis_y;          //!< labels for axes
      int32_t       exposure_time;           //!< exposure time in exposure_unit
//    int           binning[2];              //!< bin factor. element 0=cols (serial), element 1=rows (parallel)

      bool          is_userkeys_persist;     //!< should userkeys persist or be cleared after each exposure?
      bool          autodir_state;           //!< if true then images are saved in a date subdir below image_dir, i.e. image_dir/YYYYMMDD/
      bool          abortstate;              //!< set true to abort the current operation (exposure, readout, etc.)
      bool          bonn_shutter;            //!< set false if Bonn shutter is not connected (defaults true)
      bool          ext_shutter;             //!< set true if an external shutter is connected to an ARC controller (defaults false)
      std::string   writekeys_when;          //!< when to write fits keys "before" or "after" exposure

      Common::Queue async;                   /// message queue object
      Shutter shutter;                       /// Bonn Shutter object
      PreciseTimer shutter_timer;            /// precise timer object for shutter timing

      inline long get_shutter_delay() const { return this->shutter_delay; }
      long set_shutter_delay( const std::string shdel_str );
      long set_shutter_delay( long shdel );
      void wait_shutter_delay() { precise_sleep( this->shutter_delay * 1000 ); }  /// uses the precise_sleep timer in utils
      void set_abortstate(bool state);
      bool get_abortstate();

      void set_dirmode( mode_t mode_in ) { this->dirmode = mode_in; }

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
      void mex(bool state_in);
      bool mex();
      long mex(std::string state_in, std::string &state_out);
      void longerror(bool state_in);
      bool longerror();
      long longerror(std::string state_in, std::string &state_out);
      void mexamps(bool state_in);
      bool mexamps();
      long mexamps(std::string state_in, std::string &state_out);
  };
  /***** Camera::Camera *******************************************************/

  /**
   * @typedef frame_type_t
   * @brief   ENUM list of frame types
   */
  typedef enum {
    FRAME_IMAGE,
    FRAME_RAW,
    NUM_FRAME_TYPES
  } frame_type_t;

  const char * const frame_type_str[NUM_FRAME_TYPES] = {
    "IMAGE",
    "RAW"
  };

  /***** Camera::Information **************************************************/
  /**
   * @class  Information
   * @brief  
   *
   */
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
      long          axes[3];                 //!< element 0=cols, 1=cols, 2=cubedepth

      /**
       * @var     cubedepth
       * @brief   depth, or number of slices for 3D data cubes
       * @details cubedepth is used to calculate memory allocation and is not necessarily the
       *          same as fitscubed, which is used to force the fits writer to create a cube.
       *
       */
      long          cubedepth;

      /**
       * @var     fitscubed
       * @brief   depth, or number of slices for 3D data cubes
       * @details fitscubed is used to tell the fitswriter to create a file with a 3rd axis or not
       *
       */
      long          fitscubed;

      int           binning[2];
      long          axis_pixels[2];
      long          region_of_interest[4];
      long          image_center[2];
      bool          abortexposure;
      bool          ismex;                   //!< the info object given to the FITS writer will need to know multi-extension status
      int           extension;               //!< extension number for data cubes
      bool          shutterenable;           //!< set true to allow the controller to open the shutter on expose, false to disable it
      std::string   shutteractivate;         //!< shutter activation state
      bool          arcsim;                  //!< is the ARC device simulated or real?
      std::int32_t  sim_et;                  //!< arc simulator elapsed time
      std::int32_t  sim_modet;               //!< arc simulator requested modify exposure time
      int32_t       exposure_time;           //!< exposure time in exposure_unit
      std::string   exposure_unit;           //!< exposure time unit
      int           exposure_factor;         //!< multiplier for exposure_unit relative to 1 sec (=1 for sec, =1000 for msec, etc.)
      double        exposure_progress;       //!< exposure progress (fraction)
      int           num_pre_exposures;       //!< pre-exposures are exposures taken but not saved
      std::string   fits_name;               //!< contatenation of Camera's image_dir + image_name + image_num
      std::string   start_time;              //!< system time when the exposure started (YYYY-MM-DDTHH:MM:SS.sss)

      std::vector< std::vector<long> > amp_section;

      double dispersion;
      double minwavel;

      Common::Header systemkeys;     ///< Header class object holds pri/ext FitsKeys for system use
      Common::Header telemkeys;      ///< Header class object holds pri/ext FitsKeys for telemetry use
      Common::Header userkeys;       ///< Header class object holds pri/ext FitsKeys for command line use

      std::map<std::string,Common::Header> devkeys;

      /***** Camera::Information:Information **********************************/
      /**
       * @brief   Information class constructor
       */
      Information()
        : datatype(USHORT_IMG),              ///< fixed for NGPS
          type_set(true),                    ///< fixed for NGPS
          axes{1,1,1},
          cubedepth(1),
          fitscubed(1),
          binning{1,1},
          region_of_interest{1,1,1,1},
          image_center{1,1},
          abortexposure(false),
          ismex(true),                       ///< fixed for NGPS
          shutterenable(true),               // default enabled shutter
          shutteractivate(""),
          arcsim(false),                     // the ARC device is not simulated
          sim_et(0),
          sim_modet(-1),
          exposure_time(-1),                 // default is exposure time undefined
          exposure_unit(""),                 // default is exposure unit undefined
          exposure_factor(-1),               // default undefined
          num_pre_exposures(0),              // default no pre-exposures
          dispersion(0),
          minwavel(0)
          { }
      /***** Camera::Information:Information **********************************/


      /**
       * @brief   Information copy constructor
       */
      Information( const Information &other )
        : hostname(other.hostname),
          port(other.port),
          activebufs(other.activebufs),
          bitpix(other.bitpix),
          datatype(other.datatype),
          type_set(other.type_set),
          frame_type(other.frame_type),
          detector_pixels{other.detector_pixels[0], other.detector_pixels[1]},
          section_size(other.section_size),
          image_memory(other.image_memory),
          current_observing_mode(other.current_observing_mode),
          readout_name(other.readout_name),
          readout_type(other.readout_type),
          axes{other.axes[0], other.axes[1], other.axes[2]},
          cubedepth(other.cubedepth),
          fitscubed(other.fitscubed),
          binning{other.binning[0], other.binning[1]},
          axis_pixels{other.axis_pixels[0], other.axis_pixels[1]},
          region_of_interest{other.region_of_interest[0], other.region_of_interest[1],
                             other.region_of_interest[2], other.region_of_interest[3]},
          image_center{other.image_center[0], other.image_center[1]},
          abortexposure(other.abortexposure),
          ismex(other.ismex),
          extension(other.extension),
          shutterenable(other.shutterenable),
          shutteractivate(other.shutteractivate),
          arcsim(other.arcsim),
          sim_et(other.sim_et),
          sim_modet(other.sim_modet),
          exposure_time(other.exposure_time),
          exposure_unit(other.exposure_unit),
          exposure_factor(other.exposure_factor),
          exposure_progress(other.exposure_progress),
          num_pre_exposures(other.num_pre_exposures),
          fits_name(other.fits_name),
          start_time(other.start_time),
          amp_section(other.amp_section),
          dispersion(other.dispersion),
          minwavel(other.minwavel),
          systemkeys(other.systemkeys),  // this will call the Common::Header copy constructor
          telemkeys(other.telemkeys),    // this will call the Common::Header copy constructor
          userkeys(other.userkeys),      // this will call the Common::Header copy constructor
          devkeys(other.devkeys)
          { }

      /**
       * @brief   Information assignment operator
       */
      Information &operator=(const Information &other) {
        if ( this != &other ) {
          hostname = other.hostname;
          port = other.port;
          activebufs = other.activebufs;
          bitpix = other.bitpix;
          datatype = other.datatype;
          type_set = other.type_set;
          frame_type = other.frame_type;
          detector_pixels[0] = other.detector_pixels[0];
          detector_pixels[1] = other.detector_pixels[1];
          section_size = other.section_size;
          image_memory = other.image_memory;
          current_observing_mode = other.current_observing_mode;
          readout_name = other.readout_name;
          readout_type = other.readout_type;
          axes[0] = other.axes[0];
          axes[1] = other.axes[1];
          axes[2] = other.axes[2];
          cubedepth = other.cubedepth;
          fitscubed = other.fitscubed;
          binning[0] = other.binning[0];
          binning[1] = other.binning[1];
          axis_pixels[0] = other.axis_pixels[0];
          axis_pixels[1] = other.axis_pixels[1];
          region_of_interest[0] = other.region_of_interest[0];
          region_of_interest[1] = other.region_of_interest[1];
          region_of_interest[2] = other.region_of_interest[2];
          region_of_interest[3] = other.region_of_interest[3];
          image_center[0] = other.image_center[0];
          image_center[1] = other.image_center[1];
          abortexposure = other.abortexposure;
          ismex = other.ismex;
          extension = other.extension;
          shutterenable = other.shutterenable;
          shutteractivate = other.shutteractivate;
          arcsim = other.arcsim;
          sim_et = other.sim_et;
          sim_modet = other.sim_modet;
          exposure_time = other.exposure_time;
          exposure_unit = other.exposure_unit;
          exposure_factor = other.exposure_factor;
          exposure_progress = other.exposure_progress;
          num_pre_exposures = other.num_pre_exposures;
          fits_name = other.fits_name;
          start_time = other.start_time;
          amp_section = other.amp_section;
          dispersion = other.dispersion;
          minwavel = other.minwavel;
          systemkeys = other.systemkeys;
          telemkeys = other.telemkeys;
          userkeys = other.userkeys;
          devkeys = other.devkeys;
        }
        return *this;
      }

      long pre_exposures( std::string num_in, std::string &num_out );

      long manage_userkeys( std::string args, std::string &retstring );

      /***** Camera::Information:set_axes *************************************/
      /**
       * @brief   
       * @return  ERROR or NO_ERROR
       *
       */
      long set_axes() {
        std::string function = "Camera::Information::set_axes";
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

        this->axis_pixels[0] = this->region_of_interest[1] -
                               this->region_of_interest[0] + 1;
        this->axis_pixels[1] = this->region_of_interest[3] -
                               this->region_of_interest[2] + 1;

        if ( this->cubedepth > 1 ) {
          this->axes[0] = this->axis_pixels[0] / this->binning[0];  // cols
          this->axes[1] = this->axis_pixels[1] / this->binning[1];  // rows
          this->axes[2] = this->cubedepth;                          // cubedepth
        }
        else {
          this->axes[0] = this->axis_pixels[0] / this->binning[0];  // cols
          this->axes[1] = this->axis_pixels[1] / this->binning[1];  // rows
          this->axes[2] = 1;                                        // (no cube)
        }

        this->section_size = this->axes[0] * this->axes[1] * this->axes[2];    // Pixels to write for this image section, includes depth for 3D data cubes

        this->image_memory = this->detector_pixels[0] 
                           * this->detector_pixels[1] * bytes_per_pixel;       // Bytes per detector

#ifdef LOGLEVEL_DEBUG
        message << "[DEBUG] region_of_interest[1]=" << this->region_of_interest[1]
                << " region_of_interest[0]=" << this->region_of_interest[0]
                << " region_of_interest[3]=" << this->region_of_interest[3]
                << " region_of_interest[2]=" << this->region_of_interest[2]
                << " axes[0]=" << this->axes[0]
                << " axes[1]=" << this->axes[1]
                << " binning[0]=" << this->binning[0]
                << " binning[1]=" << this->binning[1];
        logwrite( function, message.str() );
#endif

        return (NO_ERROR);
      }
  };
  /***** Camera::Information **************************************************/

}
/***** Camera *****************************************************************/
