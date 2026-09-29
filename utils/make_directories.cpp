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
#include <sstream>
#include "utilities.h"

namespace fs = std::filesystem;

std::string get_date() {
  std::stringstream current_date;    // String to contain the return value
  std::time_t t=std::time(nullptr);  // Container for system time
  struct tm mytime;                  // time container

  // local now
  if ( localtime_r( &t, &mytime ) == nullptr ) return ( "" );

  // back 12h so the date changes at local noon, then ahead one day for
  // the UTC date of that night; never ahead of the night in progress
  mytime.tm_hour -= 12;
  mytime.tm_mday += 1;
  mytime.tm_isdst = -1; // let mktime determine DST

  // normalize struct handles rollovers
  if ( mktime( &mytime ) == -1 ) return "";

  current_date << std::setfill('0') << std::setprecision(0)
               << std::setw(4) << mytime.tm_year + 1900
               << std::setw(2) << mytime.tm_mon  + 1
               << std::setw(2) << mytime.tm_mday;

  return current_date.str();
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
      fs::permissions( newdir, fs::perms::owner_all | fs::perms::group_all);
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

    // point <base>/latest at the latest date directory, which is where the
    // daemons write, replacing it atomically so readers never see it missing
    std::string latest = get_latest_datedir( base );
    if ( !latest.empty() ) {
      fs::path target = fs::path( base ) / latest;
      fs::path linkdir = fs::path( base ) / "latest";
      fs::path tmplink = fs::path( base ) / ".latest.tmp";
      if ( !fs::is_symlink( linkdir ) || fs::read_symlink( linkdir ) != target ) {
        fs::remove( tmplink ); // leftover from a failed run
        fs::create_directory_symlink( target, tmplink );
        fs::rename( tmplink, linkdir );
        std::cout << "linked " << linkdir << " -> " << target << std::endl;
      }
    }
  }
  catch ( const fs::filesystem_error &e ) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
