#include "camera_interface.h"

namespace Camera {

  // this is the code for shared functions common to all implementations

  void Interface::set_server(Camera::Server* s) {
    this->server=s;
  }

  /***** Camera::Interface::handle_queue **************************************/
  /**
   * @brief      inserts a message into the asynchronous message queue
   * @param[in]  message  string to insert into the async message queue
   *
   * This should be spawned in a thread. This is how threads get messages
   * into the async message queue.
   *
   */
  void Interface::handle_queue(std::string message) {
    this->server.async.enqueue( message );
    return;
  }
  /***** Camera::Interface::handle_queue **************************************/


  void Interface::func_shared() {
    std::string function("Camera::Interface::func_shared");
    logwrite(function, "common implementation function");
  }

  /***** Camera::Interface::disconnect_controller *****************************/
  /**
   * @brief      disconnect camera controller
   * @details    use this to disconnect before exiting because it takes
   *             no arguments and returns nothing
   *
   */
  void Interface::disconnect_controller() {
    std::string retstring;
    this->disconnect_controller("", retstring);
  }
  /***** Camera::Interface::disconnect_controller *****************************/
}
