/**
 * @file    utilities.h
 * @brief   some handy utilities to use anywhere
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef UTILITIES_H
#define UTILITIES_H

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

extern std::string zone;

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

std::string get_timestamp();                        /// return current time in formatted string "YYYY-MM-DDTHH:MM:SS.ssssss"
std::string get_system_date();                      /// return current date in formatted string "YYYYMMDD"
std::string get_file_time();                        /// return current time in formatted string "YYYYMMDDHHMMSS" used for filenames

double get_clock_time();

long timeout( int wholesec=0, std::string next="" );

int compare_versions(std::string v1, std::string v2);

static inline void rtrim(std::string &s) {          /// trim off trailing whitespace from a string
  s.erase( std::find_if( s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); } ).base(), s.end() );
}

inline bool caseCompareChar( char a, char b ) { return ( std::toupper(a) == std::toupper(b) ); }

inline bool caseCompareString( const std::string &s1, const std::string &s2 ) {
  return( (s1.size()==s2.size() ) && std::equal( s1.begin(), s1.end(), s2.begin(), caseCompareChar) ); }

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

#endif
