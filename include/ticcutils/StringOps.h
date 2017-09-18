/*
  Copyright (c) 2006 - 2017
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

#ifndef TICC_STRING_OPS_H
#define TICC_STRING_OPS_H

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace TiCC {
  std::string trim( const std::string&, const std::string& = " \t\r\n" );
  std::string trim_front( const std::string&, const std::string& = " \t\r\n" );
  std::string trim_back( const std::string&, const std::string& = " \t\r\n" );

  void to_lower( std::string& );
  void to_upper( std::string& );
  std::string lowercase( const std::string& );
  std::string uppercase( const std::string& );

  bool match_front( const std::string&, const std::string& );
  bool match_back( const std::string&, const std::string& );

  size_t split_at( const std::string&, std::vector<std::string>&,
		   const std::string&, bool = false );
  size_t split_at_first_of( const std::string&, std::vector<std::string>&,
			    const std::string&, bool = false );
  inline size_t split( const std::string& s,
		       std::vector<std::string>& vec,
		       bool exact = false ){
    return split_at_first_of( s, vec, " \r\t\n", exact );
  }

  std::vector<std::string> split_at( const std::string&,
				     const std::string&,
				     size_t = 0 );
  std::vector<std::string> split_at_first_of( const std::string&,
					      const std::string&,
					      size_t = 0 );
  inline std::vector<std::string> split( const std::string& s,
					 size_t num = 0 ){
    return split_at_first_of( s, " \r\t\n", num );
  }

  std::string format_nonascii( const std::string& );

  template< typename T >
    inline T stringTo( const std::string& str ) {
    T result;
    std::stringstream dummy ( str );
    if ( !( dummy >> result ) ) {
      throw( std::runtime_error( "conversion from string '"
				 + str + "' failed" ) );
    }
    return result;
  }

  template<>
    inline bool stringTo<bool>( const std::string& str ) {
    std::string b = TiCC::uppercase( str );
    if ( b == "YES" || b == "TRUE" || b == "1" )
      return true;
    else if ( b == "FALSE" || b == "NO" || b == "0" )
      return false;
    else
      throw( std::runtime_error( "conversion from string '"
				 + str + "' to bool failed" ) );
  }

  template< typename T >
    inline bool stringTo( const std::string& str, T& result ) {
    try {
      result = stringTo<T>( str );
      return true;
    }
    catch( ... ){
     return false;
    }
  }

  template <typename T>
    inline bool stringTo( const std::string& s, T &answer, T low, T upp ){
    try {
      T tmp = stringTo<T>( s );
      if ( (tmp >= low) && (tmp <= upp) ){
	answer = tmp;
	return true;
      }
      return false;
    }
    catch(...){
      return false;
    }
  }

  template< typename T >
    inline std::string toString ( const T& obj, bool=false ) {
    std::stringstream dummy;
    if ( !( dummy << obj ) ) {
      throw( std::runtime_error( "conversion failed" ) );
    }
   return dummy.str();
  }

  std::string basename( const std::string& );
  std::string dirname( const std::string& );

}

#endif
