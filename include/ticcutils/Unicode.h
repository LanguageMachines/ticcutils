#ifndef TICC_UNICODE_H
#define TICC_UNICODE_H

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

#include <string>
#include "unicode/unistr.h"

namespace TiCC {

  std::string UnicodeToUTF8( const UnicodeString&  );

  UnicodeString UnicodeFromEnc( const std::string& ,
				const std::string& = "UTF8" );

  inline UnicodeString UnicodeFromUTF8( const std::string& s ){
    return UnicodeString::fromUTF8( s );
  }

}
#endif // TICC_UNICODE_H
