#ifndef SEQUENCER_INTERFACE_H
#define SEQUENER_INTERFACE_H

#include <map>

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

#define ERROR_TARGETLIST_BAD_HEADER 1001  // TODO change this

namespace Sequencer {

  /** TcpDevice ***************************************************************/
  /**
   * @class  TcpDevice
   * @brief  TCP device class
   *
   */
  class TcpDevice {
    private:
      std::string host;
      int port;
      bool connected;
    public:
      TcpDevice();
      ~TcpDevice();

      Network::TcpSocket socket;

      long send( std::string command, std::string &reply );
  };
  /** TcpDevice ***************************************************************/


  /** Daemon ******************************************************************/
  /**
   * @class  Daemon
   * @brief  daemon class
   *
   */
  class Daemon {
    private:
      std::string host;
      int port;
      bool connected;
      std::string name;
    public:
      Daemon();
      ~Daemon();

      Network::TcpSocket socket;    /// socket object for communications with daemon

      long command( std::string args );                          /// commands to daemon
      long command( std::string args, std::string &retstring );  /// commands to daemon that need a reply
      long send( std::string command, std::string &reply );      /// for internal use only
      long connect();                                            /// initialize socket connection to daemon
      void configure( std::string name, int port );              /// configure communication parameters
  };
  /** Daemon ******************************************************************/


  /** TargetInfo **************************************************************/
  /**
   * @class  TargetInfo
   * @brief  target information class
   *
   * This class contains the information for a target row
   * and how to read it, etc.
   *
   */
  class TargetInfo {
    private:
    public:
      TargetInfo();
      ~TargetInfo();

      enum TargetState {
        TARGET_NOT_FOUND=0,
        TARGET_FOUND
      };

      const float RA_MIN = 0;             /// minimum target right ascension
      const float RA_MAX = 24;            /// maximum target right ascension
      const float DEC_MIN = -90;          /// minimum target declination
      const float DEC_MAX = 90;           /// maximum target declination
      const float SLIT_MIN = 0;           /// minimum slit width
      const float SLIT_MAX = 10;          /// maximum slit width
      const long  EXPTIME_MIN = 0;        /// minimum value for exptime
      const long  EXPTIME_MAX = 1 << 24;  /// maximum value for exptime
      const long  REPEAT_MIN = 0;         /// minimum value for repeat
      const long  REPEAT_MAX = 1 << 24;   /// maximum value for repeat

      std::string name;                   /// target name
      float       ra;                     /// target right ascension
      float       dec;                    /// target declination
      std::string epoch;                  /// target coordinates epoch
      float       slitwidth;              /// slit width for this target
      float       slitoffset;             /// slit offset for this target
      long        exptime;                /// exposure time in seconds
      long        repeat;                 /// number of repeat exposures on this target
      std::string obsplan;                /// TBD
      int         binspect;               /// binning in spectral direction
      int         binspat;                /// binning in spatial direction

      long get_next();                    /// get the next target from the database

  };
  /** TargetInfo **************************************************************/

}
#endif
