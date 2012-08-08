/*
  $Id$
  $URL$

  Copyright (c) 1998 - 2012
  ILK   - Tilburg University
  CLiPS - University of Antwerp
 
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
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/

#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

#include <cerrno>
#include <cfloat>
#include "ticcutils/StringOps.h"

using namespace std;
namespace TiCC {

  size_t split_at( const string& src, vector<string>& results, 
		   const string& sep ){
    // split a string into substrings, using seps as seperator
    // silently skip empty entries (e.g. when two or more seperators co-incide)
    results.clear();
    string::size_type pos = 0, p;
    string res;
    while ( pos != string::npos ){
      p = src.find( sep, pos );
      if ( p == string::npos ){
	res = src.substr( pos );
	pos = p;
      }
      else {
	res = src.substr( pos, p - pos );
	pos = p + sep.length();
      }
      if ( !res.empty() )
	results.push_back( res );
    }
    return results.size();
  }

  size_t split_at_first_of( const string& src, vector<string>& results, 
			    const string& seps ){
    // split a string into substrings, using the characters in seps
    // as seperators
    // silently skip empty entries (e.g. when two or more seperators co-incide)
    results.clear();
    string::size_type e, s = src.find_first_not_of( seps );
    string res;
    while ( s != string::npos ){
      e = src.find_first_of( seps, s );
      if ( e == string::npos ){
	res = src.substr( s );
	s = e;
      }
      else {
	res = src.substr( s, e - s );
	s = src.find_first_not_of( seps, e );
      }
      if ( !res.empty() )
	results.push_back( res );
    }
    return results.size();
  }
  
  // format weird strings (like UTF8, LATIN1) printable 
  // useful for debugging
  string format_nonascii( const string& s ){
    stringstream os;
    os << showbase << hex;
    unsigned int i;
    for ( i=0; i < s.length(); ++i )
      if ( isprint(s[i]) && (int)s[i] > 31 )
	os << s[i];
      else
	os << "-" << (short int)s[i] << "-";
    os << noshowbase << dec;
    return os.str();
  }

} // namespace TiCC
