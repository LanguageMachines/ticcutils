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
#include <vector>
#include "unicode/unistr.h"
#include "unicode/ustream.h"
#include "unicode/normalizer2.h"
#include "unicode/translit.h"
#include "unicode/regex.h"

namespace TiCC {

  std::string UnicodeToUTF8( const UnicodeString&  );

  UnicodeString UnicodeFromEnc( const std::string& ,
				const std::string& = "UTF8" );

  inline UnicodeString UnicodeFromUTF8( const std::string& s ){
    return UnicodeString::fromUTF8( s );
  }

  class UnicodeNormalizer {
  public:
    UnicodeNormalizer( const std::string& = "" );
    ~UnicodeNormalizer();
    UnicodeString normalize( const UnicodeString& );
    const std::string setMode( const std::string& );
    const std::string getMode() const { return mode; };
  private:
    const Normalizer2 *_normalizer;
    std::string mode;
  };

  class UnicodeRegexMatcher {
  public:
    UnicodeRegexMatcher( const UnicodeString&, const UnicodeString& name="" );
    ~UnicodeRegexMatcher();
    bool match_all( const UnicodeString&, UnicodeString&, UnicodeString&  );
    const UnicodeString get_match( unsigned int ) const;
    int NumOfMatches() const;
    int split( const UnicodeString&, std::vector<UnicodeString>& );
    UnicodeString Pattern() const;
    bool set_debug( bool b ){ bool r = _debug; _debug = b; return r; };
  private:
    UnicodeRegexMatcher( const UnicodeRegexMatcher& );  // inhibit copies
    UnicodeRegexMatcher& operator=( const UnicodeRegexMatcher& ); // inhibit copies
    RegexPattern *pattern;
    RegexMatcher *matcher;
    UnicodeRegexMatcher();
    std::vector<UnicodeString> results;
    const UnicodeString _name;
    bool _debug;
  };

  class UniFilter {
    friend std::ostream& operator<<( std::ostream&, const UniFilter& );
  public:
    UniFilter();
    ~UniFilter();
    bool init( const UnicodeString&, const UnicodeString& );
    bool fill( const std::string&, const std::string& = "" );
    bool add( const std::string& );
    bool add( const UnicodeString& );
    UnicodeString filter( const UnicodeString& );
    UnicodeString get_rules() const;
  private:
    Transliterator *_trans;
  };

  UnicodeString filter_diacritics( const UnicodeString& );
}
#endif // TICC_UNICODE_H
