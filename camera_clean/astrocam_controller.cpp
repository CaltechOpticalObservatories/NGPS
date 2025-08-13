
#include "astrocam_controller.h"
#include "astrocam_interface.h"

namespace Camera {

  /***** Camera::Callback::exposeCallback *************************************/
  /**
   * @brief      called by CArcDevice::expose() during the exposure
   * @param[in]  devnum         device number passed to API on expose
   * @param[in]  uiElapsedTime  actually number of millisec remaining
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() during the exposure.
   * After sending the expose (SEX) command, the API polls the controller
   * using the RET command.
   *
   */
  void Callback::exposeCallback( int devnum, std::uint32_t uiElapsedTime, std::uint32_t uiExposureTime ) {
    std::string function = "Camera::Callback::exposeCallback";
    std::stringstream message;
    message << "ELAPSEDTIME_" << devnum << ":" << uiElapsedTime;
    if ( uiExposureTime != 0x1BAD1BAD ) message<< " EXPTIME:" << uiExposureTime;
    std::thread( &Camera::AstroCamInterface::handle_queue, interface, message.str() ).detach();
    return;
  }
  /***** Camera::Callback::exposeCallback *************************************/


  /***** Camera::Callback::readCallback ***************************************/
  /**
   * @brief      called by CArcDevice::expose() during readout
   * @param[in]  expbuf        exposure buffer number passed to API on expose
   * @param[in]  devnum        device number passed to API on expose
   * @param[in]  uiPixelCount  number of pixels read ( getPixelCount() )
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() when bInReadout is true.
   *
   */
  void Callback::readCallback( int expbuf, int devnum, std::uint32_t uiPixelCount, std::uint32_t uiImageSize ) {
    std::string message;
    message.reserve(36);
    message = "PIXELCOUNT_" + std::to_string(devnum)       + ":"
                            + std::to_string(uiPixelCount) + " "
                            + std::to_string(uiImageSize)  + " "
                            + std::to_string(static_cast<long>(100*uiPixelCount/uiImageSize));
    std::thread( &Camera::AstroCamInterface::handle_queue, interface, message ).detach();
    return;
  }
  /***** Camera::Callback::readCallback ***************************************/


  /***** Camera::Callback::frameCallback **************************************/
  /**
   * @brief      called by CArcDevice::expose() when a frame has been received
   * @param[in]  expbuf    exposure buffer number passed to API on expose
   * @param[in]  devnum    device number passed to API on expose
   * @param[in]  fpbcount  frames per buffer count ( uiFPBCount ) wraps to 0 at FPB
   * @param[in]  fcount    actual frame counter ( uiPCIFrameCount = getFrameCount() )
   * @param[in]  rows      image rows ( uiRows )
   * @param[in]  cols      image columns ( uiCols )
   * @param[in]  buffer    pointer to PCI image buffer
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() when a new frame is received.
   * Spawn a separate thread to handle the incoming frame.
   *
   * incoming frame from ARC -> frameCallback()
   *
   */
  void Callback::frameCallback( int expbuf, int devnum, std::uint32_t fpbcount, std::uint32_t fcount,
                                std::uint32_t rows, std::uint32_t cols, void* buffer ) {
    if ( ! interface->useframes() ) fcount=1;  // prevents fcount from being a bad value when firmware doesn't support frames
    std::stringstream message;

    if ( interface->controller.find(devnum) == interface->controller.end() ) {
      message.str(""); message << "ERROR in AstroCam::Callback::frameCallback (fatal): devnum " << devnum
                               << " not in controller configuration";
      std::thread( &Camera::AstroCamInterface::handle_queue, interface, message.str() ).detach();
      return;
    }

    message << "FRAMECOUNT_" << devnum << ":" << fcount << " rows=" << rows << " cols=" << cols;
    std::thread( &Camera::AstroCamInterface::handle_queue, interface, message.str() ).detach();
    logwrite( "Camera::Callback::frameCallback", message.str() );

    interface->add_framethread();            // framethread_count is incremented because a thread has been added

    std::thread( &Camera::AstroCamInterface::handle_frame, interface, expbuf, devnum, fpbcount, fcount, buffer ).detach();

    return;
  }
  /***** Camera::Callback::frameCallback **************************************/


  Controller::Controller() : 
    bufsize(0), framecount(0), workbuf_size(0), workbuf(nullptr)
  {
  }

}
