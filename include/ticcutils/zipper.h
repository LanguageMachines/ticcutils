/*
  $Id: XMLtools.h 15892 2013-04-03 08:49:18Z sloot $
  $URL: https://ilk.uvt.nl/svn/sources/libticcutils/trunk/include/ticcutils/XMLtools.h $

  Copyright (c) 1998 - 2013
  ILK   - Tilburg University
  CLiPS - University of Antwerp
 
  This file is part of ticcutils

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

#ifndef TICC_ZIP_TOOLS_H
#define TICC_ZIP_TOOLS_H

#include "bzlib.h"

namespace TiCC {

  bool bz2compress( const std::string&, const std::string& = "" );
  bool bz2decompress( const std::string&, const std::string& = "" );

  std::string bz2readFile( const std::string& );
  bool bz2writeFile( const std::string&, const std::string& );

} // namespace TiCC

#endif
