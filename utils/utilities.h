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

extern std::string zone;

bool cmdOptionExists( char** begin, char** end, const std::string &option );
char* getCmdOption( char** begin, char** end, const std::string &option );
int my_hardware_concurrency();
int cores_available();

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

void timeout(float seconds=0, bool next_sec=true);

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

#endif
