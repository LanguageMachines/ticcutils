/*
  $Id$
  $URL$

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

namespace TiCC {

  bool bz2Compress( const std::string&, const std::string& = "" );
  bool bz2Decompress( const std::string&, const std::string& = "" );
  std::string bz2ReadFile( const std::string& );
  bool bz2WriteFile( const std::string&, const std::string& );

  bool gzCompress( const std::string&, const std::string& = "" );
  bool gzDecompress( const std::string&, const std::string& = "" );
  std::string gzReadFile( const std::string& );
  bool gzWriteFile( const std::string&, const std::string& );


} // namespace TiCC

#endif
