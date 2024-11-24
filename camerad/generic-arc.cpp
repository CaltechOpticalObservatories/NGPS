/**
 * @file    generic-arc.cpp
 * @brief   this class calls functions for the real ARC (Leach) controller
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "camerad.h"

namespace AstroCam {

  long Interface::abort() {
    logwrite( "AstroCam::Interface::abort", "[DEBUG] calling do_abort()" );
    return this->do_abort();
  }


  /***** AstroCam::Interface::bin *********************************************/
  /**
   * @brief      wrapper for do_bin
   * @details    set or get the binning factor for the specified direction
   * @param[in]  args       argument string contains <chan> <axis> [ <factor> ]
   * @param[out] retstring  return string
   * @return     ERROR or NO_ERROR
   *
   * The bindir string can be implementation-specific.
   *
   */
  long Interface::bin( std::string args, std::string &retstring ) {
    return this->do_bin( args, retstring );
  }
  /***** AstroCam::Interface::bin *********************************************/


  /***** AstroCam::Interface::connect_controller ******************************/
  /**
   * @brief      wrapper for do_connect_controller
   * @details    opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @param[out] help        reference to string to return help on request
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::connect_controller(std::string devices_in, std::string &help) {
    return this->do_connect_controller( devices_in, help );
  }
  /***** AstroCam::Interface::connect_controller ******************************/


  /***** AstroCam::Interface::disconnect_controller ***************************/
  /**
   * @brief      wrapper for do_disconnect_controller
   * @details    closes the connection to the PCI/e device(s)
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::disconnect_controller() {
    return this->do_disconnect_controller( );
  }
  /***** AstroCam::Interface::disconnect_controller ***************************/


  /***** AstroCam::Interface::load_firmware ***********************************/
  /**
   * @brief      wrapper for do_load_firmware
   * @details    load firmware (.lod) into one or all controller timing boards
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::load_firmware(std::string &retstring) {
    return this->do_load_firmware( retstring );
  }
  /***** AstroCam::Interface::load_firmware ***********************************/


  /***** AstroCam::Interface::load_firmware ***********************************/
  /**
   * @brief      wrapper for do_load_firmware
   * @details    load firmware (.lod) into one or all controller timing boards
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::load_firmware(std::string timlodfile, std::string &retstring) {
    return this->do_load_firmware( timlodfile, retstring );
  }
  /***** AstroCam::Interface::load_firmware ***********************************/


  /***** AstroCam::Interface::configure_controller ****************************/
  /**
   * @brief      wrapper for do_configure_controller
   * @details    perform initial configuration of controller from .cfg file
   * @return     ERROR or NO_ERROR
   *
   * Called automatically by main() when the server starts up.
   *
   */
  long Interface::configure_controller() {
    return this->do_configure_controller();
  }
  /***** AstroCam::Interface::configure_controller ****************************/


  /***** AstroCam::Interface::expose ****************8*************************/
  /**
   * @brief      wrapper for do_expose
   * @details    initiate an exposure
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::expose( std::string nseq_in ) {
    return this->do_expose( nseq_in );
  }
  /***** AstroCam::Interface::expose ****************8*************************/


  /***** AstroCam::Interface::exptime ***************8*************************/
  /**
   * @brief      wrapper for do_exptime
   * @details    set/get the exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    return this->do_exptime( exptime_in, retstring );
  }
  /***** AstroCam::Interface::exptime ***************8*************************/


  /***** AstroCam::Interface::modify_exptime ********8*************************/
  /**
   * @brief      wrapper for do_modify_exptime
   * @details    modify the exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::modify_exptime( std::string exptime_in, std::string &retstring ) {
    return this->do_modify_exptime( exptime_in, retstring );
  }
  /***** AstroCam::Interface::modify_exptime ********8*************************/


  /***** AstroCam::Interface::geometry **************8*************************/
  /**
   * @brief      wrapper for do_geometry
   * @details    
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::geometry( std::string args, std::string &retstring ) {
    return this->do_geometry( args, retstring );
  }
  /***** AstroCam::Interface::geometry **************8*************************/


  /***** AstroCam::Interface::readout ***************8*************************/
  /**
   * @brief      wrapper for do_readout
   * @details    set or get type of readout
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::readout( std::string readout_in, std::string &readout_out ) {
    return this->do_readout( readout_in, readout_out );
  }
  /***** AstroCam::Interface::readout ***************8*************************/


  /***** AstroCam::Interface::native ****************8*************************/
  /**
   * @brief      wrapper for do_native
   * @details    send a 3-letter command to all or specified Leach controller(s)
   * @param[in]  cmdstr  command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string cmdstr ) {
    std::string dontcare;
    return this->do_native( cmdstr, dontcare );
  }
  /***** AstroCam::Interface::native ****************8*************************/


  /***** AstroCam::Interface::native ****************8*************************/
  /**
   * @brief      wrapper for do_native
   * @details    send a 3-letter command to all or specified Leach controller(s)
   *             and return reply
   * @param[in]  cmdstr     command to send
   * @param[out] retstring  reference to string to contain reply
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string cmdstr, std::string &retstring ) {
    return this->do_native( cmdstr, retstring );
  }
  /***** AstroCam::Interface::native ****************8*************************/

  long Interface::_image_size( std::string args, std::string &retstring, const bool save_as_default ) {
    return this->image_size( args, retstring, save_as_default );
  }
}
