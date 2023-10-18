/*
  Copyright (c) 2006 - 2023
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
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "unicode/parseerr.h"
#include "unicode/utrans.h"
#include "unicode/utypes.h"
#include "ticcutils/StringOps.h"

using namespace std;

namespace TiCC {
  using namespace icu;

  UnicodeString UnicodeFromEnc( const string& s,
				const string& encoding,
				const string& normalization ){
    /// convert a character buffer in some encoding to a UnicodeString
    /*!
      \param s the string to interpret as a character buffer
      \param encoding the encoding assumed for s. Default UTF8
      \param normalization the normalization to use. Default NFC
      \return a normalized UnicodeString object
    */
    UnicodeString result = UnicodeString( s.c_str(),
					  s.length(),
					  encoding.c_str() );
    UnicodeNormalizer UN( normalization);
    return UN.normalize( result );
  }

  string UnicodeToUTF8( const UnicodeString& s,
			const string& normalization ){
    /// convert a UnicodeString to a UTF-8 string
    /*!
      \param s the UnicodeString to convert
      \param normalization the normalization to use. Default NFC
      \return an UTF-8 encoded string
    */
    UnicodeNormalizer UN( normalization);
    UnicodeString normalized = UN.normalize( s );
    string result;
    normalized.toUTF8String(result);
    return result;
  }

  UnicodeString UnicodeFromUTF8( const string& s,
				 const string& normalization ){
    UnicodeNormalizer UN( normalization);
    UnicodeString result = UnicodeString::fromUTF8( s );
    return UN.normalize( result );
  }

  UnicodeNormalizer::UnicodeNormalizer( const string& enc ): _normalizer(0) {
    /// create an UnicodeNormalizer object
    /*!
      \param enc a string describing the wanted normalization.
      valid values are: NFC (the default), NFD, NFKC, NFKD
    */
    string my_enc = enc;
    if ( my_enc.empty() ){
      my_enc = "NFC";
    }
    setMode(my_enc);
  }

  UnicodeNormalizer::~UnicodeNormalizer(){
    /// destroy the UnicodeNormalizer
    // NEVER EVER delete _normalizer!
  }

  const string UnicodeNormalizer::setMode( const string& enc ){
    /// set the desired normalizer mode
    /*!
      \param enc the new mode to set
      \return the previous mode
    */
    if ( enc == mode
	 || (enc.empty() && mode == "NFC") ){
      return mode;
    }
    else {
      // NEVER EVER delete _normalizer! it is static
      UErrorCode err = U_ZERO_ERROR;
      if ( enc == ""
	   || enc == "NFC" ){
	_normalizer = Normalizer2::getNFCInstance( err );
      }
      else if ( enc == "NONE" ){
	_normalizer = 0;
      }
      else if ( enc == "NFD" ){
	_normalizer = Normalizer2::getNFDInstance( err );
      }
      else if ( enc == "NFKC" ){
	_normalizer = Normalizer2::getNFKCInstance( err );
      }
      else if ( enc == "NFKD" ){
	_normalizer = Normalizer2::getNFKDInstance( err );
      }
      else {
	throw logic_error( "invalid normalization mode: " + enc );
      }
      string tmp = mode;
      mode = enc;
      if ( mode.empty() ){
	mode = "NFC";
      }
      return tmp;
    }
  }

  UnicodeString UnicodeNormalizer::normalize( const UnicodeString& us ){
    /// normalize a UnicodeString to the current mode
    /*!
      \param us the UnicodeString to normalize
      \return the UnicodeString in the correct normalization
    */
    if ( _normalizer == 0 ){
      return us;
    }
    else {
      UErrorCode status=U_ZERO_ERROR;
      UnicodeString r = _normalizer->normalize( us, status );
      if (U_FAILURE(status)){
	throw invalid_argument("Normalizer");
      }
      return r;
    }
  }

  /// @cond HIDDEN
  class uRegexError: public invalid_argument {
  public:
    explicit uRegexError( const string& s ): invalid_argument( "Invalid regular expression: " + s ){};
    explicit uRegexError( const UnicodeString& us ): invalid_argument( "Invalid regular expression: " + UnicodeToUTF8(us) ){};
  };
  /// @endcond

  UnicodeString UnicodeRegexMatcher::Pattern() const {
    /// return the current Regex pattern
    return pattern->pattern();
  }

  UnicodeRegexMatcher::UnicodeRegexMatcher( const UnicodeString& pat,
					    const UnicodeString& name ):
    _name(name), _debug(false)
  {
    /// create a RegexMatcher object
    /*!
      \param pat The pattern to use
      \param name a name we give to this RegexMatcher (for error messages)
    */
    matcher = NULL;
    UErrorCode u_stat = U_ZERO_ERROR;
    UParseError errorInfo;
    pattern = RegexPattern::compile( pat, 0, errorInfo, u_stat );
    if ( U_FAILURE(u_stat) ){
      string spat = UnicodeToUTF8(pat);
      string failString = UnicodeToUTF8(_name);
      if ( errorInfo.offset >0 ){
	failString += " at position " + TiCC::toString( errorInfo.offset ) + "\n";
	UnicodeString pat1 = UnicodeString( pat, 0, errorInfo.offset -1 );
	failString += UnicodeToUTF8(pat1) + " <== HERE\n";
      }
      else {
	failString += "'" + spat + "' ";
      }
      throw uRegexError(failString);
    }
    else {
      matcher = pattern->matcher( u_stat );
      if (U_FAILURE(u_stat)){
	string failString = "'" + UnicodeToUTF8(pat) + "'";
	throw uRegexError(failString);
      }
    }
  }

  UnicodeRegexMatcher::~UnicodeRegexMatcher(){
    /// destroy a RegexMatcher
    delete pattern;
    delete matcher;
  }

  bool UnicodeRegexMatcher::match_all( const UnicodeString& line,
				       UnicodeString& pre,
				       UnicodeString& post ){
    /// apply the RegexMatcher on an Unicode line
    /*!
      \param line the UnicodeString to analyze
      \param pre the part of the line BEFORE the match, may be ""
      \param post the part of the line AFTER the match, may be ""
      \return true when there was some match found

      if match_all returns true, you need to call get_match() to get results
    */
    UErrorCode u_stat = U_ZERO_ERROR;
    pre = "";
    post = "";
    results.clear();
    if ( matcher ){
      if ( _debug ){
	cerr << "start matcher [" << line << "], pattern = " << Pattern() << endl;
      }
      matcher->reset( line );
      if ( matcher->find() ){
	if ( _debug ){
	  cerr << "matched " << line << endl;
	  for ( int i=0; i <= matcher->groupCount(); ++i ){
	    cerr << "group[" << i << "] =" << matcher->group(i,u_stat) << endl;
	  }
	}
	if ( matcher->groupCount() == 0 ){
	  // case 1: a rule without capture groups matches
	  UnicodeString us = matcher->group(0,u_stat) ;
	  if ( _debug ){
	    cerr << "case 1, result = " << us << endl;
	  }
	  results.push_back( us );
	  int start = matcher->start( 0, u_stat );
	  if ( start > 0 ){
	    pre = UnicodeString( line, 0, start );
	    if ( _debug ){
	      cerr << "found pre " << pre << endl;
	    }
	  }
	  int end = matcher->end( 0, u_stat );
	  if ( end < line.length() ){
	    post = UnicodeString( line, end );
	    if ( _debug ){
	      cerr << "found post " << post << endl;
	    }
	  }
	  return true;
	}
	else if ( matcher->groupCount() == 1 ){
	  // case 2: a rule with one capture group matches
	  int start = matcher->start( 1, u_stat );
	  if ( start >= 0 ){
	    UnicodeString us = matcher->group(1,u_stat) ;
	    if ( _debug ){
	      cerr << "case 2a , result = " << us << endl;
	    }
	    results.push_back( us );
	    if ( start > 0 ){
	      pre = UnicodeString( line, 0, start );
	      if ( _debug ){
		cerr << "found pre " << pre << endl;
	      }
	    }
	    int end = matcher->end( 1, u_stat );
	    if ( end < line.length() ){
	      post = UnicodeString( line, end );
	      if ( _debug ){
		cerr << "found post " << post << endl;
	      }
	    }
	  }
	  else {
	    // group 1 is empty, return group 0
	    UnicodeString us = matcher->group(0,u_stat) ;
	    if ( _debug ){
	      cerr << "case 2b , result = " << us << endl;
	    }
	    results.push_back( us );
	    start = matcher->start( 0, u_stat );
	    if ( start > 0 ){
	      pre = UnicodeString( line, 0, start );
	      if ( _debug ){
		cerr << "found pre " << pre << endl;
	      }
	    }
	    int end = matcher->end( 0, u_stat );
	    if ( end < line.length() ){
	      post = UnicodeString( line, end );
	      if ( _debug ){
		cerr << "found post " << post << endl;
	      }
	    }
	  }
	  return true;
	}
	else {
	  // a rule with more then 1 capture group
	  // this is quite ugly...
	  int end = 0;
	  for ( int i=0; i <= matcher->groupCount(); ++i ){
	    if ( _debug ){
	      cerr << "group " << i << endl;
	    }
	    u_stat = U_ZERO_ERROR;
	    int start = matcher->start( i, u_stat );
	    if ( _debug ){
	      cerr << "start = " << start << endl;
	    }
	    if ( !U_FAILURE(u_stat) ){
	      if ( start < 0 ){
		continue;
	      }
	    }
	    else
	      break;
	    if ( start > end ){
	      pre = UnicodeString( line, end, start );
	      if ( _debug ){
		cerr << "found pre " << pre << endl;
	      }
	    }
	    end = matcher->end( i, u_stat );
	    if ( _debug ){
	      cerr << "end = " << end << endl;
	    }
	    if ( !U_FAILURE(u_stat) ){
	      results.push_back( UnicodeString( line, start, end - start ) );
	      if ( _debug ){
		cerr << "added result " << results.back() << endl;
	      }
	    }
	    else
	      break;
	  }
	  if ( end < line.length() ){
	    post = UnicodeString( line, end );
	    if ( _debug ){
	      cerr << "found post " << post << endl;
	    }
	  }
	  return true;
	}
      }
    }
    results.clear();
    return false;
  }

  const UnicodeString UnicodeRegexMatcher::get_match( unsigned int n ) const{
    /// get one match from the RegexMatcher
    /*!
      \param n the index of the match
      \return the match result as a UnicodeString. Returns "" when n is out
      of range.
    */
    if ( n < results.size() ){
      return results[n];
    }
    return "";
  }

  int UnicodeRegexMatcher::NumOfMatches() const {
    /// give the number of matches found.
    if ( results.size() > 0 ){
      return results.size()-1;
    }
    return 0;
  }

  int UnicodeRegexMatcher::split( const UnicodeString& us,
				  vector<UnicodeString>& result ){
    /// split a UnicodeString using the stored pattern
    /*!
      \param us the UnicodeString to split
      \param result a vector with the splitted parts
      \return the number os elements in the result
    */
    result.clear();
    const int maxWords = 256;
    UnicodeString words[maxWords];
    UErrorCode status = U_ZERO_ERROR;
    int numWords = matcher->split( us, words, maxWords, status );
    for ( int i = 0; i < numWords; ++i ){
      result.push_back( words[i] );
    }
    return numWords;
  }

  UniFilter::UniFilter(): _trans(0) {
    /// create a Unicode Filter object
  }
  UniFilter::~UniFilter(){
    /// destroy a Unicode Filter object
    delete _trans;
  }

  UnicodeString UniFilter::get_rules() const {
    /// extract the current rules from the Unicode Filter
    UnicodeString result;
    if ( !_trans ){
      throw runtime_error( "UniFilter::getRules(), filter not initialized." );
    }
    else {
      return _trans->toRules( result, true );
    }
  }

  bool UniFilter::init( const UnicodeString& rules,
			const UnicodeString& name ){
    /// initialize a Unicode Filter
    /*!
      \param rules a Unicode string with filter rules
      \param name a name for the filter (used for error messages)
      \return true on succes, will throw on error
    */
    if ( _trans ){
      throw logic_error( "UniFilter::init():, filter already initialized." );
    }
    UErrorCode stat = U_ZERO_ERROR;
    UParseError err;
    _trans = Transliterator::createFromRules( name,
						   rules,
						   UTRANS_FORWARD,
						   err,
						   stat );
    if ( U_FAILURE( stat ) ){
      string msg = "creating UniFilter: " + UnicodeToUTF8( name )
	+ " failed\n" + "error in rules, line=" + toString(err.line)
	+ " at position: " + toString(err.offset);
      throw runtime_error( msg );
    }
    return true;
  }

  UnicodeString to_icu_rule( const UnicodeString& line ){
    /// convert an ICU Transcriptor rule or a trivial replacement into
    /// an ICU rule
    /*!
      \param line the rule to convert
      \return an Unicode representation of the rule

      A rule can be an ICU Transcriptor rule like " ß > sz ;"
      OR a simple mentioning of a symbol to be replaced " ss sz" (old_style)

      The old_style variants are converted to a ICU rule. (always only just 1)

      otherwise the input is just returned 'as is'
    */
    bool old_style = line.indexOf( '>' ) == -1;
    if ( old_style ){
      UnicodeString result;
      bool inserted = false;
      for ( int i=0; i < line.length(); ++i ){
	if ( line[i] == '`' || line[i] == '\'' || line[i] == '"' ){
	  result += '\\';
	}
	else if ( (line[i] == ' ' || line[i] == '\t' )
		  && !inserted ){
	  // OLD style: replace first space by a '>' symbol.
	  inserted = true;
	  result += " >";
	}
	result += line[i];
      }
      if ( !inserted ){
	// special case. line was only something like "\u00A0" or "-"
	// which means: delete (replace by nothing)
	result  += " >";
      }
      result += " ;";
      return result;
    }
    else {
      return line;
    }
  }

  bool UniFilter::fill( const string& filename,
			const string& label ){
    /// fill a Unicode Filter from a file
    /*!
      \param filename the file to read
      \param label a label for the filter
      \return true on succes, will throw on erroe
    */
    ifstream is( filename );
    if ( !is ){
      throw runtime_error( "UniFilter::fill(), unable te open rules file: '"
			   + filename + "'" );
    }
    UnicodeString rule;
    string line;
    while ( getline( is, line ) ){
      UnicodeString uline = UnicodeFromUTF8( line );
      rule += to_icu_rule( uline );
    }
    return init( rule, UnicodeFromUTF8(label) );
  }

  UnicodeString UniFilter::filter( const UnicodeString& line ){
    /// apply the Unicode Filter on a Unicode line
    /*!
      \param line the inputline
      \return the resulting filtered line
    */
    if ( !_trans ){
      //      throw logic_error( "UniFilter not initialized." );
      return line;
    }
    else {
      UnicodeString result = line;
      _trans->transliterate( result );
      return result;
    }
  }

  bool UniFilter::add( const UnicodeString& in ){
    /// add an extra rule to the Unicode Filter
    /*!
      \param in a rule to add
    */
    //
    // TODO: cache multiple add's and only (re-)init the transliterator
    //       once. On first use of the filter() method.
    //       caveat: Warnings about problems will be postponed too
    //
    UnicodeString uline = to_icu_rule( in );
    UnicodeString old_rules;
    UnicodeString id = "generatedId";
    if ( _trans ){
      _trans->toRules( old_rules, false );
      id = _trans->getID();
      delete _trans;
      _trans = 0;
    }
    // cerr << "OLD rule: " << old_rules << endl;
    // cerr << "add rule: " << uline << endl;
    old_rules += uline;
    // cerr << "NEW rule: " << old_rules << endl;
    // cerr << "ID = " << id << endl;
    return init( old_rules, id );
  }

  bool UniFilter::add( const string& line ){
    /// add an extra rule to the Unicode Filter
    /*!
      \param line a UTF-8 encoded rule
    */
    UnicodeString uline = UnicodeFromUTF8( line );
    return add( uline );
  }

  ostream& operator<<( ostream& os, const UniFilter& uf ){
    /// output the current Rules to a stream
    os << uf.get_rules();
    return os;
  }

  UnicodeString filter_diacritics( const UnicodeString& in ) {
    /// filter ALL diacritics from an UnicodeString
    /*!
      \param in the UnicodeString to filter from
      \return an UnicodeString with all diacrytics removed
    */
    static Transliterator *trans = 0;
    if ( trans == 0 ){
      UErrorCode stat = U_ZERO_ERROR;
      trans = Transliterator::createInstance( "NFD; [:M:] Remove; NFC",
						   UTRANS_FORWARD,
						   stat );
      if ( U_FAILURE( stat ) ){
	throw runtime_error( "filter_diacritics()  transliterator not created" );
      }
    }
    UnicodeString result = in;
    trans->transliterate( result );
    return result;
  }

  vector<UnicodeString> split_at( const UnicodeString& src,
				  const UnicodeString& sep,
				  size_t max ){
    /// split an UnicodeString
    /*!
      \param src the UnicodeString to split
      \param sep the separator to split at
      \param max limit the size off the result to max, when max > 0
      leaving the remainder in the last part of the result
      \return a vector with the splitted parts

      \note this function skips empty entries (e.g. when two or more separators
      co-incide)
    */
    if ( sep.isEmpty() ){
      throw runtime_error( "TiCC::split_at(): separator is empty!" );
    }
    vector<UnicodeString> results;
    size_t cnt = 0;
    int pos = 0;
    while ( pos != -1 ){
      UnicodeString res;
      int p = src.indexOf( sep, pos );
      if ( p == -1 ){
	res = src.tempSubString( pos );
	pos = p;
      }
      else {
	res = src.tempSubString( pos, p - pos );
	pos = p + sep.length();
      }
      if ( !res.isEmpty() ){
	++cnt;
	results.push_back( res );
      }
      if ( max != 0 && cnt >= max-1 ){
	if ( pos != -1 ){
	  results.push_back( src.tempSubString( pos ) );
	}
	break;
      }
    }
    return results;
  }

  vector<UnicodeString> split_exact_at( const UnicodeString& src,
					const UnicodeString& sep ){
    /// split an UnicodeString
    /*!
      \param src the UnicodeString to split
      \param sep the separator string to split at
      \return a vector with the splitted parts

      \note this function creates empty entries (e.g. when two or more
      separators co-incide)
    */
    if ( sep.isEmpty() ){
      throw runtime_error( "TiCC::split_at(): separator is empty!" );
    }
    vector<UnicodeString> results;
    int pos = 0;
    while ( pos != -1 ){
      UnicodeString res;
      int p = src.indexOf( sep, pos );
      if ( p == -1 ){
	res = src.tempSubString( pos );
	pos = p;
      }
      else {
	res = src.tempSubString( pos, p - pos );
	pos = p + sep.length();
      }
      results.push_back( res );
    }
    return results;
  }

  int find_first_of( const UnicodeString& src,
		     const UnicodeString& seps,
		     int pos ){
    /// find the first occurrence of one of the seps in a string
    /*!
      \param src the string to search
      \param seps a list of separator characters
      \param pos start position for the search
      \return the position found, or -1 when not present
    */
    int result = src.length()+10;
    for ( int i=0; i < seps.length(); ++i ){
      int p = src.indexOf( seps[i], pos );
      if ( p >= 0 ){
	result = min( p, result );
      }
    }
    if ( result >= 0 && result < src.length() ){
      return result;
    }
    return -1;
  }

  vector<UnicodeString> split_at_first_of( const UnicodeString& src,
					   const UnicodeString& seps,
					   size_t max ){
    /// split an UnicodeString
    /*!
      \param src the UnicodeString to split
      \param seps a list of separator characters
      \param max limit the size off the result to max, when max > 0
      leaving the remainder in the last part of the result
      \return a vector with the splitted parts

      \note this function skips empty entries (e.g. when two or more separators
      co-incide)
    */
    if ( seps.isEmpty() ){
      throw runtime_error( "TiCC::split_at_first_of(): separators are empty!" );
    }
    vector<UnicodeString> results;
    size_t cnt = 0;
    int pos = 0;
    while ( pos != -1 ){
      UnicodeString res;
      int e = find_first_of( src, seps, pos );
      if ( e == -1 ){
	res = src.tempSubString( pos );
	pos = e;
      }
      else {
	res = src.tempSubString( pos, e - pos );
	pos = e+1;
      }
      if ( !res.isEmpty() ){
	results.push_back( res );
	++cnt;
      }
      if ( max != 0 && cnt >= max-1 ){
	if ( pos != -1 ){
	  results.push_back( src.tempSubString( pos ) );
	}
	break;
      }
    }
    return results;
  }

  vector<UnicodeString> split( const UnicodeString& src,
			       size_t max ){
    /// split an UnicodeString at whitespace
    /*!
      \param src the UnicodeString to split
      \param max limit the size off the result to max, when max > 0
      leaving the remainder in the last part of the result
      \return a vector with the splitted parts

      \note this function skips empty entries (e.g. when two or more separators
      co-incide)
    */
    static UnicodeString spaces = TiCC::UnicodeFromUTF8( " \r\t\n" );
    return split_at_first_of( src, spaces, max );
  }

  vector<UnicodeString> split_exact_at_first_of( const UnicodeString& src,
						 const UnicodeString& seps ){
    /// split an UnicodeString
    /*!
      \param src the UnicodeString to split
      \param seps a list of separator characters
      \return a vector with the splitted parts

      \note this function may create empty entries (e.g. when two or more
      separators co-incide)
    */
    if ( seps.isEmpty() ){
      throw runtime_error( "TiCC::split_at_first_of(): separators are empty!" );
    }
    vector<UnicodeString> results;
    int pos = 0;
    while ( pos != -1 ){
      UnicodeString res;
      int e = find_first_of( src, seps, pos );
      if ( e == -1 ){
	res = src.tempSubString( pos );
	pos = e;
      }
      else {
	res = src.tempSubString( pos, e - pos );
	pos = e+1;
      }
      results.push_back( res ); // even when empty
    }
    return results;
  }

  string utf8_lowercase( const string& in ){
    /// Unicode aware conversion of an UTF-8 string to lowercase
    UnicodeString us = TiCC::UnicodeFromUTF8( in );
    us.toLower();
    return TiCC::UnicodeToUTF8( us );
  }

  string utf8_uppercase( const string& in ){
    /// Unicode aware conversion of an UTF-8 string to uppercase
    UnicodeString us = TiCC::UnicodeFromUTF8( in );
    us.toUpper();
    return TiCC::UnicodeToUTF8( us );
  }

  UnicodeString utrim( const UnicodeString& us,
		       const UnicodeString& filter ){
    UnicodeString result;
    int start_p = 0;
    for ( int i=0; i < us.length(); ++i ){
      int pos = filter.indexOf( us[i] );
      if ( pos < 0 ){
	// not found
	start_p = i;
	break;
      }
    }
    int end_p = us.length()-1;
    for ( int i = end_p; i > start_p; --i ){
      int pos = filter.indexOf( us[i] );
      if ( pos < 0 ){
	// not found
	end_p = i;
	break;
      }
    }
    return UnicodeString( us, start_p, end_p - start_p + 1 );
  }

  UnicodeString ltrim( const UnicodeString& us,
		       const UnicodeString& filter ){
    UnicodeString result;
    int start_p = 0;
    for ( int i=0; i < us.length(); ++i ){
      int pos = filter.indexOf( us[i] );
      if ( pos < 0 ){
	// not found
	start_p = i;
	break;
      }
    }
    return UnicodeString( us, start_p, us.length() - start_p + 1 );
  }

  UnicodeString rtrim( const UnicodeString& us,
		       const UnicodeString& filter ){
    UnicodeString result;
    int start_p = 0;
    int end_p = us.length()-1;
    for ( int i = end_p; i > start_p; --i ){
      int pos = filter.indexOf( us[i] );
      if ( pos < 0 ){
	// not found, done
	end_p = i;
	break;
      }
    }
    return UnicodeString( us, start_p, end_p - start_p + 1 );
  }

  UnicodeString pad( const UnicodeString& in,
		     int len,
		     const UChar32 pad_char ){
    if ( len <= in.length() ){
      return in;
    }
    else {
      size_t to_add = len - in.length();
      UnicodeString out = UnicodeString( to_add, pad_char, to_add ) + in;
      return out;
    }
  }

  istream& getline( istream& is,
		    UnicodeString& us,
		    const char delim ){
    /// read a UnicodeString from an encoded file
    /*!
      \param is The stream to read from
      \param us the UnicodeString to read. (will be cleared before reading)
      the string is normalized in NFC.
      \param delim The delimiter. Default '\n'
      \return the stream
    */
    return getline( is, us, "UTF8", delim );
  }

  istream& getline( istream& is,
		    UnicodeString& us,
		    const string& encoding,
		    const char delim ){
    /// read a UnicodeString from an encoded file
    /*!
      \param is The stream to read from
      \param us the UnicodeString to read. (will be cleared before reading)
      the string is normalized in NFC.
      \param encoding The Unicode encoding of the input stream. It is up to the
      caller to assure this encoding is valid.
      \param delim The delimiter. Default '\n'
      \return the stream
    */
    string line;
    std::getline( is, line, delim );
    us = TiCC::UnicodeFromEnc( line, encoding, "NFC" );
    return is;
  }

  UnicodeString format_non_printable( const UnicodeString& in ){
    UnicodeString result;
    for ( int n=0; n < in.length(); ++n ){
      if ( u_isprint( in[n] ) ){
	result += in[n];
      }
      else {
	string tmp = TiCC::format_non_printable( TiCC::UnicodeToUTF8(UnicodeString(in[n])) );
	result += TiCC::UnicodeFromUTF8( tmp );
      }
    }
    return result;
  }

}
