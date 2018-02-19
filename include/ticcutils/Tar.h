/*
  Copyright (c) 2006 - 2018
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

#ifndef TICC_TAR_TOOLS_H
#define TICC_TAR_TOOLS_H

#include "libtar.h"
#include <fstream>
#include <vector>

namespace TiCC {

  class tar {
  public:
    tar();
    ~tar();
    bool open( const std::string& );
    bool extract_file_names( std::vector<std::string>&,
			     const std::string& = "" );
    bool extract_file_names_match( std::vector<std::string>&,
				   const std::string& );
    bool next_ifstream( std::ifstream&, std::string& );
    bool extract_ifstream( const std::string&, std::ifstream& );
    bool close();
  private:
    TAR *tar_file;
    std::string tarname;
    tar( const tar& ); // no copies
    tar& operator=( const tar& ); // no copies
  };

} // namespace TiCC

#endif
