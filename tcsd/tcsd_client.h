/** ---------------------------------------------------------------------------
 * @file    tcsd_client.h
 * @brief   header for TcsDaemonClient
 * @details TcsDaemonClient class is used to create a client for communicating
 *          with the TCS Daemon. It instantiates a Common::DaemonClient object
 *          which provides the actual communication and includes a set of
 *          functions which make use of that client communication object.
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "common.h"
#include "tcs_constants.h"
#include "tcsd_commands.h"

  /***** TcsDaemonClient ******************************************************/
  /**
   * @class TcsDaemonClient
   * @brief defines a client object for communicating with tcsd
   */
  class TcsDaemonClient {

    public:
      Common::DaemonClient client { "tcsd", '\n', '\n' };

      long init( std::string_view which, std::string &retstring );
      long get_cass( double &cass );
      long set_focus( double &value );
      long get_focus( double &value );
      long extract_value( std::string tcs_message, int &value );
      long parse_generic_reply( int value );
      long poll_dome_position( double &domeazi, double &telazi );
      long get_dome_position( double &domeazi, double &telazi );
      long get_dome_position( bool poll, double &domeazi, double &telazi );
      long get_coords( double &ra_h, double &dec_d );
      long get_weather_coords( double &ra_h, double &dec_d );
      long get_coords_type( std::string cmd, double &ra_h, double &dec_d );

      double radec_to_decimal( std::string str_in );
      double radec_to_decimal( std::string str_in, std::string &retstring );

  };
  /***** TcsDaemonClient ******************************************************/
