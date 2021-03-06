/*
  Copyright (c) 2006 - 2021
  CLST  - Radboud University
  ILK   - Tilburg University

  This file is part of ticcutils

  ticcutils is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  ticcutils is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      https://github.com/LanguageMachines/ticcutils/issues
  or send mail to:
      lamasoftware (at ) science.ru.nl

*/

#include "ticcutils/StringOps.h"

#include <cerrno>
#include <cfloat>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include "ticcutils/Version.h"

using namespace std;
namespace TiCC {

  string BuildInfo() {
    // cannot be defined in the header because otherwise __DATE__ and
    // __TIME__ would be dynamic. (changing every time it is included)
    return VersionName() + "-" + Version() + ". Compiled on "
      + __DATE__ + " " + __TIME__;
  }

  string trim( const string& s, const string& chars ){
    /// remove leading and trailing characters from a string
    /*!
      \param s the string to trim
      \param chars the characters to remove. The default is whitespace.
      When \e chars = "", the following chars are used: " \t\r\n"
    */
    string result;
    if ( !s.empty() ){
      string::size_type b_pos = s.find_first_not_of( chars );
      if ( b_pos == string::npos ){
	return result; // 'empty' string. only garbage
      }
      string::size_type e_pos = s.find_last_not_of( chars );
      if ( e_pos == string::npos ){
	result = s.substr( b_pos );
      }
      else {
	result = s.substr( b_pos, e_pos-b_pos+1 );
      }
    }
    return result;
  }

  string trim_front( const string& s, const string& chars ){
    /// remove leading characters from a string
    /*!
      \param s the string to trim
      \param chars the characters to remove. The default is whitespace.
      When \e chars = "", the following chars are used: " \t\r\n"
    */
    string result;
    if ( !s.empty() ){
      string::size_type b_pos = s.find_first_not_of( chars );
      if ( b_pos != string::npos ){
	result = s.substr( b_pos );
      }
    }
    return result;
  }

  string trim_back( const string& s, const string& chars ){
    /// remove trailing characters from a string
    /*!
      \param s the string to trim
      \param chars the characters to remove. The default is whitespace.
      When \e chars = "", the following chars are used: " \t\r\n"
    */
    string result;
    if ( !s.empty() ){
      string::size_type e_pos = s.find_last_not_of( chars );
      if ( e_pos != string::npos ){
	result = s.substr( 0, e_pos+1 );
      }
    }
    return result;
  }

  static int toLower( const int& i ){ return tolower(i); }
  static int toUpper( const int& i ){ return toupper(i); }

  void to_lower( string& s ){
    /// convert to all lowercase. Modifies the input.
    transform( s.begin(), s.end(), s.begin(), toLower );
  }

  void to_upper( string& s ){
    /// convert to all uppercase. Modifies the input.
    transform( s.begin(), s.end(), s.begin(), toUpper );
  }

  string lowercase( const string& s ){
    /// return a lowercased copy of the inputstring
    string result = s;
    to_lower( result );
    return result;
  }

  string uppercase( const string& s ){
    /// return a uppercased copy of the inputstring
    string result = s;
    to_upper( result );
    return result;
  }

  bool match_back( const std::string& s, const std::string& tail ){
    /// check if a string matches another at the back
    /*!
      \param s the string
      \param tail the string we search in \e s
      \return true is \e tail is the last pasrt of \e s
    */
    int res = -2;
    try {
      res = s.compare( s.length() - tail.length(), tail.length(), tail );
    }
    catch ( ... ){
    }
    if ( res == 0  ){
      return true;
    }
    return false;
  }

  bool match_front( const std::string& s, const std::string& head ){
    /// check if a string matches another at the front
    /*!
      \param s the string
      \param head the string we search in \e s
      \return true is \e head is the first part of \e s
    */
    int res = s.compare(0,head.length(),head);
    if ( res == 0  ){
      return true;
    }
    return false;
  }

  size_t split_at( const string& src, vector<string>& results,
		   const string& sep, bool exact ){
    /// split a string into substrings.
    /*!
      \param src the string to split
      \param results a vector of split parts
      \param sep a separator string. May be a multi-character string.
      \param exact normally, we silently skip empty entries (e.g. when two or
      more separators co-incide), but not when exact=true. In that case result
      may contain empty strings.
      \return the number of parts found
    */
    if ( sep.empty() ){
      throw runtime_error( "TiCC::split_at(): separator is empty!" );
    }
    results.clear();
    string::size_type pos = 0;
    while ( pos != string::npos ){
      string res;
      string::size_type p = src.find( sep, pos );
      if ( p == string::npos ){
	res = src.substr( pos );
	pos = p;
      }
      else {
	res = src.substr( pos, p - pos );
	pos = p + sep.length();
      }
      if ( !res.empty() || exact ){
	results.push_back( res );
      }
    }
    return results.size();
  }

  vector<string> split_at( const string& src,
			   const string& sep,
			   size_t max ){
    /// split a string into substrings.
    /*!
      \param src the string to split
      \param sep a separator string. May be a multi-character string.
      \param max if max > 0, limit the size of the result to \e max strings,
      leaving the remainder in the last part of the result
      \return a vector of split parts
    */
    if ( sep.empty() ){
      throw runtime_error( "TiCC::split_at(): separator is empty!" );
    }
    vector<string> results;
    size_t cnt = 0;
    string::size_type pos = 0;
    while ( pos != string::npos ){
      string res;
      string::size_type p = src.find( sep, pos );
      if ( p == string::npos ){
	res = src.substr( pos );
	pos = p;
      }
      else {
	res = src.substr( pos, p - pos );
	pos = p + sep.length();
      }
      if ( !res.empty() ){
	++cnt;
	results.push_back( res );
      }
      if ( max != 0 && cnt >= max-1 ){
	if ( pos != string::npos ){
	  results.push_back( src.substr( pos ) );
	}
	break;
      }
    }
    return results;
  }

  size_t split_at_first_of( const string& src, vector<string>& results,
			    const string& seps, bool exact ){
    /// split a string into substrings.
    /*!
      \param src the string to split
      \param results a vector of split parts
      \param seps a string with separator characters. one of those should match
      for a split to happen.
      \param exact normally, we silently skip empty entries (e.g. when two or
      more separators co-incide), but not when exact=true. In that case result
      may contain empty strings.
      \return the number of parts found
    */
    if ( seps.empty() ){
      throw runtime_error( "TiCC::split_at_first_of(): separators are empty!" );
    }
    results.clear();
    string::size_type s = 0;
    while ( s != string::npos ){
      string res;
      string::size_type e = src.find_first_of( seps, s );
      if ( e == string::npos ){
	res = src.substr( s );
	s = e;
      }
      else {
	res = src.substr( s, e - s );
	s = e+1;
      }
      if ( !res.empty() || exact ){
	results.push_back( res );
      }
    }
    return results.size();
  }

  vector<string> split_at_first_of( const string& src,
				    const string& seps,
				    size_t max ){
    /// split a string into substrings.
    /*!
      \param src the string to split
      \param seps a string with separator characters. one of those should match
      for a split to happen.
      \param max if max > 0, limit the size of the result to \e max strings,
      leaving the remainder in the last part of the result
      \return a vector of split parts
    */
    if ( seps.empty() ){
      throw runtime_error( "TiCC::split_at_first_of(): separators are empty!" );
    }
    vector<string> results;
    size_t cnt = 0;
    string::size_type pos = 0;
    while ( pos != string::npos ){
      string res;
      string::size_type e = src.find_first_of( seps, pos );
      if ( e == string::npos ){
	res = src.substr( pos );
	pos = e;
      }
      else {
	res = src.substr( pos, e - pos );
	pos = e+1;
      }
      if ( !res.empty() ){
	results.push_back( res );
	++cnt;
      }
      if ( max != 0 && cnt >= max-1 ){
	if ( pos != string::npos ){
	  results.push_back( src.substr( pos ) );
	}
	break;
      }
    }
    return results;
  }

  string format_nonascii( const string& s ){
    /// format weird strings (like UTF8, LATIN1) printable
    // useful for debugging
    stringstream os;
    os << showbase << hex;
    for ( const auto& c : s ){
      if ( isprint(c) && (int)c > 31 ){
	os << c;
      }
      else {
	os << "-" << (short int)c << "-";
      }
    }
    os << noshowbase << dec;
    return os.str();
  }

  string basename( const string& path ){
    /// extract the basename of a path/filename
    string::size_type pos = path.rfind( "/" );
    if ( pos != string::npos ){
      return path.substr(pos+1);
    }
    else {
      return path;
    }
  }

  string dirname( const string& path ){
    /// extract the dirname of a path/filename
    string::size_type pos = path.rfind( "/" );
    if ( pos != string::npos ){
      return path.substr(0,pos);
    }
    else {
      return ".";
    }
  }

  string realpath( const string& path ){
    /// give the 'real' pathname for a relative path/filename
    string result;
    if ( path.empty() ){
      return result;
    }
    const char *in = path.c_str();
    char *out = 0;
    char *res = ::realpath( in, out );
    if ( res ){
      result = string(res);
      free( res);
    }
    return result;
  }

} // namespace TiCC
