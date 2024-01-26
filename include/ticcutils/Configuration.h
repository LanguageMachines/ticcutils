/*
  Copyright (c) 2006 - 2024
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

#ifndef TICC_CONFIGURATION_H
#define TICC_CONFIGURATION_H

#include <string>
#include <map>
#include <set>
#include <iostream>

namespace TiCC {

  /// \brief a class to store configurations
  ///
  /// This class stores configuration information as key-value pairs.
  /// The Configuration is devided in sections. A key must be unique \e per
  /// \e section.
  ///
  /// We provide functions to read a Configuration from a file, or to set
  /// an attribute-value in a certain section.
  ///
  /// There is an implicit \b global section when the section is unnamed.
  class Configuration {
  public:
    typedef std::map<std::string,std::string> ssMap;
    typedef std::map<std::string, ssMap> sssMap;
    Configuration();
    void merge( const Configuration&, bool = false );
    bool fill( const std::string& );
    bool fill( const std::string&, const std::string& );
    bool hasSection( const std::string& ) const;
    std::string lookUp( const std::string& v,
			const std::string& s = "" ) const {
      /// alternative name for getatt() function
      return getatt( v, s );
    }
    ssMap lookUpAll( const std::string& ) const;
    std::set<std::string> lookUpSections() const;
    std::string getatt( const std::string&,
			const std::string& = "" ) const;
    std::string setatt( const std::string&,
			const std::string&,
			const std::string& = "" );
    std::string clearatt( const std::string&,
			  const std::string& = "" );
    void erasesection( const std::string& );
    void dump( std::ostream& ) const;
    void create_configfile( const std::string& ) const;
    std::string configDir() const {
      /// give the value of the configDir attribute. (set when parsing a file)
      return lookUp( "configDir" );
    };
  private:
    sssMap myMap;
    bool get_att_val( const std::string&, const std::string& );
  };

  inline std::ostream& operator<<( std::ostream& os, const Configuration& c ){
    /// output a Configuration to a stream
    c.dump(os);
    return os;
  }

  inline std::ostream& operator<<( std::ostream& os, const Configuration* c ){
    /// output a Configuration to a stream
    if ( c ){
      c->dump(os);
    }
    else {
      os << "empty config";
    }
    return os;
  }

}
#endif
