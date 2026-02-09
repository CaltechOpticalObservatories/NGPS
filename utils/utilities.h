/**
 * @file    utilities.h
 * @brief   some handy utilities to use anywhere
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <functional>
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
#include <dirent.h>
#include <map>
#include "json.hpp"
#include <condition_variable>
#include <initializer_list>
#include <bitset>
#include <cstdlib>
#include <arpa/inet.h>
#include <netdb.h>

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

std::chrono::system_clock::time_point next_occurrence( int hour, int minute, int second );

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

bool validate_path( const std::string path );  /// validate a directory path, creating subdirs as needed
std::string get_system_date();                      /// return current date in formatted string "YYYYMMDD"
std::string get_system_date( const std::string &tmzone_in );

std::string get_file_time();                        /// return current time in formatted string "YYYYMMDDHHMMSS" used for filenames
std::string get_file_time( const std::string &tmzone_in );

double get_clock_time();

double get_time_as_double();
std::string datetime_from_double( double &time );

std::string get_latest_datedir( const std::string &basedir, bool shortform=false );

void precise_sleep( long microseconds );             /// precise, non-interruptable sleep timer for short durrations

long timeout( int wholesec=0, std::string next="" );  /// wait until next integral second or minute

double mjd_from( struct timespec &time_n );         /// modified Julian date from input timespec struct

std::string get_localhost();

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
  return in;
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
    BoolState( std::atomic<bool> &state_var ) : _state( state_var ) { _state.store( true, std::memory_order_release ); }
    ~BoolState() { _state.store( false, std::memory_order_release ); }
};
/***** BoolState **************************************************************/


/***** PreciseTimer ***********************************************************/
/**
 * @class   PreciseTimer
 * @brief   creates an interruptable, precise sleep timer object
 * @details This is a utility class to create a reasonably accurate
 *          interruptable sleep timer (precision approx 100 microseconds).
 *          This is done by making repeated calls to precise_sleep(),
 *          while checking for a cancel flag. The timer can also be
 *          modified.
 *
 *          Units are in microseconds except for the inputs, delay(ms) and
 *          modify(ms), and outputs, get_remaining() and stop(&ms), which
 *          are in milliseconds.
 */
class PreciseTimer {
  private:
    static inline constexpr long max_short_sleep = 3000000;         // units are microseconds

    std::atomic<bool> should_hold;
    std::atomic<bool> on_hold;
    std::atomic<bool> should_stop;
    std::atomic<bool> running;
    std::atomic<long> delay_time;
    std::atomic<long> remaining_time;
    mutable std::mutex mtx;
    std::condition_variable cv;

    /***** PreciseTimer::precise_sleep ****************************************/
    /** @brief  This is the inner loop which sleeps for a short time,         */
    /**         specified in microseconds. It cannot be interrupted.          */
    void precise_sleep(long microseconds) {
      struct timespec start_time;
      struct timespec current_time;
      struct timespec ts;

      // precise-loop remaining time starts as requested time
      //
      long _remaining_time = microseconds;

      // Loop until the total elapsed time reaches the requested sleep time
      //
      clock_gettime(CLOCK_MONOTONIC, &start_time);  // start time
      while ( _remaining_time > 0 ) {
        // Calculate remaining time
        //
        ts.tv_sec = _remaining_time / 1000000;
        ts.tv_nsec = (_remaining_time % 1000000) * 1000;

        // Sleep for the remaining time
        //
#ifdef __APPLE__
        nanosleep(&ts, NULL);
#else
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
#endif

        // check current time and calculate elapsed and remaining time
        //
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000 +
                            (current_time.tv_nsec - start_time.tv_nsec) / 1000;
        _remaining_time = microseconds - elapsed_time;
      }
    }

    /***** PreciseTimer::long_precise_sleep ***********************************/
    /** @brief  This is the main delay loop which can be interrupted.         */
    void long_precise_sleep() {
      // max_short_sleep is the largest sleep time for precise_sleep()
      // This should be kept small, a few seconds or less.
      //
      struct timespec start_time;
      struct timespec current_time;
      struct timespec hold_start, hold_stop;
      long hold_time=0;

      running.store(true, std::memory_order_release);

      // total remaining time starts as requested time
      //
      remaining_time.store( delay_time.load(std::memory_order_acquire), std::memory_order_release );

      // loop forever until broken
      //
      clock_gettime(CLOCK_MONOTONIC, &start_time);  // start time
      while ( true ) {
        // sleep for the shorter of max_short_sleep or remaining time
        //
        long to_sleep = std::min( remaining_time.load(std::memory_order_acquire), max_short_sleep );
        precise_sleep(to_sleep);

        // Calculate elapsed time and time remaining.
        // Elapsed time is difference between time now and start time,
        // less any time that the timer was on hold.
        //
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000 +
                            (current_time.tv_nsec - start_time.tv_nsec) / 1000  -
                            hold_time;

        remaining_time.store( (delay_time.load(std::memory_order_acquire) - elapsed_time),
                              std::memory_order_release );

        if ( remaining_time.load(std::memory_order_acquire) <= 0 ) break;    // break when time is up

        if (should_stop.load(std::memory_order_acquire)) break;              // break if stop requested

        // When hold is requested, store the hold start time, send
        // notification that loop is on hold, and wait for request
        // to either release hold or stop.
        //
        if (should_hold.load(std::memory_order_acquire)) {
          {
          std::unique_lock<std::mutex> lock(mtx);
          on_hold.store(true, std::memory_order_release);
          cv.notify_all();
          }
          clock_gettime(CLOCK_MONOTONIC, &hold_start);
          {
          std::unique_lock<std::mutex> lock(mtx);
          cv.wait( lock, [this]() { return ( should_stop.load(std::memory_order_acquire) ||
                                            !should_hold.load(std::memory_order_acquire) ); } );
          }

          if (should_stop.load(std::memory_order_acquire)) break;            // break if stop requested

          // how long was loop on hold, in microseconds
          clock_gettime(CLOCK_MONOTONIC, &hold_stop);
          hold_time = (hold_stop.tv_sec-hold_start.tv_sec)*1000000 +
                      (hold_stop.tv_nsec-hold_stop.tv_nsec)/1000;
          on_hold.store(false, std::memory_order_release);
        }
      }
      running.store(false, std::memory_order_release);
      cv.notify_all();
    }

  public:
    PreciseTimer() { reset(); }

    /***** PreciseTimer::delay ************************************************/
    /** @brief  This is the entry point to create a blocking delay, which     */
    /**         won't return until milliseconds have elapsed unless stopped   */
    /**         or modified.                                                  */
    long delay(long milliseconds) {
      if (running.load(std::memory_order_acquire)) {
        std::cerr << get_timestamp() << "  (PreciseTimer::delay) cannot start new timer while another is running\n";
        return 1;
      }
      reset();
      delay_time.store(1000*milliseconds);
      long_precise_sleep();
      return 0;
    }

    /***** PreciseTimer::get_remaining ****************************************/
    /** @brief  Returns the time remaining in milliseconds                    */
    long get_remaining() { return remaining_time.load(std::memory_order_acquire)/1000; }

    /***** PreciseTimer::modify ***********************************************/
    /** @brief  Modifies the delay time to new value in milliseconds          */
    void modify(long milliseconds) { delay_time.store(milliseconds*1000, std::memory_order_release); }

    /***** PreciseTimer::progress *********************************************/
    /** @brief  Returns by reference remaining and delay times in msec        */
    void progress(long &remaintime, long &delaytime) {
      remaintime = ( !running.load(std::memory_order_acquire) ? 0 :
                     remaining_time.load(std::memory_order_acquire)/1000 );
      delaytime  = delay_time.load(std::memory_order_acquire)/1000;
    }

    /***** PreciseTimer::hold *************************************************/
    /** @brief  Hold/pause the delay timer at the next short-sleep boundary   */
    void hold() {
      std::unique_lock<std::mutex> lock(mtx);
      // request hold
      should_hold.store(true, std::memory_order_release);
      // wait until the delay loop says it is on hold
      if (on_hold.load(std::memory_order_acquire)) return;
      cv.wait( lock, [this]() { return on_hold.load(std::memory_order_acquire); } );
    }

    /***** PreciseTimer::resume ***********************************************/
    /** @brief  Removes the hold and resumes the delay after a hold           */
    void resume() {
      std::unique_lock<std::mutex> lock(mtx);
      should_hold.store(false, std::memory_order_release);
      cv.notify_all();
    }

    /***** PreciseTimer::stop *************************************************/
    /** @brief  Stops the delay at the next short-sleep boundary              */
    void stop() {
     long ms;
     stop(ms);
    }

    /***** PreciseTimer::stop *************************************************/
    /** @brief  Stops the delay at the next short-sleep boundary, returns     */
    /**         remaining time in milliseconds by reference.                  */
    void stop(long &milliseconds) {
      {
      std::unique_lock<std::mutex> lock(mtx);
      should_stop.store(true, std::memory_order_release);  // tells the main loop to stop
      cv.notify_all();
      }
      if ( running.load(std::memory_order_acquire) ) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!cv.wait_for(lock, std::chrono::microseconds(2*max_short_sleep),
            [this]() { return !running.load(std::memory_order_acquire); })) {
          std::cerr << get_timestamp() << "  (PreciseTimer::stop) *** timed out waiting for loop to stop ***\n";
        }
      }
      milliseconds=remaining_time.load(std::memory_order_acquire)/1000;
      reset();
    }

    /***** PreciseTimer::reset ************************************************/
    /** @brief  For internal use only, resets class variables                 */
    void reset() {
      should_hold.store(false);
      on_hold.store(false);
      should_stop.store(false);
      running.store(false);
      delay_time.store(0);
      remaining_time.store(0);
    }
};
/***** PreciseTimer ***********************************************************/

template <size_t N>
class ImprovedStateManager {
  using StateCallback = std::function<void(const std::bitset<N>&)>;
  private:
    std::bitset<N> state_bits;
    const std::map<size_t, std::string> state_names;
    StateCallback callback_function;
    mutable std::mutex cb_mtx;
    mutable std::mutex mtx;
    void notify() const {
      std::lock_guard<std::mutex> lock(cb_mtx);
      if (callback_function) callback_function(state_bits);
    }

  public:
    explicit ImprovedStateManager( const std::map<size_t, std::string> &names_in )
      : state_bits(), state_names(names_in) { }

    void set_callback(StateCallback cb) {
      std::lock_guard<std::mutex> lock(cb_mtx);
      callback_function = std::move(cb);
    }
    void set(size_t bit ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits.set( bit );
      }
      notify();
    }
    void clear(size_t bit) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits.reset( bit );
      }
      notify();
    }
    void clear_all() {
      {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits.reset();
      }
      notify();
    }
    void set_only( std::initializer_list<size_t> setbits ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits.reset();
      for ( auto set : setbits ) state_bits.set(set);
      }
      notify();
    }
    void set_and_clear( std::initializer_list<size_t> setbits, std::initializer_list<size_t> clrbits ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      for ( auto set : setbits ) state_bits.set(set);
      for ( auto clr : clrbits ) state_bits.reset(clr);
      }
      notify();
    }
    bool is_set(size_t bit) {
      std::lock_guard<std::mutex> lock(mtx);
      return state_bits.test(bit);
    }
    template <typename... B>
    bool are_set( B... bits ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return ( state_bits.test(bits) && ... );
    }
    template <typename... B>
    bool are_any_set( B... bits ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return ( state_bits.test(bits) || ... );
    }
    bool are_any_set() const {
      std::lock_guard<std::mutex> lock(mtx);
      return state_bits.any();
    }
    bool are_all_set() const {
      std::lock_guard<std::mutex> lock(mtx);
      for ( size_t i=0; i < state_bits.size(); ++i ) {
        if (!state_bits.test(i)) {
          return false;
        }
      }
      return true;
    }
    bool are_all_clear() const {
      std::lock_guard<std::mutex> lock(mtx);
      for ( size_t i=0; i < state_bits.size(); ++i ) {
        if (state_bits.test(i)) {
          return false;
        }
      }
      return true;
    }
    std::string get_set_states() const {
      std::string result;
      std::lock_guard<std::mutex> lock(mtx);
      for ( size_t i=0; i < state_bits.size(); ++i ) {
        if (state_bits.test(i)) {
          result += state_names.at(i) + " ";
        }
      }
      return result;
    }
    std::string get_cleared_states() const {
      std::string result;
      std::lock_guard<std::mutex> lock(mtx);
      for ( size_t i=0; i < state_bits.size(); ++i ) {
        if (!state_bits.test(i)) {
          result += state_names.at(i) + " ";
        }
      }
      return result;
    }
};


/***** ScopedState ************************************************************/
/**
 * @class   ScopedState
 * @brief   object sets a state on construction, clears on destruction
 * @details This class is used to create a scoped state object. Constructed
 *          with a state manager object and a state, the state is set on
 *          construction and when this object goes out of scope the state is
 *          cleared.
 *
 */
template <size_t N>
class ScopedState {
  private:
    size_t _statebit;                   // set this state on construction
    size_t _destructbit;                // optionally set this state on destruction
    bool _set_on_destruct;              // set destruct bit if this is true
    ImprovedStateManager<N> &_manager;  // reference to ImprovedStateManager to use

  public:
    ScopedState( ImprovedStateManager<N> &manager, size_t statebit )
      : _statebit(statebit),
        _set_on_destruct(false),
        _manager(manager) {
        _manager.set(_statebit);
      }
    ScopedState( ImprovedStateManager<N> &manager, size_t statebit, bool setonly )
      : _statebit(statebit),
        _set_on_destruct(false),
        _manager(manager) {
        if (setonly) _manager.set_only({_statebit});
        else _manager.set(_statebit);
      }
    // When this object goes out of scope, clear the _statebit.
    // If _set_on_destruct was set then set _destructbit and clear _statebit.
    ~ScopedState() {
      if (_set_on_destruct) _manager.set_and_clear({_destructbit}, {_statebit});
      else _manager.clear(_statebit);
    }
    void destruct_set( size_t destructbit ) {
      _destructbit=destructbit;
      _set_on_destruct=true;
    }
};
/***** ScopedState ************************************************************/


/***** StateManager ***********************************************************/
/**
 * @class   StateManager
 * @brief   Creates a state manager using a thread safe bitset object
 * @details std::bitset does not provide atomic operations. This class achieves
 *          atomicity by protecting a bitset object with a mutex. Set, clear,
 *          and is_set functions are also provided.
 *
 *          The size N of the bitset object should be known at compile time.
 *
 *          All get and set and clear operations are done in a thread-safe
 *          manner using a mutex to protect operations.
 *          All set and clear operations notify any waiting threads.
 *
 *          The wait functions set a bitset indicating which bits are being
 *          waited on, and can be cancelled. When cancelled an exception
 *          is thrown.
 *
 */
template <size_t N>
class StateManager {
  private:
    std::bitset<N> state_bits;
    std::bitset<N> pending_bits;  // contains the bits being waited on
    std::map<size_t, std::string> state_names;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> should_cancel{false};

  public:
    StateManager( const std::map<size_t, std::string> &names_in )
      : state_names(names_in) { initialize(); }

    StateManager() { initialize(); }

    void initialize( bool set_all=false ) {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits = set_all ? std::bitset<N>().set() : std::bitset<N>();
    }

    std::bitset<N> get() const {
      std::lock_guard<std::mutex> lock(mtx);
      return state_bits;
    }

    std::bitset<N> get_pending() const {
      std::lock_guard<std::mutex> lock(mtx);
      return pending_bits;
    }

    /**
     * @brief      cancels all waiting threads and resets all pending bits
     */
    void cancel_wait() {
      {
      std::lock_guard<std::mutex> lock(mtx);
      should_cancel=true;
      pending_bits.reset();
      }
      cv.notify_all();
    }

    /**
     * @brief      set multiple state bits in variadic list
     * @param[in]  states  variadic list of state bits to set
     */
    template <typename... StateBits>
    void set( StateBits... states ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      ( state_bits.set( states ), ... );
      }
      cv.notify_all();
    }

    /**
     * @brief      clear multiple state bits in variadic list
     * @param[in]  states  variadic list of state bits to clear
     */
    template <typename... StateBits>
    void clear( StateBits... states ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      ( state_bits.reset( states ), ... );
      }
      cv.notify_all();
    }

    /**
     * @brief      atomically set and clear the specified state bits
     * @param[in]  setstate  state bit to set
     * @param[in]  clrstate  state bit to clear
     */
    void set_and_clear( size_t setstate, size_t clrstate ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits.set( setstate );
      state_bits.reset( clrstate );
      }
      cv.notify_all();
    }

    /**
     * @brief      atomically set and clear the specified state bits
     * @param[in]  setstates  list of state bits to set
     * @param[in]  clrstates  list of state bits to clear
     */
    void set_and_clear( std::initializer_list<size_t> setstates, std::initializer_list<size_t> clrstates ) {
      {
      std::lock_guard<std::mutex> lock(mtx);
      for ( auto set : setstates ) state_bits.set(set);
      for ( auto clr : clrstates ) state_bits.reset(clr);
      }
      cv.notify_all();
    }

    /**
     * @brief      clear all state bits
     */
    void clear_all() {
      {
      std::lock_guard<std::mutex> lock(mtx);
      state_bits.reset();
      }
      cv.notify_all();
    }

    /**
     * @brief      is the specified state bit clear?
     * @param[in]  state  state bit to check
     * @return     true|false
     */
    bool is_clear( size_t state ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return !state_bits.test( state );
    }

    /**
     * @brief      are all of the specified state bits clear?
     * @param[in]  states  variadic list of state bits to check
     * @return     true|false
     */
    template <typename... StateBits>
    bool are_all_clear( StateBits... states ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return ( (!state_bits.test(states)) && ... );
    }

    /**
     * @brief      is the specified state bit set?
     * @param[in]  state  state bit to check
     * @return     true|false
     */
    bool is_set( size_t state ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return state_bits.test( state );
    }

    /**
     * @brief      are all of the specified state bits set?
     * @param[in]  states  variadic list of state bits to check
     * @return     true|false
     */
    template <typename... StateBits>
    bool are_all_set( StateBits... states ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return ( state_bits.test(states) && ... );
    }

    /**
     * @brief      is any one of the specified state bits set?
     * @param[in]  states  variadic list of state bits to check
     * @return     true|false
     */
    template <typename... StateBits>
    bool is_any_set( StateBits... states ) const {
      std::lock_guard<std::mutex> lock(mtx);
      return ( state_bits.test(states) || ... );
    }

    /**
     * @brief      is any state bit in the bitset set?
     * @return     true|false
     */
    bool is_any_set() const {
      std::lock_guard<std::mutex> lock(mtx);
      return state_bits.any();
    }

    /**
     * @brief      wait for specified state to be set
     * @details    this can throw an exception
     * @param[in]  state  state bit to wait for
     */
    void wait_for_state_set( size_t state ) {
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_state_set cancelled by external signal");
      }
      if ( state_bits.test( state ) ) return;
      pending_bits.set(state);
      {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait( lock, [this,state]() { return should_cancel.load(std::memory_order_acquire) || state_bits.test( state ); } );
      }
      pending_bits.reset(state);
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_state_set cancelled by external signal");
      }
    }

    /**
     * @brief      wait for specified state to be clear
     * @details    this can throw an exception
     * @param[in]  state  state bit to wait for
     */
    void wait_for_state_clear( size_t state ) {
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_state_clear cancelled by external signal");
      }
      if ( !state_bits.test( state ) ) return;
      pending_bits.set(state);
      {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait( lock, [this,state]() { return ( should_cancel.load(std::memory_order_acquire) || !state_bits.test( state ) ); } );
      }
      pending_bits.reset(state);
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_state_clear cancelled by external signal");
      }
    }

    /**
     * @brief      wait for specified states to be clear
     * @details    this can throw an exception
     * @param[in]  states  state bits to wait for
     */
    template <typename... StateBits>
    void wait_for_states_clear(StateBits... states) {
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_states_clear cancelled by external signal");
      }
      (pending_bits.set(states), ...);
      if ( ( ... && !state_bits.test(states) ) ) {
        (pending_bits.reset(states), ...);
        return;
      }
      {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this, &states...]() { return should_cancel.load(std::memory_order_acquire) || (... && !state_bits.test(states)); });
      }
      (pending_bits.reset(states), ...);
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_states_clear cancelled by external signal");
      }
    }

    /**
     * @brief      wait for all state bits to match those of another StateManager object
     * @details    this can throw an exception
     * @param[in]  obj  reference to another StateManager object for comparison
     */
    void wait_for_match( const StateManager &obj ) {
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_match cancelled by external signal");
      }
      if ( state_bits == obj.get() ) return;
      pending_bits = obj.get();    // these are the bits being waited for
      {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait( lock, [this,&obj] { return ( should_cancel.load(std::memory_order_acquire) || state_bits == obj.get() ); } );
      }
      pending_bits &= ~obj.get();  // remove them from pending
      if ( should_cancel.load(std::memory_order_acquire) ) {
        should_cancel.store(false, std::memory_order_release);
        throw std::runtime_error("wait_for_match cancelled by external signal");
      }
    }

    /**
     * @brief      returns the name of a single state bit
     * @param[in]  state  state bit enum in a std::bitset
     * @return     string
     */
    std::string get_state_name( size_t state ) const {
      std::lock_guard<std::mutex> lock(mtx);
      auto it = state_names.find( state );
      return it != state_names.end() ? it->second : "unknown";
    }

    /**
     * @brief      returns a space-delimited string of names of all set states
     * @return     space-delimited string
     */
    std::string get_set_names() {
      std::stringstream names;
      for ( const auto &[state,name] : state_names ) {
        if ( state_bits.test(state) ) {
          if ( !names.str().empty() ) names << " ";
          names << name;
        }
      }
      return names.str();
    }

    /**
     * @brief      returns a space-delimited string of names of all clear states
     * @return     space-delimited string
     */
    std::string get_clear_names() {
      std::stringstream names;
      for ( const auto &[state,name] : state_names ) {
        if ( !state_bits.test(state) ) {
          if ( !names.str().empty() ) names << " ";
          names << name;
        }
      }
      return names.str();
    }

    /**
     * @brief      returns a space-delimited string of names of all pending states
     * @return     space-delimited string
     */
    std::string get_pending_names() {
      std::stringstream names;
      // loop through all bits
      for ( size_t bit=0; bit<N; ++bit ) {
        // if pending_bits has this bit set
        // then find the corresponding name from state_names
        if ( pending_bits.test(bit) ) {
          auto it = state_names.find(bit);
          if ( it != state_names.end() ) names << it->second << " ";
        }
      }
      return names.str();
    }
};
/***** StateManager ***********************************************************/


/***** GuardedCounter *******************************************************/
/**
 * @class   GuardedCounter
 * @brief   automatically increments on construction, decrements on destruction
 * @details construct with a std::atomic<int> which is passed by reference
 *
 */
class GuardedCounter {
  public:
    std::atomic<int> &counter;

    // increments on construction
    GuardedCounter(std::atomic<int> &c) : counter(c) { ++counter; }

    // decrements on destruction
    ~GuardedCounter() { --counter; }

    // non-copyable, non-movable
    GuardedCounter(const GuardedCounter &) = delete;
    GuardedCounter & operator=(const GuardedCounter &) = delete;
};
/***** GuardedCounter *******************************************************/
