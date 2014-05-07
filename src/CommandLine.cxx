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
//#include "ticcutils/PrettyPrint.h"

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

  inline bool p_or_m( char k )
  { return ( k == '+' || k == '-' ); }

  bool CL_Options::Split_Command_Line( const int Argc,
				       const char * const *Argv ){
    Opts.clear();
    int local_argc = 0;
    vector<string> local_argv;
    char Optchar;
    string Optword;
    string Option;
    bool Mood = false;
    if ( Argc == 0 )
      if ( Argv != 0 &&
	   Argv[0] != 0 ){
	local_argc = split( Argv[0], local_argv );
      }
      else
	return false;
    else {
      local_argc = Argc-1;
      for( int i=1; i < Argc; ++i ){
	// start at 1 to skip the program name
	local_argv.push_back( Argv[i] );
      }
    }
    for ( int arg_ind=0; arg_ind < local_argc; ++arg_ind ){
      bool longOpt = false;
      Option = local_argv[arg_ind];
      if ( !p_or_m(Option[0]) ){
	Optchar = '?';
	Optword = Option;
	Mood = false;
      }
      else {
	Mood = Option[0] == '+';
	if ( Option.size() > 1 ){
	  longOpt = Option[1] == '-';
	  if ( longOpt ){
	    if ( Mood )
	      throw OptionError("invalid option: " + Option );
	    string::size_type pos = Option.find( "=" );
	    if ( pos == string::npos ){
	      Optword = Option.erase(0,2);
	      if ( valid_long_par.find( Optword ) != valid_long_par.end() ) {
		string msg = "option '" + Optword + "' misses a value, (or =?)";
		throw OptionError( msg );
	      }
	      Option = "";
	    }
	    else {
	      Optword = Option.substr( 2, pos-2 );
	      Option = Option.substr( pos+1 );
	      if ( valid_long_par.find( Optword ) == valid_long_par.end() ) {
		MassOpts.push_back( Option );
		Option = "";
	      }
	    }
	    Optchar = Optword[0];
	    if ( Option.empty() && arg_ind+1 < local_argc ) {
	      string tmpOption = local_argv[arg_ind+1];
	      if ( tmpOption[0] == '=' ) {
		throw OptionError( "no spaces allowed in long options" );
	      }
	    }
	  }
	  else {
	    Optchar = Option[1];
	    Optword = Optchar;
	    Option = Option.erase(0,2);
	  }
	}
	else {
	  Optchar = 0;
	  Optword = Option;
	  Option = Option.erase(0,1);
	}
	if ( (!Optchar || Option.empty() ) && arg_ind+1 < local_argc ) {
	  string tmpOption = local_argv[arg_ind+1];
	  if ( !p_or_m(tmpOption[0]) ){
	    Option = tmpOption;
	    ++arg_ind;
	    if ( !Optchar ){
	      Optchar = Optword[0];
	    }
	  }
	}
      }
      if ( longOpt ){
	if ( valid_long.empty() && valid_chars.empty() ){
	  CL_item cl( Optword, Option );
	  Opts.push_back( cl );
	}
	else if ( valid_long.find( Optword ) == valid_long.end() ){
	  string msg = "invalid option '" + Optword + "'";
	  throw OptionError( msg );
	}
	else if ( valid_long_par.find( Optword ) != valid_long_par.end() ){
	  if ( Option.empty() ){
	    string msg = "option '" + Optword + "' misses a value";
	    throw OptionError( msg );
	  }
	}
	else if ( !Option.empty() ){
	  MassOpts.push_back( Option );
	  Option = "";
	}
	CL_item cl( Optword, Option );
	Opts.push_back( cl );
      }
      else if (Optchar == '?' ){
	//	cerr << "insert Mass van " << Optword << " !" << endl;
	MassOpts.push_back( Optword );
      }
      else if ( valid_chars.empty() && valid_long.empty() ){
	CL_item cl( Optchar, Option, Mood );
	Opts.push_back( cl );
      }
      else if ( valid_chars.find( Optchar ) != valid_chars.end() ){
	//	cerr << "opt-char = " << Optchar << " is valid!" << endl;
	//	cerr << "Option = '" << Option << "'" << endl;
	if ( !Option.empty() ){
	  if ( valid_chars_par.find( Optchar ) != valid_chars_par.end() ){
	    //	    cerr << "opt-char = " << Optchar << " is valid PAR char" << endl;
	    CL_item cl( Optchar, Option, Mood );
	    Opts.push_back( cl );
	  }
	  else {
	    MassOpts.push_back( Option );
	    //	    cerr << "maak een Mass van " << Option << " !" << endl;
	    string no;
	    CL_item cl( Optchar, no, Mood );
	    Opts.push_back( cl );
	  }
	}
	else if ( valid_chars_par.find( Optchar ) != valid_chars_par.end() ){
	  string msg = "option '";
	  msg += Optchar;
	  msg += "' misses a value";
	  throw OptionError( msg );
	}
	else {
	  CL_item cl( Optchar, Option, Mood );
	  Opts.push_back( cl );
	}
      }
      else {
	string msg = "invalid option '";
	msg += Optchar;
	msg += "'";
	throw OptionError( msg );
      }
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
      if ( value[value.size()-1] == ':' ){
	value = value.substr(0,value.size()-1);
	valid_long_par.insert( value );
      }
      valid_long.insert( value );
    }
  }

}


