/**
 * @file    astrocam.h
 * @brief   
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 * This is where you put things that are common to both ARC interfaces,
 * ARC-64 PCI and ARC-66 PCIe.
 *
 */

#pragma once

#include <CCfits/CCfits>           // needed here for types in set_axes()
#include <atomic>
#include <chrono>
#include <numeric>
#include <functional>              // pass by reference to threads
#include <thread>
#include <iostream>
#include <vector>
#include <initializer_list>

#include "utilities.h"
#include "common.h"
#include "camera.h"
#include "config.h"
#include "fits.h"
#include "CArcBase.h"
#include "ArcDefs.h"
#include "CExpIFace.h"
#include "CConIFace.h"
#include "CArcPCI.h"
#include "CArcPCIe.h"
#include "CArcDevice.h"
#include "CArcBase.h"
#include "CooExpIFace.h"


/***** AstroCam ***************************************************************/
/**
 * @namespace AstroCam
 * @brief     namespace for Astronomical Research Cameras (aka Leach) controller interface
 *
 */
namespace AstroCam {

  const int NUM_EXPBUF = 3;  // number of exposure buffers

  /**
   * ENUM list for each readout type
   */
  enum ReadoutType {
    U1,                 ///< enum for amplifier U1
    L1,                 ///< enum for amplifier L1
    U2,                 ///< enum for amplifier U2
    L2,
    SPLIT1,
    SPLIT2,
    QUAD,
    FT2,
    FT1,
//  HAWAII_1CH,         // TODO
//  HAWAII_4CH,         // TODO
//  HAWAII_16CH,        // TODO
//  HAWAII_32CH,        // TODO
//  HAWAII_32CH_LR,     // TODO
    NUM_READOUT_TYPES
    };


  /***** AstroCam::Callback ***************************************************/
  /**
   * @class Callback
   * @brief Callback class inherited from the ARC API
   */
  class Callback : public arc::gen3::CooExpIFace {
    public:
      Callback(){}
      void exposeCallback( int devnum, std::uint32_t uiElapsedTime, std::uint32_t uiExposureTime ); ///< called by CArcDevice::expose() during exposure
      void readCallback( int expbuf, int devnum, std::uint32_t uiPixelCount, std::uint32_t uiFrameSize );       ///< called by CArcDevice::expose() during readout
      void frameCallback( int expbuf,
                          int devnum,
                          std::uint32_t uiFramesPerBuffer,
                          std::uint32_t uiFrameCount,
                          std::uint32_t uiRows,
                          std::uint32_t uiCols,
                          void* pBuffer );                            ///< called by CArcDevice::expose() when a frame has been received
      void ftCallback( int expnun, int devnum );
  };
  /***** AstroCam::Callback ***************************************************/


  /***** AstroCam::XeInterlace ************************************************/
  /**
   * @class XeInterface
   * @brief deinterlacing class, experimental
   */
  template <typename T>
  class XeInterlace {
    private:
      T* imbuf;
      T* workbuf;

    public:
      XeInterlace(T* imbuf_in, T* workbuf_in) {
        this->imbuf   = imbuf_in;
        this->workbuf = workbuf_in;
      }
      ~XeInterlace() {};

      void split_parallel();
      void split_serial();
      void quad_ccd();
      std::string test(T* buf) { 
        std::stringstream ret;
        ret << " buf=" << buf << " this->workbuf=" << std::hex << this->workbuf << " imbuf=" << this->imbuf;
        return (ret.str());
      };
  };
  /***** AstroCam::XeInterlace ************************************************/


  /***** AstroCam::DeInterlace ************************************************/
  /**
   * @class  DeInterlace
   * @brief  This is the DeInterlace class.
   * @details The data it contains are pointers to the PCI image buffer and
   *          the working buffer where the deinterlacing is to take place.
   *
   *          The functions it contains are the procedures for performing
   *          the deinterlacing.
   *
   *          This is a template class so that the buffers are created of
   *          the proper type.
   *
   *          For the deinterlacing algorithms below, it is assumed that the
   *          amplifiers are written to imbuf in the order (0,1,2...) and 
   *          direction (-->) indicated.
   *
   */
  template <typename T>
  class DeInterlace {
    private:
      T* imbuf;
      T* workbuf;
      long bufsize;
      int cols;
      int rows;
      int readout_type;

      /***** AstroCam::DeInterlace::quad_ccd **********************************/
      /**
       * @brief     quad ccd deinterlacing routine
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       * ~~~
       * L2 +---------+---------+ U2
       *    | <------ | ------> |   
       *    |    3    |    2    |   
       *    +---------+---------+   
       *    |    0    |    1    |   
       *    | <------ | ------> |   
       * L1 +---------+---------+ U1
       * ~~~
       *
       */
      void quad_ccd( int row_start, int row_stop, int index ) {
        std::stringstream message;
        for ( int r = row_start / 2; r < row_stop / 2; r++ ) {
          int begin = r * this->cols;
          int end   = ( this->rows * this->cols ) - ( r * this->cols ) - 1;
          for ( int c = 0; c < this->cols / 2; c++ ) {
            message.str(""); message << "row_start=" << row_start << " row_stop=" << row_stop 
                                     << " r=" << r << " c=" << c
                                     << " begin=" << begin << " end=" << end 
                                     << " A=" << (begin + c ) << " B=" << (begin + this->cols - c - 1 ) << " C=" << (end - c ) << " D=" << (end - this->cols + c + 1 ) << " index=" << index;
//          if ( (c==0) && (r==row_start) && ( row_stop % (this->rows / 8) == 0 ) ) logwrite( "QUAD", message.str() );
            if (row_start==10) logwrite( "* QUAD *", message.str() );
//          logwrite( "* QUAD *", message.str() );
//if (index >= (this->rows*this->cols)) return;
            *( this->workbuf + begin + c )                  = *( this->imbuf + (index++) ); // index++ % 65535; // 0: L1
            *( this->workbuf + begin + this->cols - c - 1 ) = *( this->imbuf + (index++) ); // index++ % 65535; // 1: U1
            *( this->workbuf + end - c )                    = *( this->imbuf + (index++) ); // index++ % 65535; // 2: U2
            *( this->workbuf + end - this->cols + c + 1 )   = *( this->imbuf + (index++) ); // index++ % 65535; // 3: L2
          }
        }
      }
      /***** AstroCam::DeInterlace::quad_ccd **********************************/


      /***** AstroCam::DeInterlace::split_serial ******************************/
      /**
       * @brief     split serial deinterlacing routine
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       * ~~~
       *    +---------+---------+
       *    |         |         |
       *    |         |         |
       *    |    0    |    1    |
       *    | <------ | ------> |
       * L1 +---------+---------+ U1
       * ~~~
       *
       */
      void split_serial( int row_start, int row_stop, int index ) {
        for ( int r = row_start; r < row_stop; r++ ) {
          int left  = r * this->cols;
          int right = r * this->cols + this->cols - 1;
          for ( int c = 0; c < this->cols; c += 2 ) {
            *( this->workbuf + left++  ) = *( this->imbuf + (index++) );
            *( this->workbuf + right-- ) = *( this->imbuf + (index++) );
          }
        }
      }
      /***** AstroCam::DeInterlace::split_serial ******************************/


      /***** AstroCam::DeInterlace::split_serial2 *****************************/
      /**
       * @brief     split serial2 deinterlacing routine
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       *
       * ~~~
       * L2 +---------+---------+ U2
       *    | <------ | ------> |
       *    |    0    |    1    |
       *    |         |         |
       *    |         |         |
       *    +---------+---------+ 
       * ~~~
       *
       */
      void split_serial2( int row_start, int row_stop, int index ) {
        for ( int r = row_start; r < row_stop; r++ ) {
          int right = r * this->cols + this->cols/2 - 1;
          int left  = right - 1;
          for ( int c = 0; c < this->cols; c += 2 ) {
            *( this->workbuf + left--  ) = *( this->imbuf + (index--) );
            *( this->workbuf + right++ ) = *( this->imbuf + (index--) );
          }
        }
      }
      /***** AstroCam::DeInterlace::split_serial2 *****************************/


      /***** AstroCam::DeInterlace::flip_ud ***********************************/
      /**
       * @brief     flip image buffer up/down
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       *
       * ~~~
       * L2 +-------------------+
       *    | <---------------- |
       *    |         0         |
       *    |                   |
       *    |                   |
       *    +-------------------+
       * ~~~
       *
       */
      void flip_ud(int row_start, int row_stop, int index) {
        for ( int r=row_start; r<row_stop; r++ ) {
          for ( int c=this->cols-1; c>=0; c-- ) {
            int loc = (r * this->cols) + c ;
            *( this->workbuf + loc ) = *( this->imbuf + (index--) );
          }
        }
      }
      /***** AstroCam::DeInterlace::flip_ud ***********************************/


      /***** AstroCam::DeInterlace::flip_lr ***********************************/
      /**
       * @brief     flip image buffer left/right
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       *
       * ~~~
       *    +-------------------+
       *    |                   |
       *    |                   |
       *    |         0         |
       *    | ----------------> |
       *    +-------------------+ U1
       * ~~~
       *
       */
      void flip_lr(int row_start, int row_stop, int index) {
        for ( int r=row_start; r<row_stop; r++ ) {
          for ( int c=this->cols-1; c>=0; c-- ) {
            int loc = (r * this->cols) + c ;
            *( this->workbuf + loc ) = *( this->imbuf + (index++) );
          }
        }
      }
      /***** AstroCam::DeInterlace::flip_lr ***********************************/


      /***** AstroCam::DeInterlace::flip_udlr *********************************/
      /**
       * @brief     flip image buffer up/down and left/right
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       *
       * ~~~
       *    +-------------------+ U2
       *    | ----------------> |
       *    |         0         |
       *    |                   |
       *    |                   |
       *    +-------------------+
       * ~~~
       *
       */
      void flip_udlr(int row_start, int row_stop, int index) {
        for ( int r=row_start; r<row_stop; r++ ) {
          for ( int c=0; c<this->cols; c++ ) {
            int loc = (r * this->cols) + c ;
            *( this->workbuf + loc ) = *( this->imbuf + (index--) );
          }
        }
      }
      /***** AstroCam::DeInterlace::flip_udlr *********************************/


      /***** AstroCam::DeInterlace::flip_none *********************************/
      /**
       * @brief     no deinterlacing -- copy imbuf to workbuf
       * @param[in] row_start
       * @param[in] row_stop
       * @param[in] index
       * @details 
       *
       * ~~~
       *    +-------------------+
       *    |                   |
       *    |                   |
       *    |         0         |
       *    | <---------------- |
       * L1 +-------------------+
       * ~~~
       *
       */
      void none( int row_start, int row_stop, int index ) {
        for ( int r=row_start; r<row_stop; r++ ) {
          for ( int c=0; c<this->cols; c++ ) {
            int loc = (r * this->cols) + c ;
            *( this->workbuf + loc ) = *( this->imbuf + (index++) );
          }
        }
      }
      /***** AstroCam::DeInterlace::flip_none *********************************/


    public:

      /***** AstroCam::DeInterlace::DeInterlace *******************************/
      /**
       * @brief     class constructor
       * @param[in] buf1          
       * @param[in] buf2          
       * @param[in] bufsz         
       * @param[in] cols          
       * @param[in] rows          
       * @param[in] readout_type  
       *
       */
      DeInterlace(T* buf1, T* buf2, long bufsz, int cols, int rows, int readout_type) {
        this->imbuf = buf1;
        this->workbuf = buf2;
        this->bufsize = bufsz;
        this->cols = cols;
        this->rows = rows;
        this->readout_type = readout_type;
      }
      /***** AstroCam::DeInterlace::DeInterlace *******************************/


      /***** AstroCam::DeInterlace::info **************************************/
      /**
       * @brief     returns some info, just for debugging
       * @return    string
       *
       */
      std::string info() {
        std::stringstream ret;
        ret << " imbuf=" << std::hex << this->imbuf << " this->workbuf=" << std::hex << this->workbuf
            << " bufsize=" << std::dec << this->bufsize << " cols=" << this->cols << " rows=" << this->rows
            << " readout_type=" << this->readout_type;
        return ( ret.str() );
      }
      /***** AstroCam::DeInterlace::info **************************************/


      /***** AstroCam::DeInterlace::do_deinterlace ****************************/
      /**
       * @brief      calls the appropriate deinterlacing function for readout type
       * @param[in]  row_start
       * @param[in]  row_stop
       * @param[in]  index_flip
       *
       * This calls the appropriate deinterlacing function based on the readout
       * type, which is an enum that was given to the class constructor when
       * this DeInterlace object was constructed.
       *
       * The deinterlacing is performed from "row_start" to "row_stop" of the
       * final image, using the pixel "index" of the raw image buffer.
       *
       */
      void do_deinterlace( int row_start, int row_stop, int index, int index_flip ) {
        std::string function = "AstroCam::DeInterlace::do_deinterlace";
        std::stringstream message;
        int index_ud = ( this->rows * this->cols ) - index;   // index from end of buffer, backwards

        switch( this->readout_type ) {
          case U1:
            this->flip_lr( row_start, row_stop, index );
            break;
          case L1:
            this->none( row_start, row_stop, index );
            break;
          case U2:
            this->flip_lr( row_start, row_stop, index );
            break;
          case L2:
            this->flip_ud( row_start, row_stop, index_ud );
            break;
          case FT1:
          case SPLIT1:
            if ( this->cols % 2 != 0 ) {   // should have already been checked, but here for safety
              logwrite( function, "ERROR: cannot deinterlace: lowerboth requires an even number of columns" );
              break;
            }
            this->split_serial( row_start, row_stop, index );
            break;
          case FT2:
          case SPLIT2:
            if ( this->cols % 2 != 0 ) {   // should have already been checked, but here for safety
              logwrite( function, "ERROR: cannot deinterlace: upperboth requires an even number of columns" );
              break;
            }
            this->split_serial2( row_start, row_stop, index_flip );
            break;
          case QUAD:
            if ( ( this->cols % 2 != 0 ) || ( this->rows % 2 != 0 ) ) {   // should have already been checked, but here for safety
              logwrite( function, "ERROR: cannot deinterlace: quad requires an even number of rows and columns" );
              break;
            }
            this->quad_ccd( row_start, row_stop, index );
            break;
          default:
            message.str(""); message << "ERROR: unknown readout type: " << this->readout_type;
            logwrite( function, message.str() );
        }
      }
      /***** AstroCam::DeInterlace::do_deinterlace ****************************/


      /***** AstroCam::DeInterlace::do_bob_deinterlace ************************/
      /**
       * @brief      
       * @param[in]  row_start
       * @param[in]  row_stop
       * @param[in]  index
       *
       */
      static void do_bob_deinterlace( int row_start, int row_stop, int index ) {
        std::string function = "AstroCam::DeInterlace::do_bob_deinterlace";
        std::stringstream message;
        message << "row_start=" << row_start << " row_stop=" << row_stop << " index=" << index;
        logwrite( function, message.str() );
        sleep( 2 );
      }
      /***** AstroCam::DeInterlace::do_bob_deinterlace ************************/


      /***** AstroCam::DeInterlace::bob ***************************************/
      /**
       * @brief      
       *
       */
      long bob() {
        std::string function = "AstroCam::DeInterlace::bob";
        std::stringstream message;
        int nthreads = cores_available();
        std::vector<std::thread> threads;       // create a local scope vector for the threads

#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "*** [DEBUG] spawning deinterlacing threads, from 1 to " << nthreads << "...";
        logwrite( function, message.str() );
#endif
        for ( int section = 1; section <= nthreads; section++ ) {
          int rows_per_section = (int)( this->rows / nthreads );                         // whole number of rows per thread
          int index            = rows_per_section * this->cols * ( section - 1);         // index from start of buffer, forward
          int row_start        = rows_per_section * (section-1);                         // first row this thread will deinterlace
          int row_stop         = rows_per_section * section;                             // last row this thread will deinterlace
          int modrows          = this->rows % nthreads;                                  // are the rows evenly divisible by the number of threads?
          if ( ( modrows != 0 ) && ( section == nthreads ) ) row_stop += modrows;        // add any leftover rows to the last thread if not evenly divisible

          std::thread thr( do_bob_deinterlace,
                           row_start,
                           row_stop,
                           index );
          threads.push_back(std::move(thr));    // push the thread into a vector
        }

        try {
          for (std::thread & thr : threads) {   // loop through the vector of threads
            if ( thr.joinable() ) thr.join();   // if thread object is joinable then join to this function. (not to each other)
          }
        }
        catch(const std::exception &e) {
          message.str(""); message << "ERROR joining threads: " << e.what();
          logwrite(function, message.str());
        }
        catch(...) { logwrite(function, "unknown error joining threads"); }

        threads.clear();                        // deconstruct the threads vector

        return NO_ERROR;
      }
      /***** AstroCam::DeInterlace::bob ***************************************/

  };
  /***** AstroCam::DeInterlace ************************************************/


  class NewAstroCam : public Camera::InterfaceBase {
    public:

      NewAstroCam() = default;

      long new_expose( std::string nseq_in );
  };

  /***** AstroCam::Interface **************************************************/
  /**
   * @class Interface
   * @brief This class defines the interface to the AstroCam controller
   *
   */
  class Interface : public Camera::InterfaceBase {
    private:
//    int bufsize;
      int FITS_STRING_KEY;
      int FITS_DOUBLE_KEY;
      int FITS_INTEGER_KEY;
      int FITS_BPP16;
      int FITS_BPP32;

      std::atomic<int> pci_cmd_num;
      std::atomic<int> nexp;       //!< number of exposures (repeat calls to do_expose)
      int nfilmstrip;              //!< number of filmstrip frames (for enhanced-clocking dual-exposure mode)
      int deltarows;               //!< number of delta rows (for enhanced-clocking dual-exposure mode)
      int nfpseq;                  //!< number of frames per sequence
      int nframes;                 //!< total number of frames to acquire from controller, per "expose"
      int nsequences;              //!< number of sequences
      int expdelay;                //!< exposure delay
      int _expbuf;                 //!< points to next avail in exposure vector
      std::mutex _expbuf_mutex;    //!< mutex to protect expbuf operations
      int imnumber;                //!< 
      int nchans;                  //!<
      int writefreq;               //!<
      bool iscds;                  //!< is CDS mode enabled?
      bool iscdsneg;               //!< is CDS subtraction polarity negative? (IE. reset-read ?)
      bool isutr;                  //!< is SUTR mode enabled?
      std::string basename;        //!<
      std::string imdir;
      std::string fitsname;
      std::vector<int> validchans; //!< outputs supported by detector / controller
//    std::string deinterlace;     //!< deinterlacing
      unsigned short* pResetbuf;   //!< buffer to hold reset frame (first frame of CDS pair)
      long* pCDSbuf;               //!< buffer to hold CDS subtracted image
//    void* workbuf;               //!< workspace for performing deinterlacing
      int num_deinter_thr;         //!< number of threads that can de-interlace an image
      int numdev;                  //!< total number of Arc devices detected in system
      std::vector<int> configured_devnums;  //!< vector of configured Arc devices (from camerad.cfg file)
      std::vector<int> devnums;    //!< vector of all opened and connected devices

      std::mutex epend_mutex;
      std::vector<int> exposures_pending;  //!< vector of devnums that have a pending exposure (which needs to be stored)

      std::vector<std::vector<int>> writes_pending;     //!< vector of devnums that still have to write to FITS file for each exposure number
                                                        //!< (exposure number is the outer vector, dev is the inner vector)

      void retval_to_string( std::uint32_t check_retval, std::string& retstring );

    public:
      Interface();

      // Class Objects
      //
      Config config;
      Camera::Camera camera;            /// instantiate a Camera object
      Camera::Information camera_info;  /// this is the main camera_info object

// vector of pointers to Camera Information containers, one for each exposure number
//
std::vector<std::shared_ptr<Camera::Information>> fitsinfo;

      // The frameinfo structure holds frame information for each frame
      // received by the callback. This is used to keep track of all the 
      // threads spawned by handle_frame.
      //
      typedef struct {
        int   tid;                      //!< use fpbcount as the thread ID here
        int   framenum;                 //!< the current frame from ARC_API's fcount, counts from 1
        int   rows;                     //!< number of rows in this frame
        int   cols;                     //!< number of cols in this frame
        void* buf;                      //!< pointer to the start of memory holding this frame
        bool  inuse;                    //!< this thread ID is in use, set when thread is spawned, cleared when handle_frame is done
      } frameinfo_t;

//    std::map<int, frameinfo_t> frameinfo;  // instantiation moved to Controller class

//    std::vector< XeInterlace<T> > deinter;

      std::mutex frameinfo_mutex;       //!< protects access to frameinfo
      std::mutex framecount_mutex;      //!< protects access to frame count

      std::atomic<int> framethreadcount;
      std::mutex framethreadcount_mutex;
      inline void add_framethread();
      inline void remove_framethread();
      inline int get_framethread_count();
      inline void init_framethread_count();

      /**
       * @brief      is the specified camera idle?
       * @details    Asking if a camera is idle refers to its activity
       *             status -- actively reading out or in frame transfer.
       * @param[in]  dev  device number to check
       * @return     true|false
       *
       */
      inline bool is_camera_idle( int dev ) {
        int num=0;
        num += ( this->controller[dev].in_readout ? 1 : 0 );
        num += ( this->controller[dev].in_frametransfer ? 1 : 0 );
        std::lock_guard<std::mutex> lock( this->epend_mutex );
        num += this->exposures_pending.size();
        return ( num>0 ? false : true );
      }

      inline bool is_camera_idle() {
        int num=0;
        for ( auto dev : this->devnums ) {
          num += ( this->controller[dev].in_readout ? 1 : 0 );
          num += ( this->controller[dev].in_frametransfer ? 1 : 0 );
        }
        std::lock_guard<std::mutex> lock( this->epend_mutex );
        num += this->exposures_pending.size();
        return ( num>0 ? false : true );
      }

      inline bool in_readout() const {
        int num=0;
        for ( auto dev : this->devnums ) {
          num += ( this->controller.at(dev).in_readout ? 1 : 0 );
          num += ( this->controller.at(dev).in_frametransfer ? 1 : 0 );
        }
        return( num==0 ? false : true );
      }

      inline bool in_frametransfer() const {
        int num=0;
        for ( auto dev : this->devnums ) {
          num += ( this->controller.at(dev).in_frametransfer ? 1 : 0 );
        }
        return( num==0 ? false : true );
      }

      inline void inc_expbuf() {
        std::lock_guard<std::mutex> lock( _expbuf_mutex );
        _expbuf = ( ( ++_expbuf >= NUM_EXPBUF ) ? 0 : _expbuf );
        return;
      }

      inline int get_expbuf() {
        std::lock_guard<std::mutex> lock( _expbuf_mutex );
        return _expbuf;
      }

      std::atomic<bool> state_monitor_thread_running;
      std::condition_variable state_monitor_condition;
      std::mutex state_lock;
      static void state_monitor_thread( Interface &interface );


      /*
       * exposure pending stuff
       *
       */
      std::condition_variable exposure_condition;
      std::mutex exposure_lock;
      static void dothread_monitor_exposure_pending( Interface &interface );


      /***** Interface::exposure_pending_list *********************************/
      /**
       * @brief      returns the exposure_pending vector
       * @return     std::vector<int>
       *
       */
      inline std::vector<int> exposure_pending_list() {
        std::lock_guard<std::mutex> lock( this->epend_mutex );
        return( this->exposures_pending );
      }
      /***** Interface::exposure_pending_list *********************************/


      /***** Interface::exposure_pending **************************************/
      /**
       * @brief      Is an exposure pending or can a new exposure be started?
       * @returns    bool { true | false }
       *
       * This function is overloaded with a version that allows
       * setting the exposure_pending state.
       *
       */
      inline bool exposure_pending() {
        std::lock_guard<std::mutex> lock( this->epend_mutex );
        return ( this->exposures_pending.size() > 0 ? true : false );
      }
      /***** Interface::exposure_pending **************************************/


      /***** Interface::exposure_pending **************************************/
      /**
       * @brief      Set or clear the exposure pending state for a given controller
       * @param[in]  devnum  controller device number
       * @param[in]  add     bool true to add, false to remove from pending vector
       *
       * This function is overloaded with an inline version that returns a bool
       * to indicate if there are any controllers with a pending exposure.
       *
       */
      void exposure_pending( int devnum, bool add ) {
        std::lock_guard<std::mutex> lock(this->epend_mutex);       // protect exposures_pending vector

        auto it     = std::find( this->exposures_pending.begin(),
                                 this->exposures_pending.end(),
                                 devnum );                         // iterator of devnum in exposures_pending vector
        bool found  = ( it != this->exposures_pending.end() );     // was devnum found in vector?
        bool remove = !add;                                        // do I remove it?

        if ( add && !found )   { this->exposures_pending.push_back( devnum ); }
        else
        if ( found && remove ) { this->exposures_pending.erase( it ); }

        this->exposure_condition.notify_all();

        return;                                                    // lock_guard releases on exit
      }
      /***** Interface::exposure_pending **************************************/


      /*
       * write pending stuff
       *
       */
      std::condition_variable write_condition;
      std::mutex write_lock;

      /***** Interface::writes_pending_list ***********************************/
      /**
       * @brief      returns the writes_pending vector
       * @param[in]  expbuf  the exposure buffer number
       * @return     std::vector<int>, the vector of devnums for this expbuf
       *
       */
      inline std::vector<int> writes_pending_list( int expbuf ) {
        std::lock_guard<std::mutex> lock( this->write_lock );
        try { return( this->writes_pending.at( expbuf ) ); }
        catch ( std::out_of_range & ) { return {}; }
      }
      /***** Interface::writes_pending_list ***********************************/


      /***** Interface::write_pending *****************************************/
      /**
       * @brief      Set or clear the write pending state for a given exposure
       * @param[in]  expbuf  exposure buffer number
       * @param[in]  devnum  device number
       * @param[in]  add     bool true to add, false to remove from pending vector
       *
       * This function is overloaded with an inline version that returns a bool
       * to indicate if there are any controllers with a pending write.
       *
       */
      void write_pending( int expbuf, int devnum, bool add ) {

        try {
          std::lock_guard<std::mutex> lock(this->write_lock);               // protect writes_pending vector
          auto it = std::find( this->writes_pending.at( expbuf ).begin(),
                               this->writes_pending.at( expbuf ).end(),
                               devnum );                                    // iterator of devnum in writes_pending[expbuf] vector
          bool found  = ( it != this->writes_pending.at( expbuf ).end() );  // was expbuf found in vector?
          bool remove = !add;                                               // do I remove it?

          if ( add && !found )   { this->writes_pending.at( expbuf ).push_back( devnum ); }
          else
          if ( found && remove ) { this->writes_pending.at( expbuf ).erase( it ); }

        }                                                                   // lock_guard releases when it goes out of scope
        catch ( std::out_of_range & ) { return; }

        this->write_condition.notify_all();

#ifdef LOGLEVEL_DEBUG
        std::stringstream message;
        message << "[DEBUG]";
        for (const auto &dev : this->writes_pending[expbuf]) message << " " << dev;
        logwrite("Interface::write_pending", message.str());
#endif
        return;
      }
      /***** Interface::write_pending *****************************************/


      /***** AstroCam::Controller *********************************************/
      /**
       * @class    Controller
       * @brief    contains information for each controller
       * @details  The Controller class is a sub-class of Interface and is
       *           here to contain the Camera::Information class and FITS_file
       *           class objects.  There will be a vector of Controller class
       *           objects which matches the vector of controller objects.
       *
       */
      class Controller {
        private:
          uint32_t bufsize;
          int framecount;               //!< keep track of the number of frames received per expose
          long workbuf_size;

        public:
          Controller();                 //!< class constructor
          ~Controller() { };            //!< no deconstructor

          Camera::Information info;     //!< camera info object for this controller

          void* workbuf;                //!< pointer to workspace for performing deinterlacing

          /**
           * @var     expinfo
           * @brief   vector of Camera::Information class, one for each exposure buffer
           * @details This contains the camera information for each exposure buffer.
           *          Note that since Camera::Information contains extkeys and prikeys objects,
           *          expinfo will contain those objects, but prikeys should not be used.
           *          Use only Controller::expinfo.extkeys because keys specific to a given
           *          controller will be added to that controller's extension.
           */
          std::vector<Camera::Information> expinfo;

          std::mutex pcimtx;               //!< mutex protects talking to this PCI driver

          int error;

          int cols;                        //!< total number of columns read (includes overscan)
          int rows;                        //!< total number of rows read (includes overscan)

          enum Axis { ROW, COL };

          Axis spec_axis;                  ///< which physical axis {ROW,COL} is spectral
          Axis spat_axis;                  ///< which physical axis {ROW,COL} is spatial

          // translate dimensions, logical-to-physical and physical-to-logical
          void logical_to_physical(int spat, int spec, int &rows, int &cols) const;
          void physical_to_logical(int rows, int cols, int &spat, int &spec) const;

          // get the physical axis that corresponds to a logical axis
          int spat_physical_axis() const { return spat_axis == ROW ? _ROW_ : _COL_; }
          int spec_physical_axis() const { return spec_axis == ROW ? _ROW_ : _COL_; }

          // These are detector image geometry values for each device,
          // unaffected by binning.
          //
          int detcols;                     //!< number of detector columns (unchanged by binning)
          int detrows;                     //!< number of detector rows (unchanged by binning)
          int oscols0;                     //!< requested number of overscan rows
          int osrows0;                     //!< requested number of overscan columns
          int oscols;                      //!< realized number of overscan rows (can be modified by binning)
          int osrows;                      //!< realized number of overscan columns (can be modified by binning)
          int skiprows;
          int skipcols;

          int defcols;                     //!< number of detector columns (unchanged by binning)
          int defrows;                     //!< number of detector rows (unchanged by binning)
          int defoscols;                   //!< requested number of overscan rows
          int defosrows;                   //!< requested number of overscan columns

          std::string imsize_args;         ///< IMAGE_SIZE arguments read from config file, used to restore default

          arc::gen3::CArcDevice* pArcDev;  //!< arc::CController object pointer -- things pointed to by this are in the ARC API
          Callback* pCallback;             //!< Callback class object must be pointer because the API functions are virtual
          bool connected;                  //!< true if controller connected (requires successful TDL command)
          bool inactive;                   //!< set true to skip future use of controllers when unable to connect
          bool is_imsize_set;              //!< has image_size been called after controller connected?
          bool firmwareloaded;             //!< true if firmware is loaded, false otherwise
          std::string firmware;            //!< name of firmware (.lod) file
          std::string channel;             //!< name of spectrographic channel
          std::string ccd_id;              //!< CCD identifier (E.G. serial number, name, etc.)
          int devnum;                      //!< this controller's devnum
          std::string devname;             //!< comes from arc::gen3::CArcPCI::getDeviceStringList()
          std::uint32_t retval;            //!< convenient place to hold return values for threaded commands to this controller
          std::map<int, frameinfo_t>  frameinfo;  //!< STL map of frameinfo structure (see above)
          uint32_t readout_arg;

          bool have_ft;                    //!< Do I have (and am I using) frame transfer?
          std::atomic<bool> in_readout;    //!< Is the controller currently reading out/transmitting pixels?
          std::atomic<bool> in_frametransfer;  //!< Is the controller currently performing a frame transfer?

          // Functions
          //
          inline uint32_t get_bufsize() { return this->bufsize; };
          inline uint32_t set_bufsize( uint32_t sz ) { this->bufsize=sz; return this->bufsize; };
          long alloc_workbuf();
//        int get_devnum() { return this->devnum; }
//        void set_devnum(int devnum) { this->devnum = devnum; }

          inline void init_framecount();
          inline int get_framecount();
          inline void increment_framecount();

          template <class T> T* deinterlace( int expbuf, T* imbuf );

          template <class T> static void dothread_deinterlace( DeInterlace<T> &deinterlace, int cols, int rows, int section, int nthreads );

          template <class T> void* alloc_workbuf(T* buf);

          template <class T> void free_workbuf(T* buf);

          long write();                 //!< wrapper for this->pFits->write_image()

          long open_file( std::string writekeys );    //!< wrapper for this->pFits->open_file()
          void close_file( std::string writekeys );   //!< wrapper for this->pFits->close_file()
      };
      /***** AstroCam::Controller *********************************************/


      // STL map of Controller objects, indexed by dev and created by Interface::connect_controller()
      //
      std::map<int, Controller> controller;

      // This is used for Archon. May or may not use modes for Astrocam, TBD.
      //
      bool modeselected;                //!< true if a valid mode has been selected, false otherwise

      bool useframes;                   //!< Not all firmware supports frames.

//    FITS_file fits_file;              //!< instantiate a FITS container object
      std::vector<FITS_file*> pFits;    //!< vector of FITS containers, one for each exposure number for multiple buffering

      typedef struct {
        ReadoutType readout_type;       //!< enum for readout type
        uint32_t    readout_arg;        //!< argument for Arc firmware command
      } readout_info_t;

      std::map< std::string, readout_info_t > readout_source;  //!< STL map of readout sources indexed by readout name

      std::map<std::string, int> telemetry_providers;  //!< a map of port[daemon_name] for telemetry providers

      // Functions
      //
      void exposure_progress();
      void make_image_keywords( int dev );
      long handle_json_message( std::string message_in );
      long parse_spec_info( std::string args );
      long parse_det_geometry( std::string args );
      long parse_controller_config( std::string args );
      int  devnum_from_chan( const std::string &chan );
      long extract_dev_chan( std::string args, int &dev, std::string &chan, std::string &retstring );
      long test(std::string args, std::string &retstring);                 ///< test routines
      void dothread_test_shutter_timer(long ms);
      long interface(std::string &iface);                                  ///< returns the interface
      long do_abort();                                                     ///< 
      long abort();                                                        ///< 
      long do_bin( std::string args, std::string &retstring );             ///< set/get binning factor
      long bin( std::string args, std::string &retstring );                ///< set/get binning factor
      long do_connect_controller(const std::string devices_in, std::string &retstring); ///< opens a connection to the PCI/e device(s)
      long connect_controller(std::string devices_in, std::string &help);  ///< wrapper for do_connect_controller
      long is_connected( std::string &retstring );                         ///< are all selected controllers connected?
      long do_disconnect_controller( int dev );                            ///< closes the connection to the named PCI/e device
      long do_disconnect_controller();                                     ///< closes the connection to all PCI/e devices
      long disconnect_controller();                                        ///< wrapper for do_disconnect_controller
      long do_configure_controller();                                      ///< perform initial configuration of controller from .cfg file
      long configure_controller();                                         ///< wrapper for do_configure_controller
      long access_useframes(std::string& useframes);                       ///< set or get the state of this->useframes
      long access_nframes(std::string valstring);                          ///<
      int get_driversize();                                                ///<
      long do_load_firmware(std::string &retstring);                       ///< load firmware (.lod) into one or all controller timing boards
      long do_load_firmware(std::string timlodfile, std::string &retstring);
      long load_firmware(std::string &retstring);                          ///< wrapper for load_firmware
      long load_firmware(std::string timlodfile, std::string &retstring);  ///< wrapper for load_firmware
      long band_of_interest( std::string args, std::string &retstring );   ///< set/get interest bands
      long set_camera_mode(std::string mode);
      long exptime(std::string exptime_in, std::string &retstring);
      long do_exptime(std::string exptime_in, std::string &retstring);
      long modify_exptime( std::string exptime_in, std::string &retstring );
      long do_modify_exptime( std::string exptime_in, std::string &retstring );
      long stop_exposure( std::string exptime_in, std::string &retstring );
      long pause_exposure( std::string exptime_in, std::string &retstring );
      long resume_exposure( std::string exptime_in, std::string &retstring );
      long shutter(std::string shutter_in, std::string& shutter_out);
      long frame_transfer_mode( std::string args );
      long frame_transfer_mode( std::string args, std::string &retstring );
      long image_size( std::string args, std::string &retstring, const bool save_as_default=false );
      long _image_size( std::string args, std::string &retstring, const bool save_as_default=false );
      long geometry(std::string args, std::string &retstring);
      long do_geometry(std::string args, std::string &retstring);
      long bias(std::string args, std::string &retstring);
      long buffer(std::string size_in, std::string &retstring);
      long readout(std::string readout_in, std::string &readout_out);
      long do_readout(std::string readout_in, std::string &readout_out);

      void set_imagesize(int rowsin, int colsin, int* status);

      long expose(std::string nexp_in);
      long do_expose(int nexp_in);
      void make_telemetry_message( std::string &retstring );
      void collect_telemetry();
      void collect_telemetry(std::string name, std::string &retstring);
      long native(std::string cmdstr);
      long native(std::string cmdstr, std::string &retstring);

      /**
       * send 3-letter command to ...
       */
      long do_native(std::string cmdstr);                          ///< selected or all open controllers
      long do_native(std::string cmdstr, std::string &retstring);  ///< selected or all open controllers, return reply
      long do_native(std::vector<uint32_t> selectdev, std::string cmdstr);    ///< specified by vector
      long do_native(std::vector<uint32_t> selectdev, std::string cmdstr, std::string &retstring);  ///< specified by vector
      long do_native(int dev, std::string cmdstr, std::string &retstring);  ///< specified by devnum

      long write_frame( int expbuf, int devnum, const std::string chan, int fpbcount );

      static void dothread_load( Controller &con, std::string timlodfile );
      static void dothread_shutter( int expbuf, Interface &interface );
      static void dothread_read( Camera::Camera &cam, Controller &con, int expbuf );
      static void dothread_expose( Controller &con );
      void dothread_native( int dev, std::vector<uint32_t> cmd );
//    static void dothread_native( Controller &con, std::vector<uint32_t> cmd );
      static void handle_frame( int expbuf, int devnum, uint32_t fpbcount, uint32_t fcount, void* buffer );
      static void handle_queue( std::string message );
      void handle_queue2( std::string message );
      static void FITS_handler( int expbuf, Interface &interface );

      // Functions fully defined here (no code in astrocam.c)
      //
      int keytype_string()  { return this->FITS_STRING_KEY;  };
      int keytype_double()  { return this->FITS_DOUBLE_KEY;  };
      int keytype_integer() { return this->FITS_INTEGER_KEY; };
      int fits_bpp16()      { return this->FITS_BPP16;       };
      int fits_bpp32()      { return this->FITS_BPP32;       };
//    int get_rows() { return this->rows; }; // REMOVE
//    int get_cols() { return this->cols; }; // REMOVE
//    int get_bufsize() { return this->bufsize; };
//    int set_rows(int r) { if (r>0) this->rows = r; return r; }; // REMOVE
//    int set_cols(int c) { if (c>0) this->cols = c; return c; }; // REMOVE
//    int get_image_rows() { return this->rows; };  // REMOVE
//    int get_image_cols() { return this->cols; }; // REMOVE

      using json = nlohmann::json;
      template <typename T>
      void collect_telemetry_key( const std::string &name, const std::string &key, T &value ) {
        const std::string function="AstroCam::Interface::collect_telemetry_key";
        std::stringstream message;

        std::string retstring;

        // collect the telemetry from this one named provider
        //
        collect_telemetry(name, retstring);

        // extract the correct typed value for the requested key from that
        // telemetry message
        //
        try {
          // get a JSON message from the serialized return string
          //
          nlohmann::json jmessage = nlohmann::json::parse( retstring );

          // extract the value from the JSON message using jkey as the key
          //
          auto jvalue = jmessage.at( key );

          if ( jvalue == nullptr ) return;

          if constexpr ( std::is_same<T, bool>::value ) {
            if ( jvalue.type() == json::value_t::boolean ) {
              value = jvalue.template get<bool>();
            }
          }
          else
          if constexpr ( std::is_same<T, int>::value ) {
            if ( jvalue.type() == json::value_t::number_integer ) {
              value = jvalue.template get<int>();
            }
          }
          else
          if constexpr ( std::is_same<T, uint16_t>::value ) {
            if ( jvalue.type() == json::value_t::number_unsigned ) {
              value = jvalue.template get<uint16_t>();
            }
          }
          else
          if constexpr ( std::is_same<T, float>::value || std::is_same<T, double>::value ) {
            if ( jvalue.type() == json::value_t::number_float ) {
              value = jvalue.template get<double>();
            }
          }
          else
          if constexpr ( std::is_same<T, std::string>::value ) {
            if ( jvalue.type() == json::value_t::string ) {
              value = jvalue.template get<std::string>();
            }
          }
          else {
            message << "ERROR unknown type for key " << key << " from provider " << name;
            logwrite( function, message.str() );
            return;
          }
        }
        catch( const json::exception &e ) {
          message << "JSON exception parsing value for key " << key << " from provider " << name << ": " << e.what();
          logwrite( function, message.str() );
        }
        catch( const std::exception &e ) {
          message << "ERROR exception parsing value for key " << key << " from provider " << name << ": " << e.what();
          logwrite( function, message.str() );
        }
        return;
      }
  };
  /***** AstroCam::Interface **************************************************/

}
/***** AstroCam ***************************************************************/
