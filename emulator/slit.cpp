#include "slit.h"

namespace Slit {

  /**************** Slit::ControllerInfo::ControllerInfo **********************/
  /**
   * @fn         ControllerInfo
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  ControllerInfo::ControllerInfo() {
    this->servo    = false;
    this->homed    = false;
    this->ontarget = false;
    this->pos      = -1;
  }
  /**************** Slit::ControllerInfo::ControllerInfo **********************/


  /**************** Slit::ControllerInfo::~ControllerInfo *********************/
  /**
   * @fn         ~ControllerInfo
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  ControllerInfo::~ControllerInfo() {
  }
  /**************** Slit::ControllerInfo::~ControllerInfo *********************/


  /**************** Slit::Interface::Interface ********************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Slit::Interface::Interface ********************************/


  /**************** Slit::Interface::~Interface *******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Slit::Interface::~Interface *******************************/


  long Interface::test() {
    std::string function = "  (Slit::Interface::test) ";
    std::cerr << get_timestamp() << function << "controller_info.size() = " << this->controller_info.size() << "\n";
    return( NO_ERROR );
  }


  /**************** Slit::Interface::do_home **********************************/
  /**
   * @fn         do_home
   * @brief      thread to emulate homing (just sleeps)
   * @param[in]  reference to Slit::ControllerInfo object
   * @param[in]  reference to mutex
   * @return     ERROR or NO_ERROR
   *
   */
  void Interface::do_home( Slit::ControllerInfo &info, std::mutex &mlock ) {
    std::string function = "  (Slit::Interface::do_home) ";

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
  /**************** Slit::Interface::do_home **********************************/


  /**************** Slit::Interface::do_move **********************************/
  /**
   * @fn         do_move
   * @brief      thread to emulate moving (just sleeps)
   * @param[in]  reference to Slit::ControllerInfo object
   * @param[in]  reference to mutex
   * @param[in]  int distance to move
   * @param[in]  float final position
   * @return     ERROR or NO_ERROR
   *
   */
  void Interface::do_move( Slit::ControllerInfo &info, std::mutex &mlock, int distance, float pos ) {
    std::string function = "  (Slit::Interface::do_move) ";

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
  /**************** Slit::Interface::do_move **********************************/


  /**************** Slit::Interface::parse_command ****************************/
  /**
   * @fn         parse_command
   * @brief      parse incomming command
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (Slit::Interface::parse_command) ";
    int myaddr=-1;
    int mydev=-1;
    int myaxis=1;
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
        myaxis = std::stof( tokens.at(2) );
      }
      if ( tokens.size() > 3 ) {
        mypos = std::stof( tokens.at(3) );
      }
      if ( tokens.size() < 1 || tokens.size() > 4 ) {
        std::cerr << get_timestamp() << function << "ERROR: received " << tokens.size() << " args but expected 1, 2, 3, or 4\n";
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
        std::cerr << get_timestamp() << function << "myaddr=" << myaddr << " mydev=" << mydev << " myaxis=" << myaxis << "\n";
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
    if ( mycmd == "FRF" || mycmd == "FNL" || mycmd == "FPL" ) {
      this->pos_mutex.lock();
      this->controller_info.at( mydev ).homed = false;
      this->controller_info.at( mydev ).ontarget = false;
      this->pos_mutex.unlock();

      std::cerr << get_timestamp() << function << "spawning do_home thread for " << this->controller_info.at( mydev).name << "\n";

      std::thread( std::ref(Slit::Interface::do_home), 
                   std::ref(this->controller_info.at( mydev )),
                   std::ref(this->pos_mutex) ).detach();

    }
    else

    // FRF?
    //
    if ( mycmd == "FRF?" ) {
      this->pos_mutex.lock();
      std::stringstream ss;
      ss << "0 " << myaddr << " " << myaxis << "=" << ( this->controller_info.at( mydev ).homed ? "1" : "0" );
      retstring = ss.str();
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << retstring << "\n";
    }
    else

    // POS?
    //
    if ( mycmd == "POS?" ) {
      std::cerr << get_timestamp() << function << "got POS? command. mydev=" << mydev << "\n";
      this->pos_mutex.lock();
      std::stringstream ss;
      ss << "0 " << myaddr << " " << myaxis << "=" << std::to_string( this->controller_info.at( mydev ).pos );
      retstring = ss.str();
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << retstring << "\n";
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

      std::thread( std::ref(Slit::Interface::do_move),
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
      std::stringstream ss;
      ss << "0 " << myaddr << " " << myaxis << "=" << std::to_string( this->controller_info.at( mydev ).ontarget );
      retstring = ss.str();
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << retstring << "\n";
    }

    else

    // ERR?
    //
    if ( mycmd == "ERR?" ) {
      this->pos_mutex.lock();
      std::stringstream ss;
      ss << "0 " << myaddr << " " << myaxis << "=0";  // always return no error
      retstring = ss.str();
      this->pos_mutex.unlock();
      std::cerr << get_timestamp() << function << retstring << "\n";
    }

    // unknown command
    //
    else {
      std::cerr << get_timestamp() << function << "ignored unknown command: " << mycmd << "\n";
    }

    return ( NO_ERROR );
  }
  /**************** Slit::Interface::parse_command ****************************/

}
