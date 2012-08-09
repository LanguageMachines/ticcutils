/*
  $Id$
  $URL$

  Copyright (c) 1998 - 2012
  ILK   - Tilburg University
  CLiPS - University of Antwerp
 
  This file is part of timbl

  timbl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  timbl is distributed in the hope that it will be useful,
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

#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <ostream>

namespace TiCC {

  template< typename T >
    inline std::ostream& operator<< ( std::ostream& os, const std::set<T>& s ){
    os << "{";
    typename std::set<T>::const_iterator it = s.begin();
    while ( it != s.end() ){
      os << *it;
      ++it;
      if ( it != s.end() )
	os << ",";
    }
    os << "}";
    return os;
  }

  template< typename T >
    inline std::ostream& operator<<( std::ostream& os, const std::list<T>& s ){
    os << "[";
    typename std::list<T>::const_iterator it = s.begin();
    while ( it != s.end() ){
      os << *it;
      ++it;
      if ( it != s.end() )
	os << ",";
    }
    os << "]";
    return os;
  }
  
  template< typename T >
    inline std::ostream& operator<< ( std::ostream& os, const std::vector<T>& s ){
    os << "[";
    typename std::vector<T>::const_iterator it = s.begin();
    while ( it != s.end() ){
      os << *it;
      ++it;
      if ( it != s.end() )
	os << ",";
    }
    os << "]";
    return os;
  }

  template< typename S, typename T >
    inline std::ostream& operator<< ( std::ostream& os, const std::map<S,T>& s ){
    os << "{";
    typename std::map<S,T>::const_iterator it = s.begin();
    while ( it != s.end() ){
      os << "<" << it->first << "," << it->second << ">";
      ++it;
      if ( it != s.end() )
	os << ",";
    }
    os << "}";
    return os;
  }

  template< typename S, typename T, typename U >
    std::ostream& operator<< ( std::ostream& os, const std::map<S,T,U>& s ){
    os << "{";
    typename std::map<S,T,U>::const_iterator it = s.begin();
    while ( it != s.end() ){
      os << "<" << it->first << "," << it->second << ">";
      ++it;
      if ( it != s.end() )
	os << ",";
    }
    os << "}";
    return os;
  }

  template< typename S, typename T >
    inline std::ostream& operator<< ( std::ostream& os, const std::multimap<S,T>& s ){
    os << "{";
    typename std::multimap<S,T>::const_iterator it = s.begin();
    while ( it != s.end() ){
      os << "<" << it->first << "," << it->second << ">";
      ++it;
      if ( it != s.end() )
	os << ",";
    }
    os << "}";
    return os;
  }
  
}
#endif // PRETTY_PRINT_H
