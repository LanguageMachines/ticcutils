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

#ifndef TICC_STRING_OPS_H
#define TICC_STRING_OPS_H

#include <string>
#include <vector>

namespace TiCC {
  std::string trim( const std::string&, const std::string& = " \t\r\n" );
  std::string trim_front( const std::string&, const std::string& = " \t\r\n" );
  std::string trim_back( const std::string&, const std::string& = " \t\r\n" );

  void to_lower( std::string& );
  void to_upper( std::string& );
  std::string lowercase( const std::string& );
  std::string uppercase( const std::string& );

  size_t split_at( const std::string&, std::vector<std::string>&, 
		   const std::string& );
  size_t split_at_first_of( const std::string&, std::vector<std::string>&, 
			    const std::string& );
  inline size_t split( const std::string& s, std::vector<std::string>& vec ){
    return split_at_first_of( s, vec, " \r\t\n" );
  }
  
  std::string format_nonascii( const std::string& );
  
}

#endif
