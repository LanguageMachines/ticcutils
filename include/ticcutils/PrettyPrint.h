/*
  Copyright (c) 2006 - 2025
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

#ifndef TICC_PRETTY_PRINT_H
#define TICC_PRETTY_PRINT_H

#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <ostream>

namespace TiCC {


  template< typename S, typename T >
  inline std::ostream& operator<< ( std::ostream& os, const std::pair<S,T>& p ){
    os << "<" << p.first << "," << p.second << ">";
    return os;
  }

  template< typename T >
    inline std::ostream& operator<< ( std::ostream& os, const std::set<T>& s ){
    os << "{";
    for( const auto& it : s ){
      if ( &it != &(*s.begin()) ){
	os << ",";
      }
      os << it;
    }
    os << "}";
    return os;
  }

  template< typename T >
  inline std::ostream& operator<< ( std::ostream& os,
				    const std::multiset<T>& ms ){
    os << "{";
    for( const auto& it : ms ){
      if ( &it != &(*ms.begin()) ){
	os << ",";
      }
      os << it;
    }
    os << "}";
    return os;
  }

  template< typename T >
    inline std::ostream& operator<<( std::ostream& os,
				     const std::list<T>& ls ){
    os << "[";
    for ( const auto& it : ls ){
      os << it;
      if ( &it != &ls.back() )
	os << ",";
    }
    os << "]";
    return os;
  }

  template< typename T >
  inline std::ostream& operator<< ( std::ostream& os,
				    const std::vector<T>& v ){
    os << "[";
    for( const auto& it : v ){
      os << it;
      if ( &it != &v.back() ){
	os << ",";
      }
    }
    os << "]";
    return os;
  }

  template< typename S, typename T >
  inline std::ostream& operator<< ( std::ostream& os,
				    const std::map<S,T>& m ){
    os << "{";
    for( const auto& it : m ){
      if ( &it != &(*m.begin()) ){
	os << ",";
      }
      os << "<" << it.first << "," << it.second << ">";
    }
    os << "}";
    return os;
  }

  template< typename S, typename T, typename U >
  std::ostream& operator<< ( std::ostream& os,
			     const std::map<S,T,U>& m ){
    os << "{";
    for ( const auto& it : m ){
      if ( &it != &(*m.begin()) ){
	os << ",";
      }
      os << "<" << it.first << "," << it.second << ">";
    }
    os << "}";
    return os;
  }

  template< typename S, typename T >
  inline std::ostream& operator<< ( std::ostream& os,
				    const std::multimap<S,T>& mm ){
    os << "{";
    for ( const auto& it : mm ){
      if ( &it != &(*mm.begin()) ){
	os << ",";
      }
      os << "<" << it.first << "," << it.second << ">";
    }
    os << "}";
    return os;
  }

  template< typename S, typename T, typename SORT >
  inline std::ostream& operator<< ( std::ostream& os,
				    const std::multimap<S,T,SORT>& mm ){
    os << "{";
    for ( const auto& it : mm ){
      if ( &it != &(*mm.begin()) ){
	os << ",";
      }
      os << "<" << it.first << "," << it.second << ">";
    }
    os << "}";
    return os;
  }

}
#endif // PRETTY_PRINT_H
