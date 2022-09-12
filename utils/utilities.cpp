/**
 * @file    utilities.cpp
 * @brief   some handy utilities to use anywhere
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "utilities.h"

std::string zone="";

  /** cmdOptionExists *********************************************************/
  /**
   * @fn         cmdOptionExists
   * @brief      returns true if option is found in argv
   * @param[in]  begin char**
   * @param[in]  end char**
   * @param[out] option ref to string
   * @return     true or false
   *
   * Intended to be called as cmdOptionExists( argv, argv+argc, "-X" )
   * to search for "-X" option in argv.
   *
   * Pair with getCmdOption() function.
   *
   */
  bool cmdOptionExists( char** begin, char** end, const std::string &option ) {
    return std::find( begin, end, option ) != end;
  }
  /** cmdOptionExists *********************************************************/


  /** getCmdOption ************************************************************/
  /**
   * @fn         getCmdOption
   * @brief      returns pointer to command line option specified with "-X option"
   * @param[in]  begin char**
   * @param[in]  end char**
   * @param[out] option ref to string
   * @return     char*
   *
   * Intended to be called as char* option = getCmdOption( argv, argv+argc, "-X" );
   * to get option associated with "-X option" in argv.
   *
   * Pair with cmdOptionExists()
   *
   */
  char* getCmdOption( char** begin, char** end, const std::string &option ) {
    char** itr = std::find(begin, end, option);
    if ( itr != end && ++itr != end ) {
      return *itr;
    }
    return 0;
  }
  /** getCmdOption ************************************************************/


  /** my_hardware_concurrency *************************************************/
  /**
   * @fn         my_hardware_concurrency
   * @brief      return number of concurrent threads supported by the implementation
   * @param[in]  none
   * @return     int
   *
   * Counts the number of processors listed in /proc/cpuinfo
   *
   */
  int my_hardware_concurrency() {
    std::ifstream cpuinfo( "/proc/cpuinfo" );
    return std::count( std::istream_iterator<std::string>(cpuinfo),
                       std::istream_iterator<std::string>(),
                       std::string("processor") );
  }
  /** my_hardware_concurrency *************************************************/


  /** cores_available *********************************************************/
  /**
   * @fn         cores_available
   * @brief      return number of concurrent threads supported by the implementation
   * @param[in]  none
   * @return     int
   *
   * If the value is not known then check /proc/cpuinfo
   *
   */
  int cores_available() {
    unsigned int cores = std::thread::hardware_concurrency();
    return cores ? cores : my_hardware_concurrency();
  }
  /** cores_available *********************************************************/


  /** parse_val ***************************************************************/
  /**
   * @fn         parse_val
   * @brief      returns an unsigned int from a string
   * @param[in]  string
   * @return     unsigned int
   *
   */
  unsigned int parse_val(const std::string& str) {
    std::stringstream v;
    unsigned int u=0;
    if ( (str.find("0x")!=std::string::npos) || (str.find("0X")!=std::string::npos))
      v << std::hex << str;
    else
      v << std::dec << str;
    v >> u;
    return u;
  }
  /** parse_val ***************************************************************/


  /** Tokenize ****************************************************************/
  /**
   * @fn         Tokenize
   * @brief      break a string into a vector
   * @param[in]  str, input string
   * @param[out] tokens, vector of tokens
   * @param[in]  delimiters, string
   * @return     number of tokens
   *
   */
  int Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters) {
    // Clear the tokens vector, we only want to putput new tokens
    tokens.clear();

    // If the string is zero length, return now with no tokens
    if (str.length() == 0) { return(0); }

    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    std::string quote("\"");
    unsigned int quote_start = str.find(quote); //finds first quote mark
    bool quotes_found = false;

    if (quote_start != std::string::npos) {
    }
    else {
      quote_start = -1;
    }

    while (std::string::npos != pos || std::string::npos != lastPos) {
      if (quotes_found == true) {
        tokens.push_back(str.substr(lastPos + 1, pos - lastPos - 2));
        pos++;
        lastPos = str.find_first_not_of(delimiters, pos);
        quotes_found = false;
      }
      else {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
      }

      // If the next character is a quote, grab between the quotes 
      if (std::string::npos != lastPos && lastPos == quote_start){
        pos = str.find_first_of("\"", lastPos + 1) + 1;
        quotes_found = true;
      }
      // Otherwise, find next "non-delimiter"
      else {
        pos = str.find_first_of(delimiters, lastPos);
      }
    }
    return(tokens.size());
  }
  /** Tokenize ****************************************************************/


  /** Tokenize ****************************************************************/
  /**
   * @fn         Tokenize
   * @brief      break a string into device list and arg list vectors
   * @param[in]  &str, reference to input string
   * @param[in]  &devlist, reference to vector for device list
   * @param[in]  &ndev, reference to number of devices
   * @param[out] &arglist, reference to vector for arg list
   * @param[in]  &narg, reference to number of args
   * @return     nothing
   *
   * This is a special version of Tokenize.
   *
   * The expected format is a comma-delimited device list, followed by a colon,
   * followed by a space-delimited argument list.
   *
   * On error, ndev is set to -1
   *
   */
  void Tokenize( const std::string &str, std::vector<uint32_t> &devlist, int &ndev, std::vector<std::string> &arglist, int &narg ) {

    devlist.clear(); ndev=0;                     // empty the dev and arg list vectors
    arglist.clear(); narg=0;

    std::size_t devdelim = str.find( ":" );      // position of device delimiter

    // If there is a device delimiter then build a vector of the device numbers
    //
    if ( devdelim != std::string::npos ) {
      std::string dev_str = str.substr( 0, str.find( ":" ) );
      std::vector<std::string> tokens;
      Tokenize( dev_str, tokens, "," );          // Tokenize the dev string on the comma ","
      for ( auto tok : tokens ) {
        try {
          devlist.push_back( std::stoi( tok ) );
        }
        catch (std::invalid_argument &) {
          ndev=-1;
          return;
        }
        catch ( std::out_of_range & ) {
          ndev=-1;
          return;
        }
      }
      ndev = devlist.size();
    }

    // Anything left, look for space-delimited tokens for the arg list
    //
    std::string arg_str = str.substr( devdelim+1 );
    std::vector<std::string> tokens;
    Tokenize( arg_str, tokens, " " );            // Tokenize the arg string on the space " "
    for ( auto tok : tokens ) {
      arglist.push_back( tok );
    }
    narg = arglist.size();

    return;
  }
  /** Tokenize ****************************************************************/


  /** chrrep ******************************************************************/
  /**
   * @fn         chrrep
   * @brief      replace one character within a string with a new character
   * @param[out] str     pointer to string
   * @param[in]  oldchr  the old character to replace in the string
   * @param[in]  newchr  the replacement value
   * @return     none
   *
   * This function modifies the original string pointed to by *str.
   *
   */
  void chrrep(char *str, char oldchr, char newchr) {
    char *p = str;
    int   i=0;
    while ( *p ) {
      if (*p == oldchr) {
        /**
         * special case for DEL character. move string over by one char
         */
        if (newchr == 127) {    // if newchr==DEL copy memory after that chr
          memmove(&str[i], &str[i+1], strlen(str)-i);
        }
        else {                  // otherwise just replace chr
          *p = newchr;
        }
      }
      ++p; i++;                 // increment pointer and byte counter
    }
  }
  /** chrrep ******************************************************************/


  /** string_replace_char *****************************************************/
  /**
   * @fn         string_replace_char
   * @brief      replace one character within a std::string with a new character
   * @param[out] reference to str
   * @param[in]  oldchar is the char to replace
   * @param[in]  newchar is the replacement value
   * @return     none
   *
   */
  void string_replace_char(std::string &str, const char *oldchar, const char *newchar) {
    while (str.find(oldchar) != std::string::npos) {
      if ( str.find(oldchar) != std::string::npos ) str.replace(str.find(oldchar), 1, newchar);
    }
  }
  /** string_replace_char *****************************************************/


  /** get_time ****************************************************************/
  /**
   * @fn         get_time
   * @brief      gets the current time and returns values by reference
   * @param[out] int & year
   * @param[out] int & mon
   * @param[out] int & mday
   * @param[out] int & hour
   * @param[out] int & min
   * @param[out] int & sec
   * @param[out] int & usec
   * @return     1 on error, 0 if okay
   *
   */
  long get_time( int &year, int &mon, int &mday, int &hour, int &min, int &sec, int &usec ) {
    std::stringstream current_time;  // String to contain the time
    time_t t;                        // Container for system time
    struct timespec timenow;         // Time of day container
    struct tm mytime;                // GMT time container
    long error = 0;

    // Get the system time, return a bad timestamp on error
    //
    if ( clock_gettime( CLOCK_REALTIME, &timenow ) != 0 ) error = 1;

    // Convert the time of day to local or GMT
    //
    if ( !error ) {
      t = timenow.tv_sec;
      if ( zone == "local" )  { if ( localtime_r( &t, &mytime ) == NULL ) error = 1; }
      else {
        if ( gmtime_r( &t, &mytime ) == NULL ) error = 1;
      }
    }

    // prepare the return values
    //
    if ( error == 0 ) {
        year = mytime.tm_year + 1900;
        mon  = mytime.tm_mon + 1;
        mday = mytime.tm_mday;
        hour = mytime.tm_hour;
        min  = mytime.tm_min;
        sec  = mytime.tm_sec;
        usec = (int)(timenow.tv_nsec/1000);
    }
    else {
      year = 9999;
      mon  = 99;
      mday = 99;
      hour = 99;
      min  = 99;
      sec  = 99;
      usec = 999999;
    }

    return( error );
  }
  /** get_time ****************************************************************/


  /** get_timestamp ***********************************************************/
  /**
   * @fn         get_timestamp
   * @brief      return a string of the current time "YYYY-MM-DDTHH:MM:SS.ssssss"
   * @param[in]  none
   * @return     string
   *
   */
  std::string get_timestamp() {
    std::stringstream current_time;  // String to contain the time
    time_t t;                        // Container for system time
    struct timespec timenow;         // Time of day container
    struct tm mytime;                // GMT time container

    // Get the system time, return a bad timestamp on error
    //
    if ( clock_gettime( CLOCK_REALTIME, &timenow ) != 0 ) return("9999-99-99T99:99:99.999999");

    // Convert the time of day to local or GMT
    //
    t = timenow.tv_sec;
    if ( zone == "local" ) { if ( localtime_r( &t, &mytime ) == NULL ) return( "9999-99-99T99:99:99.999999" ); }
    else                   { if ( gmtime_r( &t, &mytime ) == NULL )    return( "9999-99-99T99:99:99.999999" ); }

    current_time.setf(std::ios_base::right);
    current_time << std::setfill('0') << std::setprecision(0)
                 << std::setw(4) << mytime.tm_year + 1900   << "-"
                 << std::setw(2) << mytime.tm_mon + 1 << "-"
                 << std::setw(2) << mytime.tm_mday    << "T"
                 << std::setw(2) << mytime.tm_hour  << ":"
                 << std::setw(2) << mytime.tm_min << ":"
                 << std::setw(2) << mytime.tm_sec
                 << "." << std::setw(6) << timenow.tv_nsec/1000;

    return(current_time.str());
  }
  /** get_timestamp ***********************************************************/


  /** get_system_date *********************************************************/
  /**
   * @fn         get_system_date
   * @brief      return current date in formatted string "YYYYMMDD"
   * @param[in]  none
   * @return     string
   *
   */
  std::string get_system_date() {
    std::stringstream current_date;  // String to contain the return value
    time_t t;                        // Container for system time
    struct timespec timenow;;        // Time of day container
    struct tm mytime;                // time container

    // Get the system time, return a bad datestamp on error
    //
    if ( clock_gettime( CLOCK_REALTIME, &timenow ) != 0 ) return( "99999999" );

    // Convert the time of day to GMT
    //
    t = timenow.tv_sec;
    if ( zone == "local" ) { if ( localtime_r( &t, &mytime ) == NULL ) return( "9999-99-99T99:99:99.999999" ); }
    else                   { if ( gmtime_r( &t, &mytime ) == NULL )    return( "9999-99-99T99:99:99.999999" ); }

    current_date.setf(std::ios_base::right);
    current_date << std::setfill('0') << std::setprecision(0)
                 << std::setw(4) << mytime.tm_year + 1900
                 << std::setw(2) << mytime.tm_mon + 1
                 << std::setw(2) << mytime.tm_mday;

    return( current_date.str() );
  }
  /** get_system_date *********************************************************/


  /** get_file_time ***********************************************************/
  /**
   * @fn         get_file_time
   * @brief      return current time in formatted string "YYYYMMDDHHMMSS"
   * @param[in]  none
   * @return     string
   *
   * Used for filenames
   *
   */
  std::string get_file_time() {
    std::stringstream current_time;  // String to contain the time
    time_t t;                        // Container for system time
    struct timespec timenow;         // Time of day container
    struct tm mytime;                // time container

    // Get the system time, return a bad timestamp on error
    //
    if ( clock_gettime( CLOCK_REALTIME, &timenow ) != 0 ) return( "99999999999999" );

    // Convert the time of day to local or GMT
    //
    t = timenow.tv_sec;
    if ( zone == "local" ) { if ( localtime_r( &t, &mytime ) == NULL ) return( "99999999999999" ); }
    else                   { if ( gmtime_r( &t, &mytime ) == NULL )    return( "99999999999999" ); }

    current_time.setf(std::ios_base::right);
    current_time << std::setfill('0') << std::setprecision(0)
                 << std::setw(4) << mytime.tm_year + 1900
                 << std::setw(2) << mytime.tm_mon + 1
                 << std::setw(2) << mytime.tm_mday
                 << std::setw(2) << mytime.tm_hour
                 << std::setw(2) << mytime.tm_min
                 << std::setw(2) << mytime.tm_sec;

    return(current_time.str());
  }
  /** get_file_time ***********************************************************/


  /** get_clock_time **********************************************************/
  /**
   * @fn         get_clock_time
   * @brief      get the current clock time using REALTIME flag from the processor
   * @param[in]  none
   * @return     time in seconds
   *
   */
  double get_clock_time() {
    struct timespec data;  // Container for the current time
    if (clock_gettime(CLOCK_REALTIME, &data) != 0) return 0;
    return ( data.tv_sec + (data.tv_nsec / 1000000000.0) );
  }
  /** get_clock_time **********************************************************/


  /** timeout *****************************************************************/
  /**
   * @fn         timeout
   * @brief      
   * @param[in]  seconds
   * @param[in]  next_sec
   * @return     none
   *
   */
  void timeout(float seconds, bool next_sec) {
    if (seconds < 1 && seconds > 0) {
      usleep(seconds * 1000000);
    }
    else {
      // Sleep the number of seconds called for
      //
      if (seconds > 0) {
        sleep((int)seconds);
      }

      if (next_sec == true) {
        // Get the current time to the microsecond
        //
        struct timespec data;                // Time of day container
        if(clock_gettime(CLOCK_REALTIME, &data) != 0) return;

        // If the microseconds are less than 0.9999999, we sleep that long
        //
        if (data.tv_nsec/1000000000 < 0.999999999) {
          usleep((useconds_t)((999999999-data.tv_nsec)/1000));
        }
      }
    }
  }
  /** timeout *****************************************************************/


  /** compare_versions ********************************************************/
  /**
   * @fn         compare_versions
   * @brief      compares two version numbers represented as strings
   * @param[in]  v1
   * @param[in]  v2
   * @return     0,1,-1,-999
   *
   * This function compares version or revision numbers which are represented
   * as strings which contain decimals. Each portion of a version number is
   * compared.
   *
   * Returns 1 if v2 is smaller, -1 if v1 is smaller, 0 if equal
   * Returns -999 on error
   *
   * Revision numbers can be like n.n.n etc with any number of numbers n,
   * but must be numbers.
   *
   */
  int compare_versions(std::string v1, std::string v2) {
    std::vector<std::string> tokens1;
    std::vector<std::string> tokens2;

    // Tokenize the version numbers, using the decimal point "." as the separator
    //
    Tokenize( v1, tokens1, "." );
    Tokenize( v2, tokens2, "." );

    // Compare each token.
    // As soon as one is greater than the other then return.
    //
    for (size_t i=0,j=0; ( i < tokens1.size() && j < tokens2.size() ); i++,j++) {
      try {
        if ( std::stoi( tokens1.at(i) ) > std::stoi( tokens2.at(j) ) ) return 1;
        if ( std::stoi( tokens2.at(j) ) > std::stoi( tokens1.at(i) ) ) return -1;
      }
      catch (std::invalid_argument &) {
        return( -999 );
      }
      catch ( std::out_of_range & ) {
        return( -999 );
      }
    }

    // If we finished the loop then either they were all equal or
    // one version had more tokens than the other (E.G., 1.1.123 vs 1.1).
    // The one with more tokens has to be greater.
    //
    if ( tokens1.size() > tokens2.size() ) return 1;
    if ( tokens2.size() > tokens1.size() ) return -1;

    return 0;      // or they are equal
  }
  /** compare_versions ********************************************************/

