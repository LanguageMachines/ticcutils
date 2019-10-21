/*
  Copyright (c) 2006 - 2019
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

#include <vector>
#include <string>

#ifndef TICC_FILE_UTILS_H
#define TICC_FILE_UTILS_H

namespace TiCC {
  std::vector<std::string> glob( const std::string&);
  std::vector<std::string> searchFilesExt( const std::string&,
					   const std::string&,
					   bool = true );
  inline std::vector<std::string> searchFiles( const std::string& name,
					bool recurse = true ){
    return searchFilesExt( name, "", recurse );
  }
  std::vector<std::string> searchFilesMatch( const std::string&,
					     const std::string&,
					     bool = true );
  bool isDir( const std::string& );
  bool isFile( const std::string& );

  bool createPath( const std::string& );

  std::string tempname( const std::string& );

}

#endif // TICC_FILE_UTILS_H
