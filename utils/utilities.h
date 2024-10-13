/**
 * @file    utilities.h
 * @brief   some handy utilities to use anywhere
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <iomanip>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <iostream>  // for istream
#include <fstream>   // for ifstream
#include <iterator>  // for istream_iterator
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <cmath>
#include <sys/stat.h>
#include "md5.h"
#include <string>
#include <string_view>
#include <cctype>
#include <set>
#include <limits>
#include <ctime>
#include "time.h"

#define TO_DEGREES ( 360. / 24. )
#define TO_HOURS   ( 24. / 360. )

extern std::string tmzone_cfg;                      /// time zone if set in cfg file
extern std::mutex generate_tmpfile_mtx;

bool cmdOptionExists( char** begin, char** end, const std::string &option );
char* getCmdOption( char** begin, char** end, const std::string &option );
int my_hardware_concurrency();
int cores_available();

inline int mod( int k, int n ) { return ( (k %= n) < 0 ) ? k+n : k; }

unsigned int parse_val(const std::string& str);     /// returns an unsigned int from a string

int Tokenize(const std::string& str, 
             std::vector<std::string>& tokens, 
             const std::string& delimiters);        /// break a string into a vector

void Tokenize(const std::string &str, 
              std::vector<uint32_t> &devlist, 
              int &ndev, 
              std::vector<std::string> &arglist, 
              int &narg );

void chrrep(char *str, char oldchr, char newchr);   /// replace one character within a string with a new character
void string_replace_char(std::string &str, const char *oldchar, const char *newchar);

long get_time( int &year, int &mon, int &mday, int &hour, int &min, int &sec, int &usec );
long get_time( const std::string &tmzone_in, int &year, int &mon, int &mday, int &hour, int &min, int &sec, int &usec );

std::string timestamp_from( struct timespec &time_n );  /// return time from input timespec struct in formatted string "YYYY-MM-DDTHH:MM:SS.sss"
std::string timestamp_from( const std::string &tmzone_in, struct timespec &time_in );

inline std::string get_timestamp(const std::string &tz) {  /// return current time in formatted string "YYYY-MM-DDTHH:MM:SS.sss"
  struct timespec timenow;
  clock_gettime( CLOCK_REALTIME, &timenow );
  return timestamp_from( tz, timenow );
}
inline std::string get_timestamp() {                /// return current time in formatted string "YYYY-MM-DDTHH:MM:SS.sss"
  return get_timestamp(tmzone_cfg);
}
inline std::string get_datetime() {
  /// return current time in formatted string "YYYY-MM-DD HH:MM:SS.sss"
  std::string str = get_timestamp(tmzone_cfg);
  str[10] = ' ';
  return str;
}

std::string get_system_date();                      /// return current date in formatted string "YYYYMMDD"
std::string get_system_date( const std::string &tmzone_in );

std::string get_file_time();                        /// return current time in formatted string "YYYYMMDDHHMMSS" used for filenames
std::string get_file_time( const std::string &tmzone_in );

double get_clock_time();

double get_time_as_double();
std::string datetime_from_double( double &time );

void precise_sleep( long microseconds );             /// precise, non-interruptable sleep timer for short durrations

long timeout( int wholesec=0, std::string next="" );  /// wait until next integral second or minute

double mjd_from( struct timespec &time_n );         /// modified Julian date from input timespec struct

inline double mjd_now() {                           /// modified Julian date now
  struct timespec timenow;
  clock_gettime( CLOCK_REALTIME, &timenow );
  return( mjd_from( timenow ) );
}

int compare_versions(const std::string &v1, const std::string &v2);

long md5_file( const std::string &filename, std::string &hash );  /// compute md5 checksum of file

bool is_owner( const std::filesystem::path &filename );
bool has_write_permission( const std::filesystem::path &filename );
const std::string &tchar( const std::string &str );
const std::string strip_newline( const std::string &str_in );
std::string strip_control_characters( const std::string &str );
bool starts_with( const std::string &str, std::string_view prefix );
bool ends_with( const std::string &str, std::string_view suffix );
std::string generate_temp_filename( const std::string &prefix );

double radec_to_decimal( std::string str_in );                          ///< convert ra,dec from string to double
double radec_to_decimal( std::string str_in, std::string &retstring );  ///< convert ra,dec from string to double
void decimal_to_sexa( const double dec_in, std::string &retstring );    ///< convert decimal to sexagesimal
double angular_separation( double ra1, double dec1, double ra2, double dec2 );  ///< compute angular separation between points on sphere

static inline void rtrim(std::string &s) {          /// trim off trailing whitespace from a string
  s.erase( std::find_if( s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); } ).base(), s.end() );
}

inline bool caseCompareChar( char a, char b ) { return ( std::toupper(a) == std::toupper(b) ); }

inline bool caseCompareString( const std::string &s1, const std::string &s2 ) {
  return( (s1.size()==s2.size() ) && std::equal( s1.begin(), s1.end(), s2.begin(), caseCompareChar) ); }


inline void make_uppercase( std::string &in ) {
  try { std::transform( in.begin(), in.end(), in.begin(), ::toupper ); }
  catch( std::exception &e ) { std::cerr << "ERROR transforming string \"" << in << "\": " << e.what() << "\n"; }
}

inline std::string to_uppercase( std::string in ) {
  try { std::transform( in.begin(), in.end(), in.begin(), ::toupper ); return in; }
  catch( std::exception &e ) { std::cerr << "ERROR transforming string \"" << in << "\": " << e.what() << "\n"; }
}


/***** to_string_prec *******************************************************/
/**
 * @brief      convert a numeric value to a string with specified precision
 * @details    Since std::to_string doesn't allow changing the precision,
 *             I wrote my own equivalent. Probably don't want to use this
 *             in a tight loop.
 * @param[in]  value_in  numeric value in of type <T>
 * @param[in]  prec      desired precision, default=6
 * @return     string
 *
 */
template <typename T>
std::string to_string_prec( const T value_in, const int prec = 6 ) {
  std::ostringstream out;
  out.precision(prec);
  out << std::fixed << value_in;
  return std::move(out).str();
}
/***** to_string_prec *******************************************************/


/***** InterruptableSleepTimer***********************************************/
/**
 * @class   InterruptableSleepTimer
 * @brief   creates a sleep timer that can be interrupted.
 * @details This class uses try_lock_for in order to put a thread to sleep,
 *          while allowing it to be woken up early.
 *
 */
class InterruptableSleepTimer {
  private:
    std::timed_mutex _mut;
    std::atomic<bool> _locked;        // track whether the mutex is locked
    std::atomic<bool> _run;

    inline void _lock() { _mut.lock(); _locked = true; }       // lock mutex

    inline void _unlock() { _locked = false; _mut.unlock(); }  // unlock mutex

  public:
    // lock on creation
    //
    InterruptableSleepTimer() {
      _lock();
      _run = true;
    }

    // unlock on destruction, if wake was never called
    //
    ~InterruptableSleepTimer() {
      if ( _locked ) {
        _unlock();
        _run = false;
      }
    }

    inline bool running() { return _run; }

    // called by any thread except the creator
    // waits until wake is called or the specified time passes
    //
    template< class Rep, class Period >
    void sleepFor( const std::chrono::duration<Rep,Period> &timeout_duration ) {
      if ( _run && _mut.try_lock_for( timeout_duration ) ) {
        // if successfully locked, remove the lock
        //
        _mut.unlock();
      }
    }

    // unblock any waiting threads, handling a situation
    // where wake has already been called.
    // should only be called by the creating thread
    //
    inline void stop() {
      if ( _locked ) {
        _run = false;
        _unlock();
      }
    }
    inline void start() {
      if ( ! _locked ) {
        _lock();
        _run = true;
      }
    }
};
/***** InterruptableSleepTimer **********************************************/


/***** Time *****************************************************************/
/**
 * @class   Time
 * @brief   encapsulates the logic of getting current time into timespec struct
 * @details static function getTimeNow allows calling without creating an
 *          instance of the class.
 *
 */
class Time {
  public:
    static timespec getTimeNow() {
      struct timespec timenow;
      clock_gettime( CLOCK_REALTIME, &timenow );
      return timenow;
    }
};
/***** Time *****************************************************************/


/***** NumberPool ***********************************************************/
/**
 * @class   NumberPool
 * @brief   manages a pool of numbers
 * @details Provides function of getting the lowest available number out
 *          of a pool of numbers, and the ability to return a number back
 *          to the pool. The pool grows as needed. The pool is meant to
 *          contain positive integers and if the number would exceed the
 *          max value for int, then -1 is returned.
 *
 */
class NumberPool {
  private:
    std::mutex pool_lock;        /// protects pool access from multiple threads
    int next_number;             /// next number available
    std::set<int> used_numbers;  /// The pool is a set (for automatic sorting) which
                                 /// represents the numbers that are being used, so
                                 /// missing numbers are the available numbers.
  public:

    /**
     * Constructed with the starting number of the pool
     */
    NumberPool( int starting_number ) : next_number(starting_number) { }


    /***** NumberPool::get_next_number ****************************************/
    /**
     * @brief      gets the lowest available number from the pool
     * @details    This returns the lowest missing value in the set and works
     *             because a std::set is auto-sorted.
     * @return     positive int or -1 if pool exceeds max allowed by an int
     *
     */
    int get_next_number() {
      std::lock_guard<std::mutex> lock( pool_lock );
      int number = next_number;
      used_numbers.insert( number );
      if ( next_number < std::numeric_limits<int>::max() ) ++next_number;
      else return -1;
      return number;
    }
    /***** NumberPool::get_next_number ****************************************/


    /***** NumberPool::release_number *****************************************/
    /**
     * @brief      returns indicated number back to the pool
     * @details    numbers are "returned" by removing them from the set of used numbers
     * @param[in]  number  number to put back into pool
     *
     */
    void release_number( int number ) {
      std::lock_guard<std::mutex> lock( pool_lock );
      used_numbers.erase( number );
      if ( number < next_number ) next_number = number;
      return;
    }
    /***** NumberPool::release_number *****************************************/
};
/***** NumberPool *************************************************************/


/***** BoolState **************************************************************/
/**
 * @class   BoolState
 * @brief   automatically set/clear a boolean
 * @details This is a utility class to automatically set and clear a boolean
 *          for the duration of a limited scope. You supply the atomic bool
 *          variable and construct this class with a reference to that variable
 *          and the bool state is set (true). When the class goes out of scope
 *          the destructor will clear the bool state (false).
 *
 */
class BoolState {
  private:
    std::atomic<bool> &_state;
  public:
    BoolState( std::atomic<bool> &state_var ) : _state( state_var ) { _state.store( true, std::memory_order_seq_cst ); }
    ~BoolState() { _state.store( false, std::memory_order_seq_cst ); }
};
/***** BoolState **************************************************************/


/***** PreciseTimer ***********************************************************/
/**
 * @class   PreciseTimer
 * @brief   creates an interruptable, precise sleep timer object
 * @details This is a utility class to create a reasonably accurate
 *          interruptable sleep timer (precision approx 100 microseconds).
 *          This is done by making repeated calls to precise_sleep(),
 *          while checking for a cancel flag.
 *
 */
class PreciseTimer {
  private:
    std::atomic<bool> cancelled;

    /***** PreciseTimer::precise_sleep ****************************************/
    void precise_sleep(long microseconds) {
      struct timespec start_time;
      struct timespec current_time;
      struct timespec ts;

      // remaining time starts as requested time
      //
      long remaining_time = microseconds;

      // Loop until the total elapsed time reaches the requested sleep time
      //
      clock_gettime(CLOCK_MONOTONIC, &start_time);  // start time
      while ( remaining_time > 0 ) {
        // Calculate remaining time
        //
        ts.tv_sec = remaining_time / 1000000;
        ts.tv_nsec = (remaining_time % 1000000) * 1000;

        // Sleep for the remaining time
        //
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);

        // check current time and calculate elapsed and remaining time
        //
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000 +
                            (current_time.tv_nsec - start_time.tv_nsec) / 1000;
        remaining_time = microseconds - elapsed_time;
      }
    }

    /***** PreciseTimer::long_precise_sleep ***********************************/
    void long_precise_sleep(long microseconds) {
      // max_short_sleep is the largest sleep time for precise_sleep()
      // This should be kept small, a few seconds or less.
      //
      const long max_short_sleep = 3000000;         // units are microseconds
      struct timespec start_time;
      struct timespec current_time;

      // remaining time starts as requested time
      //
      long remaining_time = microseconds;

      // loop forever until broken
      //
      clock_gettime(CLOCK_MONOTONIC, &start_time);  // start time
      while ( true ) {
        if (cancelled) return;                      // break if cancelled

        // sleep for the shorter of max_short_sleep or remaining time
        //
        long to_sleep = std::min( remaining_time, max_short_sleep );
        precise_sleep(to_sleep);

        // calculate elapsed time and time remaining
        //
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000 +
                            (current_time.tv_nsec - start_time.tv_nsec) / 1000;
        remaining_time = microseconds - elapsed_time;

        if ( remaining_time <= 0 ) break;           // break when time is up
      }
    }

  public:
    PreciseTimer() : cancelled(false) { }

    /***** PreciseTimer::delay ************************************************/
    void delay(long milliseconds) {
      cancelled=false;
      long microseconds = milliseconds * 1000;
      long_precise_sleep( microseconds );
    }

    /***** PreciseTimer::stop *************************************************/
    void stop() { cancelled=true; }
};
/***** PreciseTimer ***********************************************************/

