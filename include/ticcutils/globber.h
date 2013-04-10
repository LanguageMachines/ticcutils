/*
  $Id: Version.h 15979 2013-04-10 12:38:38Z sloot $
  $URL: https://ilk.uvt.nl/svn/sources/libticcutils/trunk/include/ticcutils/Version.h $

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

#ifndef TICC_GLOBBER_H
#define TICC_GLOBBER_H

namespace TiCC {
  std::vector<std::string> glob( const std::string&);
}

#endif // TICC_GLOBBER_H
