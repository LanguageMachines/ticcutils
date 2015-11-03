/*
  $Id: CommandLine.cxx 16820 2014-01-06 10:12:30Z sloot $
  $URL: https://ilk.uvt.nl/svn/trunk/sources/Timbl6/src/CommandLine.cxx $

  Copyright (c) 1998 - 2015
  ILK   - Tilburg University
  CLiPS - University of Antwerp

  This file is part of timbl

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

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <iostream>

#include "ticcutils/StringOps.h"
#include "ticcutils/CommandLine.h"
#include "ticcutils/PrettyPrint.h"

using namespace std;

namespace TiCC {

  CL_Options::CL_Options( const string& short_o, const string& long_o ){
    is_init  = false;
    debug = false;
    set_short_options( short_o );
    set_long_options( long_o );
  }

  CL_Options::CL_Options( const int argc, const char * const *argv,
			  const string& valid_s, const string& valid_l ){
    is_init  = false;
    debug = false;
    set_short_options( valid_s );
    set_long_options( valid_l );
    init( argc, argv );
  }

  CL_Options::~CL_Options(){
  }

  bool CL_Options::init( const int argc, const char * const *argv ){
    if ( is_init ){
      throw OptionError( "cannot init() an options object twice" );
    }
    if ( Parse_Command_Line( argc, argv ) )
      is_init = true;
    return is_init;
  }

  bool CL_Options::init( const std::string& args ){
    if ( is_init ){
      throw OptionError( "cannot init() an options object twice" );
    }
    const char *argstr = args.c_str();
    if ( Parse_Command_Line( 0, &argstr ) )
      is_init = true;
    return is_init;
  }

  ostream& operator<<( ostream& os, const CL_item& it ){
    os << it.toString();
    return os;
  }

  string CL_item::toString( ) const {
    string result;
    if ( longOpt ){
      result = "--" + opt_word;
      if ( !option.empty() ){
	result += "=";
      }
      result += option;
    }
    else
      result += (mood ? "+": "-" ) + opt_word + option;
    return result;
  }

  ostream& operator<<( ostream& os, const CL_Options& cl ){
    os << cl.toString() << " ";
    for ( size_t i=0; i < cl.MassOpts.size(); ++i ){
      os << cl.MassOpts[i] << " ";
    }
    return os;
  }

  string CL_Options::toString() const {
    string result;
    for( const auto& pos : Opts ){
      result += pos.toString() + " ";
    }
    if ( !result.empty() )
      result.erase(result.length()-1);
    return result;
  }

  ostream& CL_Options::dump( ostream& os ) {
    os << *this;
    if ( !valid_chars.empty() ){
      os << endl;
      os << "Valid short options: " << get_short_options();
    }
    if ( !valid_long.empty() ){
      os << endl;
      os << "Valid long options: " << get_long_options();
    }
    return os;
  }

  bool CL_Options::is_present_internal( const char c, string &opt, bool& mood ) const {
    mood = false;
    for ( auto const& pos : Opts ){
      if ( pos.isLong() )
	continue;
      if ( pos.OptChar() == c ){
	opt = pos.Option();
	mood = pos.Mood();
	if ( debug ){
	  cerr << "is_present '" << c << "' ==> '" << opt << "'" << endl;
	}
	return true;
      }
    }
    if ( debug ){
      cerr << "is_present '" << c << "' FAILS " << endl;
    }
    return false;
  }

  bool CL_Options::is_present_internal( const string& w, string &opt ) const {
    for ( const auto& pos : Opts ){
      if ( pos.OptWord() == w ){
	opt = pos.Option();
	if ( debug ){
	  cerr << "is_present '" << w << "' ==> '" << opt << "'" << endl;
	}
	return true;
      }
    }
    if ( debug ){
      cerr << "is_present '" << w << "' FAILS " << endl;
    }
    return false;
  }

  bool CL_Options::extract_internal( const char c, string &opt, bool& mood ) {
    mood = false;
    for ( auto pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( !pos->isLong() ){
	if ( pos->OptChar() == c ){
	  opt = pos->Option();
	  mood = pos->Mood();
	  Opts.erase(pos);
	  if ( debug ){
	    cerr << "extract '" << c << "' ==> '" << opt << "'" << endl;
	  }
	  return true;
	}
      }
    }
    if ( debug ){
      cerr << "extract '" << c << "' FAILS " << endl;
    }
    return false;
  }

  bool CL_Options::extract_internal( const string& w, string &opt ) {
    for ( auto pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( pos->OptWord() == w ){
	opt = pos->Option();
	Opts.erase(pos);
	if ( debug ){
	  cerr << "extract '" << w << "' ==> '" << opt << "'" << endl;
	}
	return true;
      }
    }
    if ( debug ){
      cerr << "extract '" << w << "' FAILS " << endl;
    }
    return false;
  }

  bool CL_Options::remove( const char c, bool all ){
    for ( auto pos = Opts.begin(); pos != Opts.end(); ){
      if ( pos->OptChar() == c ){
	pos = Opts.erase(pos);
	if ( !all )
	  return true;
      }
      else {
	++pos;
      }
    }
    return false;
  }

  bool CL_Options::remove( const string& w, bool all ){
    for ( auto pos = Opts.begin(); pos != Opts.end(); ){
      if ( pos->OptWord() == w ){
	pos = Opts.erase(pos);
	if ( !all )
	  return true;
      }
      else {
	++pos;
      }
    }
    return false;
  }

  void CL_Options::insert( const string& s, const string& line ){
    CL_item cl( s, line );
    Opts.push_back( cl );
  }

  void CL_Options::insert( const char c, const string& line, bool mood ){
    CL_item cl( c, line, mood );
    Opts.push_back( cl );
  }

  enum argstat { PLUS, MIN, LONG, MASS, UNKNOWN };
  struct arg {
    arg(): stat(UNKNOWN),c(0){};
    argstat stat;
    char c;
    string s;
    string val;
  };

  ostream& operator<<( ostream& os, const arg& a ){
    switch ( a.stat ){
    case UNKNOWN:
      os << "?";
      os << a.s;
      os << "=" << a.val;
      break;
    case MASS:
      os << "mass:" + a.val;
      break;
    case PLUS:
      os << "+";
      os << a.c;
      os << a.s;
      os << "=" << a.val;
      break;
    case MIN:
      os << "-";
      os << a.c;
      os << a.s;
      os << "=" << a.val;
      break;
    case LONG:
      os << "--";
      os << a.s;
      os << "=" << a.val;
      break;
    }
    return os;
  }


  bool CL_Options::Parse_Command_Line( const int Argc,
				       const char * const *Argv ){
    Opts.clear();
    vector<string> local_argv;
    if ( Argc == 0 )
      if ( Argv != 0 &&
	   Argv[0] != 0 ){
	split( Argv[0], local_argv );
      }
      else
	return false;
    else {
      _prog_name = basename(Argv[0]);
      for( int i=1; i < Argc; ++i ){
	// start at 1 to skip the program name
	local_argv.push_back( Argv[i] );
      }
    }
    if ( debug ){
      cerr << "Parse_CommandLine: Option vector:  " << local_argv << endl;
    }
    vector<string> cleaned;
    for ( size_t i=0; i < local_argv.size(); ++i ){
      string Option = local_argv[i];
      if ( debug ){
	cerr << "examine potential option = " << Option << endl;
      }
      if ( Option.size() == 1 ){
	cleaned.push_back( Option );
	continue;
      }
      char first = Option[0];
      switch ( first ){
      case '+':
      case '-':
	if ( (Option.size() <= 2 || Option[1] == '-' )
	     && Option.find("=") == string::npos ) {
	  if ( i < local_argv.size()-1 ){
	    string Option2 = local_argv[i+1];
	    if ( debug ){
	      cerr << "examine potential extra option: " << Option2 << endl;
	    }
	    if ( Option2[0] != '+' && Option2[0] != '-' ){
	      if ( Option[1] == '-' ){
		Option += "=" + Option2;
	      }
	      else {
		Option += Option2;
	      }
	      ++i;
	    }
	  }
	}
	cleaned.push_back( Option );
	break;
      case '=':
	throw OptionError( "stray '='. (maybe it belongs to an long option?)" );
      default:
	cleaned.push_back( Option );
      }
    }
    if ( debug ){
      cerr << "Cleaned vector: " << cleaned << endl;
    }
    vector<arg> arguments;
    for ( size_t i=0; i < cleaned.size(); ++i ){
      string Option = cleaned[i];
      char first = Option[0];
      arg argument;
      switch ( first ){
      case '+':
	argument.stat = PLUS;
	argument.c = Option[1];
	argument.val = Option.substr(2);
	arguments.push_back(argument);
	break;
      case '-':
	if ( Option[1] == '-' ){
	  if ( Option.size() > 2 ){
	    string Optword;
	    string OptValue;
	    string::size_type pos = Option.find( "=" );
	    if ( pos == string::npos ) {
	      Optword = Option.substr( 2 );
	    }
	    else {
	      Optword = Option.substr( 2, pos-2 );
	      OptValue = Option.substr( pos+1 );
	    }
	    argument.stat = LONG;
	    argument.s = Optword;
	    argument.val = OptValue;
	    arguments.push_back(argument);
	  }
	  else {
	    // special: '--'
	    argument.stat = MIN;
	    argument.c = '-';
	    arguments.push_back(argument);
	  }
	}
	else {
	  argument.stat = MIN;
	  argument.c = Option[1];
	  argument.val =  Option.substr(2);
	  arguments.push_back(argument);
	}
	break;
      default:
	argument.stat = MASS;
	if ( debug ){
	  cerr << "OPTION=" << Option << endl;
	}
	argument.val = Option;
	arguments.push_back(argument);
	break;
      }
    }

    if ( debug ){
      cerr << "ARGUMENTS list: " << arguments << endl;
    }

    // are there some options to check?
    bool doCheck = !valid_long.empty() || !valid_chars.empty();
    if ( doCheck ){
      if ( debug ){
	cerr << "we must check those " << endl;
      }
      auto it = arguments.begin();
      while ( it != arguments.end() ){
	if ( debug ){
	  cerr << "ARGUMENT: " << *it << endl;
	}
	if ( it->stat == LONG ){
	  if ( valid_long.find( it->s ) == valid_long.end() ){
	    throw OptionError( "invalid option '" + it->s + "'" );
	  }
	  bool has_par = valid_long_par.find( it->s ) != valid_long_par.end();
	  bool has_opt = valid_long_opt.find( it->s ) != valid_long_opt.end();
	  if ( debug ){
	    if ( has_par )
	      cerr << "\"" << it->s << "\" must have a parameter." << endl;
	    else if ( has_opt )
	      cerr << "\"" << it->s << "\" may have a parameter!" << endl;
	    else
	      cerr << "\"" << it->s << "\" doesn't take a parameter." << endl;
	  }
	  if ( it->val.empty() ){
	    if ( !has_par ){
	      if ( debug ){
		cerr << "no parameter: OK" << endl;
	      }
	      ++it;
	      continue;
	    }
	    else {
	      if ( debug ){
		cerr << "search a parameter: ";
	      }
	      auto it2 = it;
	      if ( ++it2 != arguments.end() ){
		if ( !has_opt ){
		  if ( debug ){
		    cerr << " FAILED " << endl;
		  }
		  throw OptionError( "missing value for long option: '"
				     + it->s + "'" );
		}
		else {
		  if ( debug ){
		    cerr << "OK: " << it->val << endl;
		  }
		  ++it;
		  continue;
		}
	      }
	      else if ( !has_opt ){
		if ( debug ){
		  cerr << " OHO FAILED " << endl;
		}
		throw OptionError( "missing value for long option: '"
				   + it->s + "'" );
		++it;
		continue;
	      }
	    }
	  }
	  else if ( has_par || has_opt){
	    if ( debug ){
	      cerr << "found a parameter: " <<  it->val << endl;
	    }
	    ++it;
	    continue;
	  }
	  else {
	    auto next = it+1;
	    while ( next != arguments.end() && next->stat == MASS ){
	      ++next;
	    }
	    if ( next != arguments.end() ){
	      throw OptionError( string("option '") + it->s
				 + "' may not have a value. (found "
				 + it->val + ")" );
	    }
	    else {
	      MassOpts.push_back( it->val );
	      it->val.clear();
	    }
	    ++it;
	  }
	}
	else if ( it->stat == MIN || it->stat == PLUS ){
	  if ( valid_chars.find( it->c ) == valid_chars.end() ){
	    throw OptionError( string("invalid option '") + it->c + "'" );
	  }
	  bool has_par = valid_chars_par.find( it->c ) != valid_chars_par.end();
	  bool has_opt = valid_chars_opt.find( it->c ) != valid_chars_opt.end();
	  if ( debug ){
	    if ( has_par )
	      cerr << "'" << it->c << "' must have a parameter." << endl;
	    else if ( has_opt )
	      cerr << "'" << it->c << "' may have a parameter." << endl;
	    else
	      cerr << "'" << it->c << "' doesn't take a parameter." << endl;
	  }
	  if ( it->val.empty() ){
	    if ( !has_par ){
	      if ( debug ){
		cerr << "no parameter: OK" << endl;
	      }
	      ++it;
	      continue;
	    }
	    else {
	      if ( debug ){
		cerr << "search a parameter: ";
	      }
	      auto it2 = it;
	      if ( ++it2 != arguments.end() ){
		if ( !has_opt ){
		  if ( debug ){
		    cerr << " FAILED " << endl;
		  }
		  throw OptionError( string("missing value for option '")
				     + it->c + "'" );
		}
		else {
		  if ( debug ){
		    cerr << "OK: " << it->val << endl;
		  }
		  ++it;
		  continue;
		}
	      }
	      else if ( !has_opt ){
		if ( debug ){
		  cerr << " AHA FAILED " << endl;
		}
		throw OptionError( string("missing value for option ''")
				   + it->c + "'" );
	      }
	      else {
		++it;
		continue;
	      }
	    }
	  }
	  else if ( has_par || has_opt ){
	    if ( debug ){
	      cerr << "found a parameter: " <<  it->val << endl;
	    }
	    ++it;
	    continue;
	  }
	  else {
	    auto next = it+1;
	    while ( next != arguments.end() && next->stat == MASS ){
	      ++next;
	    }
	    if ( next != arguments.end() ){
	      throw OptionError( string("option '") + it->c
				 + "' may not have a value. (found "
				 + it->val + ")" );
	    }
	    else {
	      MassOpts.push_back( it->val );
	      it->val.clear();
	    }
	    ++it;
	  }
	}
	else if ( it->stat == MASS ){
	  auto next = it+1;
	  while ( next != arguments.end() && next->stat == MASS ){
	    ++next;
	  }
	  if ( next != arguments.end() ){
	    throw OptionError( "spurious value: '" + it->val +
			       "' in options" );
	  }
	  else {
	    MassOpts.push_back( it->val );
	    it = arguments.erase(it);
	  }
	}
      }
    }
    if ( debug ){
      cerr << "arguments after check: " << arguments << endl;
    }
    for ( const auto& it : arguments ){
      if ( it.stat == LONG ){
	CL_item cl( it.s, it.val );
	Opts.push_back( cl );
      }
      else if ( it.stat == PLUS || it.stat == MIN ){
	CL_item cl( it.c, it.val, (it.stat == PLUS) );
	Opts.push_back( cl );
      }
      else {
	MassOpts.push_back( it.val );
      }
    }
    if ( debug ){
      cerr << "mass opts: " << MassOpts << endl;
    }

    if ( debug ){
      cerr << "CL_options after Parse " << endl;
      dump(cerr);
      cerr << endl;
    }
    return true;
  }

  void CL_Options::set_short_options( const string& s ){
    char last = '\0';
    for ( size_t i=0; i < s.size(); ++i ){
      if ( s[i] == ':' && last != '\0' ){
	if ( i < s.size()-1 && s[i+1] == ':' ){
	  valid_chars_opt.insert( last );
	  ++i;
	}
	else
	  valid_chars_par.insert( last );
      }
      else {
	valid_chars.insert( s[i] );
	last = s[i];
      }
    }
  }

  string CL_Options::get_short_options() const {
    string result;
    for ( auto const& it : valid_chars ){
      result += it;
      if ( valid_chars_par.find( it ) != valid_chars_par.end() )
	result += ":";
      else if ( valid_chars_opt.find( it ) != valid_chars_opt.end() )
	result += "::";
    }
    return result;
  }

  void CL_Options::set_long_options( const string& s ){
    vector<string> parts;
    TiCC::split_at( s, parts, "," );
    for ( size_t i=0; i < parts.size(); ++i ){
      string value = parts[i];
      string::size_type pos = value.find( ':' );
      if ( pos != string::npos ){
	if ( pos == value.size()-2){
	  if ( value[value.size()-1] == ':' ){
	    value = value.substr(0,value.size()-2);
	    valid_long_opt.insert( value );
	  }
	  else {
	    throw OptionError( "':' may only be present at the end of a long option ("
			       + value + ")" );
	  }
	}
	else if ( pos == value.size()-1){
	  value = value.substr(0,value.size()-1);
	  valid_long_par.insert( value );
	}
	else {
	  throw OptionError( "':' may only be present at the end of a long option ("
			     + value + ")" );
	}
      }
      valid_long.insert( value );
    }
  }

  string CL_Options::get_long_options() const {
    string result;
    for ( auto const& s : valid_long ){
      result += s;
      if ( valid_long_par.find( s ) != valid_long_par.end() )
	result += ":";
      else if ( valid_long_opt.find( s ) != valid_long_opt.end() )
	result += "::";
      result += ",";
    }
    if ( !result.empty() )
      result = result.substr(0,result.size()-1);
    return result;
  }


}
