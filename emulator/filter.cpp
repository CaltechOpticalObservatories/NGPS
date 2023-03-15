#include "filter.h"

namespace Filter {

  /**************** Filter::ControllerInfo::ControllerInfo ********************/
  /**
   * @fn         ControllerInfo
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  ControllerInfo::ControllerInfo() {
    this->homed    = false;
    this->ontarget = false;
    this->pos      = -1;
  }
  /**************** Filter::ControllerInfo::ControllerInfo ********************/


  /**************** Filter::ControllerInfo::~ControllerInfo *******************/
  /**
   * @fn         ~ControllerInfo
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  ControllerInfo::~ControllerInfo() {
  }
  /**************** Filter::ControllerInfo::~ControllerInfo *******************/


  /**************** Filter::Interface::Interface ******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Filter::Interface::Interface ******************************/


  /**************** Filter::Interface::~Interface *****************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Filter::Interface::~Interface *****************************/


  long Interface::test() {
    std::string function = "  (Filter::Interface::test) ";
    std::cerr << get_timestamp() << function << "controller_info.size() = " << this->controller_info.size() << "\n";
    return( NO_ERROR );
  }


  /**************** Filter::Interface::do_home ********************************/
  /**
   * @fn         do_home
   * @brief      thread to emulate homing (just sleeps)
   * @param[in]  reference to Filter::ControllerInfo object
   * @param[in]  reference to mutex
   * @return     ERROR or NO_ERROR
   *
   */
  void Interface::do_home( Filter::ControllerInfo &info, std::mutex &mlock ) {
    std::string function = "  (Filter::Interface::do_home) ";

    // sleep here
    //
    for ( int i = 0; i < 15; i++ ) {
      std::cerr << get_timestamp() << function << "homing " << info.name << "... \n";
      usleep( 500000 );
    }

    // after "home" complete, reset pos and set the home and ontarget flags
    //
    mlock.lock();
    info.homed    = true;
    info.ontarget = true;
    info.pos      = 0.0;
    mlock.unlock();

    std::cerr << get_timestamp() << function << "home " << info.name << " complete!\n";
  }
  /**************** Filter::Interface::do_home ********************************/


  /**************** Filter::Interface::do_move ********************************/
  /**
   * @fn         do_move
   * @brief      thread to emulate moving (just sleeps)
   * @param[in]  reference to Filter::ControllerInfo object
   * @param[in]  reference to mutex
   * @param[in]  int distance to move
   * @param[in]  float final position
   * @return     ERROR or NO_ERROR
   *
   */
  void Interface::do_move( Filter::ControllerInfo &info, std::mutex &mlock, int distance, float pos ) {
    std::string function = "  (Filter::Interface::do_move) ";

    // sleep here
    //
    for ( int i = 0; i < distance; i++ ) {
      std::cerr << get_timestamp() << function << "moving " << info.name << "... \n";
      usleep( 500000 );
    }

    // after "move" complete, reset pos and set the ontarget flags
    //
    mlock.lock();
    info.ontarget = true;
    info.pos      = pos;
    mlock.unlock();

    std::cerr << get_timestamp() << function << "move " << info.name << " complete!\n";
  }
  /**************** Filter::Interface::do_move ********************************/


  /**************** Filter::Interface::parse_command **************************/
  /**
   * @fn         parse_command
   * @brief      parse incomming commands
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (Filter::Interface::parse_command) ";
    int myaddr=-1;
    int mydev=-1;
    std::string mycmd="";
    float mypos=0.0;

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    try {
      if ( tokens.size() > 0 ) {
        myaddr = std::stoul( tokens.at(0) );
      }
      if ( tokens.size() > 1 ) {
        mycmd = tokens.at(1);
      }
      if ( tokens.size() > 2 ) {
        mypos = std::stof( tokens.at(2) );
      }
      if ( tokens.size() < 1 || tokens.size() > 3 ) {
        std::cerr << get_timestamp() << function << "ERROR: received " << tokens.size() << " args but expected 1, 2, or 3\n";
        return( ERROR );
      }
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values: " << e.what() << "\n";
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more values out of range: " << e.what() << "\n";
      return( ERROR );
    }

    if ( myaddr < 1 || myaddr > (long)this->controller_info.size() ) {
      std::cerr << get_timestamp() << function << "bad address " << myaddr 
                << ". expected {0:" << this->controller_info.size() << "}\n";
      return( ERROR );
    }

    for ( size_t dev = 0; dev < this->controller_info.size(); dev++ ) {
      if ( this->controller_info.at(dev).addr == myaddr ) {
        mydev  = dev;
        std::cerr << get_timestamp() << function << "myaddr=" << myaddr << " mydev=" << mydev << "\n";
        break;
      }
    }

    if ( mydev < 0 ) {
      std::cerr << get_timestamp() << function << "ERROR: addr " << myaddr << " not found in controller list\n";
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */

    // FRF
    //
    if ( mycmd == "FRF" ) {
      this->pos_mutex.lock();
      this->controller_info.at( mydev ).homed = false;
      this->controller_info.at( mydev ).ontarget = false;
      this->pos_mutex.unlock();

      std::cerr << get_timestamp() << function << "spawning do_home thread for " << this->controller_info.at( mydev).name << "\n";

      std::thread( std::ref(Filter::Interface::do_home), 
                   std::ref(this->controller_info.at( mydev )),
                   std::ref(this->pos_mutex) ).detach();

    }
    else

    // FRF?
    //
    if ( mycmd == "FRF?" ) {
      this->pos_mutex.lock();
      retstring = this->controller_info.at( mydev ).homed ? "1" : "0";
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << "homed = " << retstring << "\n";
    }
    else

    // POS?
    //
    if ( mycmd == "POS?" ) {
      std::cerr << get_timestamp() << function << "got POS? command. mydev=" << mydev << "\n";
      this->pos_mutex.lock();
      retstring = std::to_string( this->controller_info.at( mydev ).pos );
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << "pos = " << retstring << "\n";
    }
    else

    // MOV
    //
    if ( mycmd == "MOV" ) {
      this->pos_mutex.lock();
      this->controller_info.at( mydev ).ontarget = false;
      int distance = (int) ( std::abs( this->controller_info.at( mydev ).pos - mypos ) );
      this->pos_mutex.unlock();

      std::cerr << get_timestamp() << function << "spawning do_move thread for " << this->controller_info.at( mydev).name << "\n";

      std::thread( std::ref(Filter::Interface::do_move),
                   std::ref(this->controller_info.at( mydev )),
                   std::ref(this->pos_mutex),
                   distance,
                   mypos ).detach();
    }
    else

    // ONT?
    //
    if ( mycmd == "ONT?" ) {
      this->pos_mutex.lock();
      retstring = this->controller_info.at( mydev ).ontarget ? "1" : "0";
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << "ontarget = " << retstring << "\n";
    }

    // unknown command
    //
    else {
      std::cerr << get_timestamp() << function << "ignored unknown command: " << mycmd << "\n";
    }

    return ( NO_ERROR );
  }
  /**************** Filter::Interface::parse_command **************************/

}
