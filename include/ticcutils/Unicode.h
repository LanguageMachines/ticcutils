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

  std::string UnicodeToUTF8( const icu::UnicodeString&  );

  icu::UnicodeString UnicodeFromEnc( const std::string& ,
				const std::string& = "UTF8" );

  inline icu::UnicodeString UnicodeFromUTF8( const std::string& s ){
    return icu::UnicodeString::fromUTF8( s );
  }

  class UnicodeNormalizer {
  public:
    UnicodeNormalizer( const std::string& = "" );
    ~UnicodeNormalizer();
    icu::UnicodeString normalize( const icu::UnicodeString& );
    const std::string setMode( const std::string& );
    const std::string getMode() const { return mode; };
  private:
    const icu::Normalizer2 *_normalizer;
    std::string mode;
  };

  class UnicodeRegexMatcher {
  public:
    UnicodeRegexMatcher( const icu::UnicodeString&, const icu::UnicodeString& name="" );
    ~UnicodeRegexMatcher();
    bool match_all( const icu::UnicodeString&, icu::UnicodeString&, icu::UnicodeString&  );
    const icu::UnicodeString get_match( unsigned int ) const;
    int NumOfMatches() const;
    int split( const icu::UnicodeString&, std::vector<icu::UnicodeString>& );
    icu::UnicodeString Pattern() const;
    bool set_debug( bool b ){ bool r = _debug; _debug = b; return r; };
  private:
    UnicodeRegexMatcher( const UnicodeRegexMatcher& );  // inhibit copies
    UnicodeRegexMatcher& operator=( const UnicodeRegexMatcher& ); // inhibit copies
    icu::RegexPattern *pattern;
    icu::RegexMatcher *matcher;
    UnicodeRegexMatcher();
    std::vector<icu::UnicodeString> results;
    const icu::UnicodeString _name;
    bool _debug;
  };

  class UniFilter {
    friend std::ostream& operator<<( std::ostream&, const UniFilter& );
  public:
    UniFilter();
    ~UniFilter();
    bool init( const icu::UnicodeString&, const icu::UnicodeString& );
    bool is_initialized() const { return _trans != 0; };
    bool fill( const std::string&, const std::string& = "" );
    bool add( const std::string& );
    bool add( const icu::UnicodeString& );
    icu::UnicodeString filter( const icu::UnicodeString& );
    icu::UnicodeString get_rules() const;
  private:
    icu::Transliterator *_trans;
  };

  icu::UnicodeString filter_diacritics( const icu::UnicodeString& );

  std::vector<icu::UnicodeString> split_at( const icu::UnicodeString&,
					    const icu::UnicodeString&,
					    size_t = 0 );

  std::vector<icu::UnicodeString> split_at_first_of( const icu::UnicodeString&,
						     const icu::UnicodeString&,
						     size_t = 0 );

  std::vector<icu::UnicodeString> split( const icu::UnicodeString&,
					 size_t = 0 );

  std::string utf8_lowercase( const std::string& ); // Unicode safe version
  std::string utf8_uppercase( const std::string& ); // Unicode safe version

}
#endif // TICC_UNICODE_H
