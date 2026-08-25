#include "focus.h"

namespace Focus {

  /**************** Focus::Interface::Interface *******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Focus::Interface::Interface *******************************/


  /**************** Focus::Interface::~Interface ******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Focus::Interface::~Interface ******************************/

  /***** Focus::Interface::do_home *********************************************/
  /**
   * @fn         do_home
   * @brief      thread to emulate homing (just sleeps)
   * @param[in]  reference to Focus::ControllerInfo object
   * @param[in]  reference to mutex
   * @return     none
   *
   */
  void Interface::do_home( Focus::ControllerInfo &info, std::mutex &mlock ) {
    std::string function = "  (Focus::Interface::do_home) ";

    for ( int i = 0; i < 15; i++ ) {
      std::cerr << get_timestamp() << function << "homing " << info.name << "... \n";
      usleep( 500000 );
    }

    mlock.lock();
    info.homed = true;
    info.ontarget = true;
    info.pos = 0.0;
    mlock.unlock();

    std::cerr << get_timestamp() << function << "home " << info.name << " complete!\n";
  }
  /***** Focus::Interface::do_home *********************************************/


  /***** Focus::Interface::do_move *********************************************/
  /**
   * @fn         do_move
   * @brief      thread to emulate moving (just sleeps)
   * @param[in]  reference to Focus::ControllerInfo object
   * @param[in]  reference to mutex
   * @param[in]  int distance to move
   * @param[in]  float final position
   * @return     none
   *
   */
  void Interface::do_move( Focus::ControllerInfo &info, std::mutex &mlock, int distance, float pos ) {
    std::string function = "  (Focus::Interface::do_move) ";

    for ( int i = 0; i < distance; i++ ) {
      std::cerr << get_timestamp() << function << "moving " << info.name << "... \n";
      usleep( 500000 );
    }

    mlock.lock();
    info.ontarget = true;
    info.pos = pos;
    mlock.unlock();

    std::cerr << get_timestamp() << function << "move " << info.name << " complete!\n";
  }
  /***** Focus::Interface::do_move *********************************************/


  /**************** Focus::Interface::parse_command ***************************/
  /**
   * @fn         parse_command
   * @brief      parse commands received from focusd and emulate a PI response
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   * Commands arrive in the form "<addr> <cmd> [<axis>] [<pos>]", matching
   * Physik_Instrumente::Interface's own command construction (PI/pi.cpp).
   * Only the PI command subset used by focusd for I/R/G is handled here;
   * U/GALIL has no protocol emulation.
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (Focus::Interface::parse_command) ";
    int myaddr = -1;
    int mydev = -1;
    int myaxis = 1;
    std::string mycmd;
    float mypos = 0.0;

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    try {
      if ( tokens.size() > 0 ) {
        myaddr = std::stoul( tokens.at( 0 ) );
      }
      if ( tokens.size() > 1 ) {
        mycmd = tokens.at( 1 );
      }
      if ( tokens.size() > 2 ) {
        myaxis = std::stof( tokens.at( 2 ) );
      }
      if ( tokens.size() > 3 ) {
        mypos = std::stof( tokens.at( 3 ) );
      }
      if ( tokens.size() < 1 || tokens.size() > 4 ) {
        std::cerr << get_timestamp() << function << "ERROR: received " << tokens.size() << " args but expected 1, 2, 3, or 4\n";
        return ( ERROR );
      }
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values: " << e.what() << "\n";
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more values out of range: " << e.what() << "\n";
      return ( ERROR );
    }

    for ( size_t dev = 0; dev < this->controller_info.size(); dev++ ) {
      if ( this->controller_info.at( dev ).addr == myaddr ) {
        mydev = dev;
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

      std::cerr << get_timestamp() << function << "spawning do_home thread for " << this->controller_info.at( mydev ).name
                << "\n";

      std::thread( std::ref( Focus::Interface::do_home ), std::ref( this->controller_info.at( mydev ) ),
                   std::ref( this->pos_mutex ) )
          .detach();
    }
    else

      // FRF?
      //
      if ( mycmd == "FRF?" ) {
        this->pos_mutex.lock();
        retstring = std::to_string( myaxis ) + "=" + ( this->controller_info.at( mydev ).homed ? "1" : "0" );
        this->pos_mutex.unlock();
      }
      else

        // POS?
        //
        if ( mycmd == "POS?" ) {
          this->pos_mutex.lock();
          retstring = std::to_string( myaxis ) + "=" + std::to_string( this->controller_info.at( mydev ).pos );
          this->pos_mutex.unlock();
        }
        else

          // MOV
          //
          if ( mycmd == "MOV" ) {
            this->pos_mutex.lock();
            this->controller_info.at( mydev ).ontarget = false;
            int distance = (int)( std::abs( this->controller_info.at( mydev ).pos - mypos ) );
            this->pos_mutex.unlock();

            std::cerr << get_timestamp() << function << "spawning do_move thread for " << this->controller_info.at( mydev ).name
                      << "\n";

            std::thread( std::ref( Focus::Interface::do_move ), std::ref( this->controller_info.at( mydev ) ),
                         std::ref( this->pos_mutex ), distance, mypos )
                .detach();
          }
          else

            // ONT?
            //
            if ( mycmd == "ONT?" ) {
              this->pos_mutex.lock();
              retstring = std::to_string( myaxis ) + "=" + ( this->controller_info.at( mydev ).ontarget ? "1" : "0" );
              this->pos_mutex.unlock();
            }
            else

              // SVO
              //
              if ( mycmd == "SVO" ) {
                std::cerr << get_timestamp() << function << "servo on\n";
              }
              else

                // ERR?
                //
                if ( mycmd == "ERR?" ) {
                  std::stringstream ss;
                  ss << myaddr << "=0"; // always return no error
                  retstring = ss.str();
                }

                // unknown command
                //
                else {
                  std::cerr << get_timestamp() << function << "ignored unknown command: " << mycmd << "\n";
                  retstring = "unknown_command";
                }

    std::cerr << get_timestamp() << function << "reply from focus emulator: " << retstring << "\n";

    return ( NO_ERROR );
  }
  /**************** Focus::Interface::parse_command ***************************/

}
