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

#include "ticcutils/Unicode.h"
#include <exception>
#include <stdexcept>
#include "unicode/normalizer2.h"

using namespace std;

namespace TiCC {

  UnicodeString UnicodeFromEnc( const string& s, const string& enc ){
    return UnicodeString( s.c_str(), s.length(), enc.c_str() );
  }

  string UnicodeToUTF8( const UnicodeString& s ){
    string result;
    s.toUTF8String(result);
    return result;
  }

  UnicodeNormalizer::UnicodeNormalizer( const std::string& enc ){
    UErrorCode err = U_ZERO_ERROR;
    if ( enc == ""
	 || enc == "NFC" )
      _normalizer = Normalizer2::getNFCInstance( err );
    else if ( enc == "NFD" )
      _normalizer = Normalizer2::getNFDInstance( err );
    else if ( enc == "NFKC" )
      _normalizer = Normalizer2::getNFKCInstance( err );
    else if ( enc == "NFKD" )
      _normalizer = Normalizer2::getNFKDInstance( err );
    else {
      throw std::logic_error( "invalid normalization mode: " + enc );
    }
  }

  UnicodeString UnicodeNormalizer::normalize( const UnicodeString& us ){
    if ( _normalizer == 0 ){
      return us;
    }
    else {
      UErrorCode status=U_ZERO_ERROR;
      UnicodeString r = _normalizer->normalize( us, status );
      if (U_FAILURE(status)){
	throw std::invalid_argument("Normalizer");
      }
      return r;
    }
  }
}
