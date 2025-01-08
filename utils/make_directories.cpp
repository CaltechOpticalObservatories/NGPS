/**
 * @file    make_directories.cpp
 * @brief   creates /data/YYYYMMDD subdirectory, meant to be run with cron
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include <filesystem>
#include <chrono>
#include <ctime>
#include <string>
#include <iostream>
#include <iomanip>

namespace fs = std::filesystem;

std::string get_date() {
  std::stringstream current_date;    // String to contain the return value
  std::time_t t=std::time(nullptr);  // Container for system time
  struct timespec timenow;;          // Time of day container
  struct tm mytime;                  // time container

  // Get the system time, return a bad datestamp on error
  if ( clock_gettime( CLOCK_REALTIME, &timenow ) != 0 ) return( "" );

  // Convert the time of day to local or GMT
  t = timenow.tv_sec;
  if ( localtime_r( &t, &mytime ) == nullptr ) return( "" );

  current_date.setf(std::ios_base::right);
  current_date << std::setfill('0') << std::setprecision(0)
               << std::setw(4) << mytime.tm_year + 1900
               << std::setw(2) << mytime.tm_mon  + 1
               << std::setw(2) << mytime.tm_mday + 1;  // tomorrow!

  return( current_date.str() );
}

int main() {

  // base directory
  const std::string base("/data");

  // get local time as YYYYMMDD
  std::string date = get_date();
  if ( date.empty() ) {
    std::cerr << "error getting date" << std::endl;
    return 1;
  }

  // new directory is <base>/<date>
  fs::path newdir  = fs::path(base) / date;
  fs::path acamdir = newdir / "acam";
  fs::path scamdir = newdir / "slicecam";
  fs::path logdir  = newdir / "logs";

  try {
    // if the directory doesn't exist then create it
    // and set the permissions
    if ( !fs::exists(newdir) ) {
      fs::create_directory(newdir);
      fs::permissions( newdir,
                       fs::perms::owner_all   |
                       fs::perms::group_read  |
                       fs::perms::group_exec  |
                       fs::perms::others_read |
                       fs::perms::others_exec
                     );
      std::cout << "created directory " << newdir << std::endl;
    }
    else std::cout << "directory " << newdir << " already exists" << std::endl;

    if ( !fs::exists(acamdir) ) {
      fs::create_directory(acamdir);
      std::cout << "created directory " << acamdir << std::endl;
    }
    if ( !fs::exists(scamdir) ) {
      fs::create_directory(scamdir);
      std::cout << "created directory " << scamdir << std::endl;
    }
    if ( !fs::exists(logdir) ) {
      fs::create_directory(logdir);
      std::cout << "created directory " << logdir << std::endl;
    }
  }
  catch ( const fs::filesystem_error &e ) {
    std::cerr << "ERROR creating " << newdir << ": " << e.what();
  }

  return 0;
}
