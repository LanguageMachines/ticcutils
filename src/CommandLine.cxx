/*
  $Id: CommandLine.cxx 16820 2014-01-06 10:12:30Z sloot $
  $URL: https://ilk.uvt.nl/svn/trunk/sources/Timbl6/src/CommandLine.cxx $

  Copyright (c) 1998 - 2014
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
#include <ostream>
#include <iostream>

#include "ticcutils/StringOps.h"
#include "ticcutils/CommandLine.h"
#include "ticcutils/PrettyPrint.h"

using namespace std;

namespace TiCC {

  CL_Options::CL_Options( const string& short_o, const string& long_o ){
    is_init  = false;
    set_short_options( short_o );
    set_long_options( long_o );
  }

  CL_Options::CL_Options( const int argc, const char * const *argv,
			  const string& valid_s, const string& valid_l ){
    is_init  = false;
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
    if ( Split_Command_Line( argc, argv ) )
      is_init = true;
    return is_init;
  }

  bool CL_Options::init( const std::string& args ){
    if ( is_init ){
      throw OptionError( "cannot init() an options object twice" );
    }
    const char *argstr = args.c_str();
    if ( Split_Command_Line( 0, &argstr ) )
      is_init = true;
    return is_init;
  }

  ostream& operator<<( ostream& os, const CL_item& it ){
    if ( it.longOpt ){
      os << "--" << it.opt_word;
      if ( !it.option.empty() )
	os << "=" << it.option;
    }
    else
      os << (it.mood ? "+": "-" ) << it.opt_word << it.option;
    return os;
  }

  ostream& operator<<( ostream& os, const CL_Options& cl ){
    vector<CL_item>::const_iterator pos = cl.Opts.begin();
    while ( pos != cl.Opts.end() ){
      os << *pos << " ";
      ++pos;
    }
    for ( size_t i=0; i < cl.MassOpts.size(); ++i ){
      os << cl.MassOpts[i] << " ";
    }
    if ( !cl.valid_chars.empty() ){
      os << endl;
      os << "Valid short options: ";
      set<char>::const_iterator it = cl.valid_chars.begin();
      while ( it != cl.valid_chars.end() ){
	os << *it;
	if ( cl.valid_chars_par.find( *it ) != cl.valid_chars_par.end() )
	  os << ":";
	++it;
      }
    }
    if ( !cl.valid_long.empty() ){
      os << endl;
      os << "Valid long options: ";
      set<string>::const_iterator it = cl.valid_long.begin();
      while ( it != cl.valid_long.end() ){
	os << *it;
	if ( cl.valid_long_par.find( *it ) != cl.valid_long_par.end() )
	  os << ":";
	++it;
	if ( it != cl.valid_long.end() )
	  os << ",";
      }
    }
    return os;
  }

  bool CL_Options::find( const char c, string &opt, bool& mood ) const {
    vector<CL_item>::const_iterator pos;
    for ( pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( pos->isLong() )
	continue;
      if ( pos->OptChar() == c ){
	opt = pos->Option();
	mood = pos->Mood();
	return true;
      }
    }
    return false;
  }

  bool CL_Options::find( const string& w, string &opt ) const {
    vector<CL_item>::const_iterator pos;
    for ( pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( pos->OptWord() == w ){
	opt = pos->Option();
	return true;
      }
    }
    return false;
  }

  bool CL_Options::pull( const char c, string &opt, bool& mood ) {
    if ( find( c, opt, mood ) )
      return remove( c );
    return false;
  }

  bool CL_Options::pull( const string& w, string &opt ) {
    if ( find( w, opt ) )
      return remove( w );
    return false;
  }

  bool CL_Options::remove( const char c, bool all ){
    vector<CL_item>::iterator pos;
    for ( pos = Opts.begin(); pos != Opts.end(); ){
      if ( pos->OptChar() == c ){
	pos = Opts.erase(pos);
	if ( !all )
	  return true;
      }
      ++pos;
    }
    return false;
  }

  bool CL_Options::remove( const string& w ){
    vector<CL_item>::iterator pos;
    for ( pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( pos->OptWord() == w ){
	Opts.erase(pos);
	return true;
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

  //#define DEBUG

  bool CL_Options::Split_Command_Line( const int Argc,
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
      for( int i=1; i < Argc; ++i ){
	// start at 1 to skip the program name
	local_argv.push_back( Argv[i] );
      }
    }
#ifdef DEBUG
    cerr << "Option vector:  " << local_argv << endl;
#endif
    vector<string> cleaned;
    for ( size_t i=0; i < local_argv.size(); ++i ){
      string Option = local_argv[i];
#ifdef DEBUG
      cerr << "bekijk Option = " << Option << endl;
#endif
      if ( Option.size() == 1 ){
	string msg = "stray '";
	msg += Option[0];
	msg += "'. (maybe it belongs to another option?)";
	throw OptionError( msg );
      }
      char first = Option[0];
      switch ( first ){
      case '+':
      case '-':
	if ( Option.size() <= 2 ){
	  if ( i < local_argv.size()-1 ){
	    string Option2 = local_argv[i+1];
#ifdef DEBUG
	    cerr << "bekijk Option2 = " << Option2 << endl;
#endif
	    if ( Option2[0] != '+' && Option2[0] != '-' ){
	      Option += Option2;
	      ++i;
	    }
	  }
	}
#ifdef DEBUG
	cerr << "PUSH " << Option << endl;
#endif
	cleaned.push_back( Option );
	break;
      case '=':
	throw OptionError( "stray '='. (maybe it belongs to an long option?)" );
      default:
	Option = "?" + Option;
	if ( Option.size() <= 2 ){
	  Option += local_argv[++i];
	}
	cleaned.push_back( Option );
      }
    }
#ifdef DEBUG
    cerr << "Cleaned vector: " << cleaned << endl;
#endif

    map<char,string> min_shortMap;
    map<char,string> plus_shortMap;
    map<string,string> longMap;
    set<string> extra;

    for ( size_t i=0; i < cleaned.size(); ++i ){
      char OptChar;
      string Optword;
      string OptValue;
      string Option = cleaned[i];
      char first = Option[0];
      switch ( first ){
      case '+':
	OptChar = Option[1];
	OptValue = Option.substr(2);
	plus_shortMap[OptChar] = OptValue;
	break;
      case '-':
	if ( Option[1] == '-' ){
	  if ( Option.size() > 2 ){
	    string::size_type pos = Option.find( "=" );
	    if ( pos == string::npos ) {
	      Optword = Option.substr( 2 );
	    }
	    else {
	      Optword = Option.substr( 2, pos-2 );
	      OptValue = Option.substr( pos+1 );
	    }
	    longMap[Optword] = OptValue;
	  }
	  else {
	    // special: '--'
	    OptChar = '-';
	    min_shortMap[OptChar] = OptValue;
	  }
	}
	else {
	  OptChar = Option[1];
	  OptValue = Option.substr(2);
	  min_shortMap[OptChar] = OptValue;
	}
	break;
      case '?':
	OptChar = Option[0];
	OptValue = Option.substr(1);
	extra.insert( OptValue );
	break;
      default:
	//
	break;
      }
    }


#ifdef DEBUG
    cerr << "plus:: " << plus_shortMap << endl;
    cerr << "min::  " << min_shortMap << endl;
    cerr << "long:  " << longMap << endl;
    cerr << "extra: " << extra << endl;

    cerr << "Valid chars are: " << valid_chars << endl;
    cerr << "Valid long are: " << valid_long << endl;
#endif
    // there are some options to check?
    bool doCheck = !( valid_long.empty() && valid_chars.empty() );
    if ( doCheck ){
      map<char,string>::const_iterator it=plus_shortMap.begin();
      while ( it != plus_shortMap.end() ){
	if ( valid_chars.find( it->first ) == valid_chars.end() ){
	  string msg = "illegal option '";
	  msg += it->first;
	  msg +=  "'";
	  throw OptionError( msg );
	}
	else {
	  if ( valid_chars_par.find( it->first ) == valid_chars_par.end() ){
	    if ( !it->second.empty() ){
	      string msg = "option '";
	      msg += it->first;
	      msg += "' may not have a value";
	      throw OptionError( msg );
	    }
	  }
	  // else if ( it->second.empty() ){
	  //   string msg = "option '";
	  //   msg += it->first;
	  //   msg += "' is missing a value";
	  //   throw OptionError( msg );
	  // }
	}
	CL_item cl( it->first, it->second, true );
	Opts.push_back( cl );
	++it;
      }

      it = min_shortMap.begin();
      while ( it != min_shortMap.end() ){
	if ( valid_chars.find( it->first ) == valid_chars.end() ){
	  string msg = "illegal option '";
	  msg += it->first;
	  msg +=  "'";
	  throw OptionError( msg );
	}
	else {
	  if ( valid_chars_par.find( it->first ) == valid_chars_par.end() ){
	    if ( !it->second.empty() ){
	      extra.insert( it->second );
	      CL_item cl( it->first, "", false );
	      Opts.push_back( cl );
	      ++it;
	      continue;
	    }
	  }
	  // else if ( it->second.empty() ){
	  //   string msg = "option '";
	  //   msg += it->first;
	  //   msg += "' is missing a value";
	  //   throw OptionError( msg );
	  // }
	}
	CL_item cl( it->first, it->second, false );
	Opts.push_back( cl );
	++it;
      }
      map<string,string>::const_iterator lit = longMap.begin();
      while ( lit != longMap.end() ){
	if ( valid_long.find( lit->first ) == valid_long.end() ){
	  throw OptionError( "illegal option '" + string(lit->first) + "'" );
	}
	CL_item cl( lit->first, lit->second );
	Opts.push_back( cl );
	++lit;
      }
    }

    set<string>::const_iterator sit = extra.begin();
    while( sit != extra.end() ){
      MassOpts.push_back( *sit );
      ++sit;
    }
    return true;
  }

  void CL_Options::set_short_options( const string& s ){
    char last = '\0';
    for ( size_t i=0; i < s.size(); ++i ){
      if ( s[i] == ':' && last != '\0' ){
	valid_chars_par.insert( last );
      }
      else {
	valid_chars.insert( s[i] );
	last = s[i];
      }
    }
  }

  void CL_Options::set_long_options( const string& s ){
    vector<string> parts;
    TiCC::split_at( s, parts, "," );
    for ( size_t i=0; i < parts.size(); ++i ){
      string value = parts[i];
      string::size_type pos = value.find( ':' );
      if ( pos != string::npos && pos != value.size()-1){
	throw OptionError( "':' may only be present at the end of a long option ("
			   + value + ")" );
      }
      if ( value[value.size()-1] == ':' ){
	value = value.substr(0,value.size()-1);
	valid_long_par.insert( value );
      }
      valid_long.insert( value );
    }
  }

}


