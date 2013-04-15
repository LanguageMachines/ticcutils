/*
  $Id$
  $URL$

  Copyright (c) 1998 - 2013
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

#ifndef TICC_FILE_UTILS_H
#define TICC_FILE_UTILS_H
#include "config.h"

namespace TiCC {
  std::vector<std::string> glob( const std::string&);
  std::vector<std::string> searchFilesExt( const std::string&,
					   const std::string&,
					   bool = true );
  std::vector<std::string> searchFiles( const std::string& name,
					bool recurse = true ){
    return searchFilesExt( name, "", recurse );
  }
  std::vector<std::string> searchFilesMatch( const std::string&,
					     const std::string&,
					     bool = true );
  bool isDir( const std::string& );
  bool isFile( const std::string& );

}

#endif // TICC_FILE_UTILS_H
