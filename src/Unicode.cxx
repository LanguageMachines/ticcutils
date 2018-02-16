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
	throw std::logic_error( "invalid normalization mode: " + enc );
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
	throw std::invalid_argument("Normalizer");
      }
      return r;
    }
  }

  class uRegexError: public std::invalid_argument {
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

  UnicodeString UniFilter::getRules() const {
    UnicodeString result;
    if ( !_trans ){
      throw runtime_error( "UniFilter not initialized." );
    }
    else {
      return _trans->toRules( result, true );
    }
  }

  void UniFilter::init( const UnicodeString& rules,
			const UnicodeString& name ){
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
  }

  UnicodeString escape( const UnicodeString& line ){
    bool old_style = line.indexOf( '>' ) == -1;
    bool inserted = false;
    UnicodeString result;
    for ( int i=0; i < line.length(); ++i ){
      if ( line[i] == '`' || line[i] == '\'' || line[i] == '"' ){
	result += '\\';
      }
      else if ( old_style
		&& (line[i] == ' ' || line[i] == '\t' )
		&& !inserted ){
	inserted = true;
	result += " > ";
      }
      result += line[i];
    }
    if ( old_style && result.indexOf( '>' ) == -1 ){
      result  += "> ";
    }
    return result;
  }

  void UniFilter::fill( const string& filename,
			const string& label ){
    ifstream is( filename );
    if ( !is ){
      throw runtime_error( "UniFilter: unable te open rules file: '"
			   + filename + "'" );
    }
    UnicodeString rule;
    string line;
    while ( getline( is, line ) ){
      UnicodeString uline = UnicodeFromUTF8( line );
      uline = escape( uline );
      rule += uline + " ;";
    }
    init( rule, UnicodeFromUTF8(label) );
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

  void UniFilter::add( const UnicodeString& in ){
    // if ( !_trans ){
    //   throw logic_error( "UniFilter::add() not initialized" );
    // }
    UnicodeString uline = escape( in );
    uline += " ;";
    UnicodeString old_rules;
    UnicodeString id;
    if ( _trans ){
      _trans->toRules( old_rules, false );
      id = _trans->getID();
      delete _trans;
    }
    cerr << "OLD rule: " << old_rules << endl;
    cerr << "add rule: " << uline << endl;
    old_rules += uline;
    cerr << "NEW rule: " << old_rules << endl;
    cerr << "ID = " << id << endl;
    init( old_rules, id );
  }

  void UniFilter::add( const string& line ){
    UnicodeString uline = UnicodeFromUTF8( line );
    add( uline );
  }

}
