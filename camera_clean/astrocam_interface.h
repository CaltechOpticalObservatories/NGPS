/**
 * @file    astrocam_interface.h
 * @brief   this defines the AstroCam implementation of Camera::Interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "camera_interface.h"
#include "astrocam_controller.h"
#include "shutter.h"

#include <map>
#include <memory>

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

namespace Camera {

  const int NUM_EXPBUF = 3;  // number of exposure buffers
  const int _COL_ = 0;
  const int _ROW_ = 1;

  /**
   * @brief ENUM list for each readout type
   */
  enum ReadoutType {
    U1,
    L1,
    U2,
    L2,
    SPLIT1,
    SPLIT2,
    QUAD,
    FT2,
    FT1,
    NUM_READOUT_TYPES
    };

  class AstroCamInterface : public Interface {
    public:
      AstroCamInterface();
      ~AstroCamInterface() override;

      // These are virtual functions inherited by the Camera::Interface base class
      // and have their own controller-specific implementations which are
      // implemented in astrocam_interface.cpp
      //
      long abort( const std::string args, std::string &retstring ) override;
      long autodir( const std::string args, std::string &retstring ) override;
      long basename( const std::string args, std::string &retstring ) override;
      long bias( const std::string args, std::string &retstring ) override;
      long bin( const std::string args, std::string &retstring ) override;
      long connect_controller( const std::string args, std::string &retstring ) override;
      long parse_controller_config(std::string args) override;
      long configure_camera() override;
      long disconnect_controller( const std::string args, std::string &retstring ) override;
      long load_firmware( const std::string args, std::string &retstring ) override;
      long native( const std::string args, std::string &retstring ) override;
      long power( const std::string args, std::string &retstring ) override;
      long test( const std::string args, std::string &retstring ) override;

    private:
      std::map<int, Controller> controller;
      Shutter shutter;
      PreciseTimer shutter_timer;
      std::vector<FITS_file*> pFits;    //!< vector of FITS containers, one for each exposure number for multiple buffering
      std::atomic<int> pci_cmd_num;
      int numdev;                           //!< number of PCI devices detected in system
      int nexp;
      int nframes;                          //!< total number of frames to acquire from controller per expose
      bool useframes;                       //!< not all firmware supports frames
      std::vector<int> configured_devnums;  //!< configured PCI devices (from camerad.cfg file)
      std::vector<int> devnums;             //!< all opened and connected devices

      typedef struct {
        ReadoutType readout_type;       //!< enum for readout type
        uint32_t    readout_arg;        //!< argument for Arc firmware command
      } readout_info_t;

      std::map< std::string, readout_info_t > readout_source;  //!< STL map of readout sources indexed by readout name

      std::atomic<bool> state_monitor_thread_running;
      std::condition_variable state_monitor_condition;
      std::mutex state_lock;
      static void state_monitor_thread( Interface &interface );


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

      std::mutex frameinfo_mutex;       //!< protects access to frameinfo
      std::mutex framecount_mutex;      //!< protects access to frame count

      std::atomic<int> framethreadcount;
      std::mutex framethreadcount_mutex;
      void add_framethread();
      void remove_framethread();
      int get_framethread_count();
      void init_framethread_count();

      std::vector<std::vector<int>> writes_pending;     //!< vector of devnums that still have to write to FITS file for each exposure number
                                                        //!< (exposure number is the outer vector, dev is the inner vector)

      /**
       * @brief exposure pending stuff
       */
      std::vector<int> exposures_pending;  //!< vector of devnums with pending exposure (which needs to be stored)
      std::condition_variable exposure_condition;
      std::mutex exposure_lock;
      std::mutex pending_mtx;
      static void dothread_monitor_exposure_pending( Interface &interface );
      std::vector<int> exposure_pending_list();
      bool exposure_pending();
      void exposure_pending( int devnum, bool add );

      // These functions are specific to the AstroCam Interface and are not
      // found in the base class.
      //
      long buffer(std::string size_in, std::string &retstring);
      long disconnect_controller();
      long disconnect_controller(int dev);
      void exposure_progress();
      long exptime( const std::string args, std::string &retstring ) override;
      long expose( const std::string args, std::string &retstring ) override;
      long extract_dev_chan( std::string args, int &dev, std::string &chan, std::string &retstring );
      long geometry(std::string args, std::string &retstring);
      long image_size( std::string args, std::string &retstring, const bool save_as_default=false );
      bool is_camera_idle(int dev);
      bool is_camera_idle();

      // send 3-letter command to ...
      //
      long do_native(std::string cmdstr);                          ///< selected or all open controllers
      long do_native(std::string cmdstr, std::string &retstring);  ///< selected or all open controllers, return reply
      long do_native(std::vector<uint32_t> selectdev, std::string cmdstr);    ///< specified by vector
      long do_native(std::vector<uint32_t> selectdev, std::string cmdstr, std::string &retstring);  ///< specified by vector
      long do_native(int dev, std::string cmdstr, std::string &retstring);  ///< specified by devnum

  };
}
