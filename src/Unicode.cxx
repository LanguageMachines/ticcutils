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
#include <iostream>
#include <fstream>
#include "unicode/normalizer2.h"
#include "unicode/ustream.h"
#include "ticcutils/StringOps.h"

using namespace std;

namespace TiCC {
  using namespace icu;

  UnicodeString UnicodeFromEnc( const string& s, const string& enc ){
    return UnicodeString( s.c_str(), s.length(), enc.c_str() );
  }

  string UnicodeToUTF8( const UnicodeString& s ){
    string result;
    s.toUTF8String(result);
    return result;
  }

  UnicodeNormalizer::UnicodeNormalizer( const string& enc ): _normalizer(0) {
    string mode = enc;
    if ( mode.empty() ){
      mode = "NFC";
    }
    setMode(mode);
  }

  UnicodeNormalizer::~UnicodeNormalizer(){
    // NEVER EVER delete _normalizer!
  }

  const string UnicodeNormalizer::setMode( const string& enc ){
    if ( enc == mode
	 || (enc.empty() && mode == "NFC") ){
      return mode;
    }
    else {
      // NEVER EVER delete _normalizer! it is static
      UErrorCode err = U_ZERO_ERROR;
      if ( enc == ""
	   || enc == "NFC" )
	_normalizer = Normalizer2::getNFCInstance( err );
      else if ( enc == "NONE" )
	_normalizer = 0;
      else if ( enc == "NFD" )
	_normalizer = Normalizer2::getNFDInstance( err );
      else if ( enc == "NFKC" )
	_normalizer = Normalizer2::getNFKCInstance( err );
      else if ( enc == "NFKD" )
	_normalizer = Normalizer2::getNFKDInstance( err );
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

  class uRegexError: public invalid_argument {
  public:
    explicit uRegexError( const string& s ): invalid_argument( "Invalid regular expression: " + s ){};
    explicit uRegexError( const UnicodeString& us ): invalid_argument( "Invalid regular expression: " + UnicodeToUTF8(us) ){};
  };


  UnicodeString UnicodeRegexMatcher::Pattern() const{
    return pattern->pattern();
  }

  UnicodeRegexMatcher::UnicodeRegexMatcher( const UnicodeString& pat,
					    const UnicodeString& name ):
    _name(name), _debug(false)
  {
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
    delete pattern;
    delete matcher;
  }

  bool UnicodeRegexMatcher::match_all( const UnicodeString& line,
				       UnicodeString& pre,
				       UnicodeString& post ){
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
	    if (!U_FAILURE(u_stat)){
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
	    if (!U_FAILURE(u_stat)){
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
    if ( n < results.size() )
      return results[n];
    else
      return "";
  }

  int UnicodeRegexMatcher::NumOfMatches() const {
    if ( results.size() > 0 )
      return results.size()-1;
    else
      return 0;
  }

  int UnicodeRegexMatcher::split( const UnicodeString& us,
				  vector<UnicodeString>& result ){
    result.clear();
    const int maxWords = 256;
    UnicodeString words[maxWords];
    UErrorCode status = U_ZERO_ERROR;
    int numWords = matcher->split( us, words, maxWords, status );
    for ( int i = 0; i < numWords; ++i )
      result.push_back( words[i] );
    return numWords;
  }

  UniFilter::UniFilter(): _trans(0) {}
  UniFilter::~UniFilter(){
    delete _trans;
  }

  UnicodeString UniFilter::get_rules() const {
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
	+ " at postion: " + toString(err.offset);
      throw runtime_error( msg );
    }
    return true;
  }

  UnicodeString to_icu_rule( const UnicodeString& line ){
    // a line can be an ICU Transcriptor rule " ß > sz ;"
    // OR a simple mentioning of a symbol to be replaced " ss sz" (old_style)
    // we try to covert old style to a ICU rule. (always only 1)
    bool old_style = line.indexOf( '>' ) == -1;
    if ( old_style ){
      UnicodeString result;
      bool inserted = false;
      for ( int i=0; i < line.length(); ++i ){
	if ( line[i] == '`' || line[i] == '\'' || line[i] == '"' ){
	  result += '\\';
	}
	else if ( old_style
		  && (line[i] == ' ' || line[i] == '\t' )
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
    UnicodeString uline = UnicodeFromUTF8( line );
    return add( uline );
  }

  ostream& operator<<( ostream& os, const UniFilter& uf ){
    os << uf.get_rules();
    return os;
  }

  UnicodeString filter_diacritics( const UnicodeString& in ) {
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
    // split a string into substrings, using sep as separator
    // silently skip empty entries (e.g. when two or more separators co-incide)
    // if max > 0, limit the size off the result to max,
    // leaving the remainder in the last part of the result
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

  int find_first_of( const UnicodeString& src,
		     const UnicodeString& seps,
		     int pos ){
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
    // split a string into substrings, using the characters in seps
    // as possible separators
    // silently skip empty entries (e.g. when two or more separators co-incide)
    // if max > 0, limit the size of the result to max,
    // leaving the remainder in the last part of the result
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
    static UnicodeString spaces = TiCC::UnicodeFromUTF8( " \r\t\n" );
    return split_at_first_of( src, spaces, max );
  }

  string utf8_lowercase( const string& in ){
    // Unicode safe version
    UnicodeString us = TiCC::UnicodeFromUTF8( in );
    us.toLower();
    return TiCC::UnicodeToUTF8( us );
  }

  string utf8_uppercase( const string& in ){
    // Unicode safe version
    UnicodeString us = TiCC::UnicodeFromUTF8( in );
    us.toUpper();
    return TiCC::UnicodeToUTF8( us );
  }

}
