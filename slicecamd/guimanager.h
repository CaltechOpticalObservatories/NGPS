/** ---------------------------------------------------------------------------
 * @file    guimanager.h
 * @brief   slicecam display GUI manager
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

/***** Slicecam ***************************************************************/
/**
 * @namespace Slicecam
 * @brief     namespace for slicer cameras
 *
 */
namespace Slicecam {

  /***** Slicecam::GUIManager *************************************************/
  /**
   * @class  GUIManager
   * @brief  defines functions and settings for the display GUI
   *
   */
  class GUIManager {
    private:
      const std::string camera_name = "slicev";
      std::atomic<bool> update;  ///<! set if the menus need to be updated
      std::string push_settings; ///<! name of script to push settings to GUI
      std::string push_image;    ///<! name of script to push an image to GUI

    public:
      GUIManager() : update(false), exptime(NAN), gain(-1), bin(-1), navg(NAN) { }

      // These are the GUIDER GUI settings
      //
      float exptime;
      int gain;
      int bin;
      float navg;

      // sets the private variable push_settings, call on config
      inline void set_push_settings( std::string sh ) { this->push_settings=sh; }

      // sets the private variable push_image, call on config
      inline void set_push_image( std::string sh ) { this->push_image=sh; }

      // sets the update flag true
      inline void set_update() { this->update.store( true ); }

      /**
       * @fn         get_update
       * @brief      returns the update flag then clears it
       * @return     boolean true|false
       */
      inline bool get_update() { return this->update.exchange( false ); }

      /**
       * @fn         get_message_string
       * @brief      returns a formatted message of all gui settings
       * @details    This message is the return string to guideset command.
       * @return     string in form of <exptime> <gain> <bin>
       */
      std::string get_message_string() {
        std::ostringstream oss;
        if ( this->exptime < 0 ) oss << "ERR"; else { oss << std::fixed << std::setprecision(3) << this->exptime; }
        oss << " ";
        if ( this->gain < 1 ) oss << "ERR"; else { oss << std::fixed << std::setprecision(3) << this->gain; }
        oss << " ";
        if ( this->bin < 1 ) oss << "x"; else { oss << std::fixed << std::setprecision(3) << this->bin; }
        oss << " ";
        if ( std::isnan(this->navg) ) oss << "NaN"; else { oss << std::fixed << std::setprecision(2) << this->navg; }
        return oss.str();
      }

      /**
       * @brief      calls the push_settings script with the formatted message string
       * @details    the script pushes the settings to the Guider GUI
       */
      void push_gui_settings() {
        const std::string function("Slicecam::GUIManager::push_gui_settings");
        std::ostringstream cmd;
        cmd << push_settings << " "
            << ( get_update() ? "true" : "false" ) << " "
            << get_message_string();

        if ( std::system( cmd.str().c_str() ) && errno!=ECHILD ) {
          logwrite( function, "ERROR updating GUI" );
        }
      }

      void send_fifo_warning(const std::string &message) {
        const std::string fifo_name("/tmp/.slicev_warning.fifo");
        std::ofstream fifo(fifo_name);
        if (!fifo.is_open()) {
          logwrite("Slicecam::GUIManager::send_fifo_warning", "failed to open " + fifo_name + " for writing");
        }
        else {
          fifo << message << std::endl;
          fifo.close();
        }
      }

      /**
       * @brief      calls the push_image script with the formatted message string
       * @details    the script pushes the indicated file to the Guider GUI display
       * @param[in]  filename  fits file to send
       */
      void push_gui_image( std::string_view filename ) {
        const std::string function("Slicecam::GUIManager::push_gui_image");
        std::ostringstream cmd;
        cmd << push_image << " "
            << camera_name << " "
            << filename;

        if ( std::system( cmd.str().c_str() ) && errno!=ECHILD ) {
          logwrite( function, "ERROR pushing image to GUI" );
        }
      }
  };
  /***** Slicecam::GUIManager *************************************************/
}
