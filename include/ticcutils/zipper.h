/*
  Copyright (c) 2006 - 2016
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

#ifndef TICC_ZIP_TOOLS_H
#define TICC_ZIP_TOOLS_H

namespace TiCC {

  bool bz2Compress( const std::string&, const std::string& = "" );
  bool bz2Decompress( const std::string&, const std::string& = "" );
  std::string bz2ReadFile( const std::string& );
  std::string bz2ReadStream( std::istream& );
  bool bz2WriteFile( const std::string&, const std::string& );
  bool bz2WriteStream( std::ostream& );

  bool gzCompress( const std::string&, const std::string& = "" );
  bool gzDecompress( const std::string&, const std::string& = "" );
  std::string gzReadFile( const std::string& );
  std::string gzReadStream( std::istream& );
  bool gzWriteFile( const std::string&, const std::string& );
  bool gzWriteStream( std::ostream& );


} // namespace TiCC

#endif
