/*
  $Id$
  $URL$

  Copyright (c) 2006 - 2015
  Tilburg University

  This file is part of ticcutils.

  ticcutils is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  ticcutils is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/

#ifndef TICC_CONFIGURATION_H
#define TICC_CONFIGURATION_H

#include <string>
#include <map>
#include <set>
#include <iosfwd>

namespace TiCC {
  class Configuration {
    typedef std::map<std::string,std::string> ssMap;
    typedef std::map<std::string, ssMap> sssMap;
  public:
    Configuration();
    bool fill( const std::string& );
    bool fill( const std::string&, const std::string& );
    bool hasSection( const std::string& ) const;
    std::string lookUp( const std::string&, const std::string& = "" ) const;
    ssMap lookUpAll( const std::string& ) const;
    std::set<std::string> lookUpSections() const;
    std::string setatt( const std::string&,
			const std::string&,
			const std::string& = "" );
    void dump( std::ostream& ) const;
    std::string configDir() const { return cdir; };
  private:
    sssMap myMap;
    std::string cdir;
  };
}
#endif
