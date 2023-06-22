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

#include "ticcutils/CommandLine.h"

#include <memory>
#include <cstddef> // for size_t
#include <string>
#include <vector>
#include <algorithm> // for find_if
#include <iostream>

#include "ticcutils/StringOps.h"
#include "ticcutils/PrettyPrint.h"

using namespace std;

namespace TiCC {

  /// \cond HIDDEN
  class ImplementationError: public std::logic_error {
  public:
    explicit ImplementationError( const std::string& s ): std::logic_error( "CommandLine: implementation error." + s ){};
  };
  /// \endcond

  CL_Options::CL_Options( const string& short_opts, const string& long_opts ):
    is_init(false),
    debug(false){
    /// setup an CL_Options structure with possible options.
    /*!
      \param short_opts a string describing the acceptable short options
      \param long_opts a string describing the acceptable long options
    */
    allow_args( short_opts, long_opts );
  }

  CL_Options::CL_Options( ): is_init(false),debug(false){
    /// setup an CL_Options structure
 }

  CL_Options::~CL_Options(){
    /// destructor
  }

  void CL_Options::allow_args( const string& short_opts,
			       const string& long_opts ){
    /// register the possible options for this structure
    /*!
      \param short_opts a string describing the acceptable short options
      \param long_opts a string describing the acceptable long options
    */
    set_short_options( short_opts );
    set_long_options( long_opts );
  }

  bool CL_Options::parse_args( const int argc, const char * const *argv ){
    /// parse a commandline and check against the possible options
    /*!
      \param argc the number of aruments in \e argv
      \param argv a list of arguments
    */
    if ( is_init ){
      throw ImplementationError( "cannot parse() a commandline twice" );
    }
    if ( Parse_Command_Line( argc, argv ) ){
      is_init = true;
    }
    return is_init;
  }

  bool CL_Options::parse_args( const std::string& args ){
    /// parse a commandline and check against the possible options
    /*!
      \param args a string describing a command line
    */
    if ( is_init ){
      throw ImplementationError( "cannot parse() a commandline twice" );
    }
    const char *argstr = args.c_str();
    if ( Parse_Command_Line( 0, &argstr ) ){
      is_init = true;
    }
    return is_init;
  }

  ostream& operator<<( ostream& os, const CL_item& it ){
    /// output a CL_item to a stream
    os << it.toString();
    return os;
  }

  string CL_item::toString( ) const {
    /// convert a CL_item to a string
    string result;
    if ( _long_opt ){
      result = "--" + _option;
      if ( !_value.empty() ){
	result += "=";
      }
      result += _value;
    }
    else {
      result += (_mood ? "+": "-" ) + _option + " " + _value;
    }
    return result;
  }

  ostream& operator<<( ostream& os, const CL_Options& cl ){
    /// output a CL_Options struct to a stream
    os << cl.toString() << " ";
    for ( const auto& opt : cl.MassOpts ){
      os << opt << " ";
    }
    return os;
  }

  string CL_Options::toString() const {
    /// convert a CL_Options struct to a string
    string result;
    for ( const auto& pos : Opts ){
      result += pos.toString() + " ";
    }
    if ( !result.empty() ){
      result.erase(result.length()-1);
    }
    return result;
  }

  ostream& CL_Options::dump( ostream& os ) {
    /// dump a complete CL_Options struct to a stream. (DEBUGGING ONLY)
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

  bool CL_Options::is_present_internal( const char c,
					string &opt_val,
					bool& mood ) const {
    /// check if a character option \e c is available
    /*!
      \param c the character to search for
      \param opt_val if \e c is available return the value
      \param mood if \e c is available return if it was set with + or -
      \return true if a result is found
      \note when NO result is found, \e opt_val is NOT changed
    */
    mood = false;
    for ( auto const& pos : Opts ){
      if ( pos.is_long() ){
	continue;
      }
      if ( pos.opt_char() == c ){
	opt_val = pos.value();
	mood = pos.get_mood();
	if ( debug ){
	  cerr << "is_present '" << c << "' ==> '" << opt_val << "'" << endl;
	}
	return true;
      }
    }
    if ( debug ){
      cerr << "is_present '" << c << "' FAILS " << endl;
    }
    return false;
  }

  bool CL_Options::is_present_internal( const string& w,
					string& opt_val ) const {
    /// check if a long option \e w is available
    /*!
      \param w the long options to search for
      \param opt_val if \e w is available return the value
      \return true if a result is found
      \note when NO result is found, \e opt_val is NOT changed
    */
    auto it = find_if( Opts.begin(), Opts.end(),
		       [&w]( const CL_item& o ){ return o.option() == w; } );
    if ( it != Opts.end() ){
      opt_val = it->value();
      if ( debug ){
	cerr << "is_present '" << w << "' ==> '" << opt_val << "'" << endl;
      }
      return true;
    }
    if ( debug ){
      cerr << "is_present '" << w << "' FAILS " << endl;
    }
    return false;
  }

  bool CL_Options::extract_internal( const char c,
				     string &opt_val,
				     bool& mood ) {
    /// check if a character option \e c is available and remove it
    /*!
      \param c the character to search for
      \param opt_val if \e c is available return the value
      \param mood if \e c is available return if it was set with + or -
      \return true if a result is found
      When the option is set, it is removed from the options list.
      \note when NO result is found, \e opt_val is NOT changed
    */
    mood = false;
    for ( auto pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( !pos->is_long() ){
	if ( pos->opt_char() == c ){
	  opt_val = pos->value();
	  mood = pos->get_mood();
	  Opts.erase(pos);
	  if ( debug ){
	    cerr << "extract '" << c << "' ==> '" << opt_val << "'" << endl;
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

  bool CL_Options::extract_internal( const string& w,
				     string &opt_val ) {
    /// check if a long option \e w is available, and extract it
    /*!
      \param w the long options to search for
      \param opt_val if \e w is available return the value
      \return true if a result is found
      When the option is set, it is removed from the options list.
      \note when NO result is found, \e opt_val is NOT changed
    */
    for ( auto pos = Opts.begin(); pos != Opts.end(); ++pos ){
      if ( pos->option() == w ){
	opt_val = pos->value();
	Opts.erase(pos);
	if ( debug ){
	  cerr << "extract '" << w << "' ==> '" << opt_val << "'" << endl;
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
    /// remove a character option from the list of options
    /*!
      \param c the character option we look for
      \param all when true, remove ALL occurrences
      \return true if at least 1 match was found
     */
    bool result = false;
    for ( auto pos = Opts.begin(); pos != Opts.end(); ){
      if ( pos->opt_char() == c ){
	result = true;
	pos = Opts.erase(pos);
	if ( !all ){
	  return result;
	}
      }
      else {
	++pos;
      }
    }
    return result;
  }

  bool CL_Options::remove( const string& w, bool all ){
    /// remove a long option from the list of options
    /*!
      \param w the long option we look for
      \param all when true, remove ALL occurrences
      \return true if at least 1 match was found
     */
    bool result = false;
    for ( auto pos = Opts.begin(); pos != Opts.end(); ){
      if ( pos->value() == w ){
	result = true;
	pos = Opts.erase(pos);
	if ( !all ){
	  return result;
	}
      }
      else {
	++pos;
      }
    }
    return result;
  }

  void CL_Options::insert( const string& s, const string& value ){
    /// add a long CL_item to the list of items
    CL_item cl( s, value );
    Opts.push_back( cl );
  }

  void CL_Options::insert( const char c, const string& value, bool mood ){
    /// add a short CL_item to the list of items
    CL_item cl( c, value, mood );
    Opts.push_back( cl );
  }

  /// status values of the argument parser
  enum argstat {
    PLUS,   ///< a + was parsed
    MIN,    ///< a - was parsed
    LONG,   ///< a long option is found
    MASS,   ///< we are parsing a 'mass' option
    UNKNOWN ///< not known (yet)
  };

  /// @cond HIDDEN
  struct arg {
    arg(): stat(UNKNOWN),c(0){};
    argstat stat;
    char c;
    string s;
    string val;
  };
  /// @endcond

  ostream& operator<<( ostream& os, const arg& a ){
    /// output an 'arg' to a stream
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

  vector<string> fix_quotes( const vector<string>& argv ){
    /// fix quotes in the input
    // handle only balanced quotes, and 1 quote per option
    vector<string> result;
    bool q_found = false;
    for ( auto str : argv ){
      if ( !q_found ){
	string::size_type pos = str.find("\"");
	if ( pos != string::npos ){
	  q_found = true;
	  str.erase( pos, 1 );
	}
	result.push_back( str );
      }
      else {
	string::size_type pos = str.find("\"");
	if ( pos != string::npos ){
	  // balanced
	  q_found = false;
	  str.erase( pos, 1 );
	}
	result.back() += " " + str;
      }
    }
    if ( q_found ){
      throw OptionError( "unbalanced double quotes (\") in commandline" );
    }
    return result;
  }

  bool CL_Options::Parse_Command_Line( const int Argc,
				       const char * const *Argv ){
    /// parse a commandline into a CL_options structure
    Opts.clear();
    vector<string> local_argv;
    if ( Argc == 0 ){
      if ( Argv != 0 &&
	   Argv[0] != 0 ){
	local_argv = split( Argv[0] );
	local_argv = fix_quotes( local_argv );
      }
      else {
	return false;
      }
    }
    else {
      _prog_name = basename(Argv[0]);
      for ( int i=1; i < Argc; ++i ){
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
	else if ( Option.size() >3
		  && Option[1] == '-'
		  && Option.back() == '=' ){
	  if ( i < local_argv.size()-1 ){
	    string Option2 = local_argv[i+1];
	    if ( debug ){
	      cerr << "examine potential extra option: " << Option2 << endl;
	    }
	    if ( Option2[0] != '+' && Option2[0] != '-' ){
	      Option += Option2;
	    }
	    ++i;
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
    for ( auto const& Option : cleaned ){
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
	  cerr << "MASS-OPTION=" << Option << endl;
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
	cerr << "we must check this list" << endl;
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
	    if ( has_par ){
	      cerr << "\"" << it->s << "\" must have a parameter." << endl;
	    }
	    else if ( has_opt ){
	      cerr << "\"" << it->s << "\" may have a parameter!" << endl;
	    }
	    else {
	      cerr << "\"" << it->s << "\" doesn't take a parameter." << endl;
	    }
	  }
	  if ( it->val.empty() ){
	    if ( !has_par ){
	      if ( debug ){
		cerr << "no parameter: OK" << endl;
	      }
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
		  throw OptionError( "missing value for long option: '--"
				     + it->s + "'" );
		}
		else {
		  if ( debug ){
		    cerr << "OK: " << it->val << endl;
		  }
		}
	      }
	      else if ( !has_opt ){
		if ( debug ){
		  cerr << " OHO FAILED " << endl;
		}
		throw OptionError( "missing value for long option: '--"
				   + it->s + "'" );
	      }
	    }
	  }
	  else if ( has_par || has_opt){
	    if ( debug ){
	      cerr << "found a parameter: " <<  it->val << endl;
	    }
	  }
	  else {
	    // ok, some random data. assume it to be a Mass opt
	    arg a = *it;
	    a.stat = MASS;
	    it->val.clear();
	    it = arguments.insert( ++it, a );
	    if ( debug ){
	      cerr << "inserted " << a << endl;
	    }
	  }
	}
	else if ( it->stat == MIN || it->stat == PLUS ){
	  if ( valid_chars.find( it->c ) == valid_chars.end() ){
	    throw OptionError( string("invalid option '") + it->c + "'" );
	  }
	  bool has_par = valid_chars_par.find( it->c ) != valid_chars_par.end();
	  bool has_opt = valid_chars_opt.find( it->c ) != valid_chars_opt.end();
	  if ( debug ){
	    if ( has_par ){
	      cerr << "'" << it->c << "' must have a parameter." << endl;
	    }
	    else if ( has_opt ) {
	      cerr << "'" << it->c << "' may have a parameter." << endl;
	    }
	    else {
	      cerr << "'" << it->c << "' doesn't take a parameter." << endl;
	    }
	  }
	  if ( it->val.empty() ){
	    if ( !has_par ){
	      if ( debug ){
		cerr << "no parameter: OK" << endl;
	      }
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
		  throw OptionError( string("missing value for option '-")
				     + it->c + "'" );
		}
		else {
		  if ( debug ){
		    cerr << "OK: " << it->val << endl;
		  }
		}
	      }
	      else if ( !has_opt ){
		if ( debug ){
		  cerr << " AHA FAILED " << endl;
		}
		throw OptionError( string("missing value for option '-")
				   + it->c + "'" );
	      }
	    }
	  }
	  else if ( has_par || has_opt ){
	    if ( debug ){
	      cerr << "found a parameter: " <<  it->val << endl;
	    }
	  }
	  else {
	    // ok, some random data. assume it to be a Mass opt
	    arg a = *it;
	    a.stat = MASS;
	    it->val.clear();
	    it = arguments.insert( ++it, a );
	    if ( debug ){
	      cerr << "inserted " << a << endl;
	    }
	  }
	}
	else if ( it->stat == MASS ){
	  // skip
	}
	++it;
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
      cerr << "Final mass opts: " << MassOpts << endl;
      cerr << "CL_options after Parse " << endl;
      dump(cerr);
      cerr << endl;
    }
    return true;
  }

  void CL_Options::set_short_options( const string& s ){
    /// set the valid short options
    /*!
      \param s a string representing the valid options

      such a line looks like: "ab:cde:" defining options a,b,c,d and e, were
      options b and e must have a parameter
     */
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
    /// return the current short options in a neatly manner
    /*!
      \return a string representation of the short options in such a way
      that they could be parsed again by set_short_options
    */
    string result;
    for ( auto const& it : valid_chars ){
      result += it;
      if ( valid_chars_par.find( it ) != valid_chars_par.end() ){
	result += ":";
      }
      else if ( valid_chars_opt.find( it ) != valid_chars_opt.end() ){
	result += "::";
      }
    }
    return result;
  }

  void CL_Options::set_long_options( const string& s ){
    /// set the valid long options
    /*!
      \param s a string representing the valid options
     */
    vector<string> parts = TiCC::split_at( s, "," );
    for ( auto value: parts ){
      string::size_type pos = value.find( ':' );
      if ( pos != string::npos ){
	// found a ':'
	if ( pos == value.size()-2){
	  if ( value[value.size()-1] == ':' ){
	    // Ok, two ':' at the end
	    // remove them
	    value.resize(value.size()-2);
	    valid_long_opt.insert( value );
	  }
	  else {
	    throw ImplementationError( "':' may only be present at the end of a long option specification ("
			       + value + ")" );
	  }
	}
	else if ( pos == value.size()-1){
	  // Ok, one ':' at the end
	  // remove it
	  value.pop_back();
	  valid_long_par.insert( value );
	}
	else {
	  throw ImplementationError( "':' may only be present at the end of a long option specification ("
				     + value + ")" );
	}
      }
      valid_long.insert( value );
    }
  }

  string CL_Options::get_long_options() const {
    /// return the current long options in a neatly manner
    /*!
      \return a string representation of the long options in such a way
      that they could be parsed again by set_long_options
    */
    string result;
    for ( auto const& s : valid_long ){
      result += s;
      if ( valid_long_par.find( s ) != valid_long_par.end() ){
	result += ":";
      }
      else if ( valid_long_opt.find( s ) != valid_long_opt.end() ){
	result += "::";
      }
      result += ",";
    }
    if ( !result.empty() ){
      result.pop_back(); // remove last ','
    }
    return result;
  }


}
