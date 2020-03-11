/*
  Copyright (c) 2006 - 2020
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

#ifndef TICC_COMMANDLINE_H
#define TICC_COMMANDLINE_H

#include <set>
#include <vector>
#include <iosfwd>
#include <stdexcept>
#include <ticcutils/StringOps.h>

namespace TiCC {

  class OptionError: public std::runtime_error {
  public:
    explicit OptionError( const std::string& s ): std::runtime_error( "option-error: " + s ){};
  };

  class CL_item {
    friend std::ostream& operator<<( std::ostream&, const CL_item& );
  public:
  CL_item( const std::string& s, const std::string& o ):
    _opt_value(s), _option( o ), _mood( false ), _long_opt(true) {};
  CL_item( char c, const std::string& o, bool m=false ):
    _opt_value(1,c),_option( o ), _mood( m ), _long_opt(false){};
  CL_item( const CL_item& in ):
    _opt_value( in._opt_value ),
      _option(in._option),
      _mood(in._mood),
      _long_opt(in._long_opt){
    };
    CL_item& operator=( const CL_item& in ){
      if ( &in != this ){
	_opt_value = in._opt_value;
	_option = in._option;
	_mood = in._mood;
	_long_opt = in._long_opt;
      }
      return *this;
    }
    char opt_char() const {
      /// return the first character of _opt_value
      return _opt_value[0];
    };
    const std::string& opt_value() const {
      /// return the option's value
      return _opt_value;
    };
    const std::string& option() const {
      /// return the option.
      return _option;
    };
    bool is_long() const {
      /// return true for a long option, false otherwise
      return _long_opt;
    };
    bool get_mood() const {
      /// return the 'mood' value ( '+' or '-' )
      /*!
	\return for a character option, return true if it was preceded by a '+',
	otherwise return false. For long options always return false
      */
      return _mood;
    };
    std::string toString( ) const;
  private:
    std::string _opt_value;
    std::string _option;
    bool _mood;
    bool _long_opt;
  };

  class CL_Options {
    friend std::ostream& operator<<( std::ostream&, const CL_Options& );
  public:
    typedef std::vector<CL_item>::const_iterator const_iterator;
    CL_Options();
    CL_Options( const std::string&, const std::string& );
    void allow_args( const std::string& = "", const std::string& = "" );
    ~CL_Options();
    const_iterator begin() const { return Opts.begin(); };
    const_iterator end() const { return Opts.end(); };
    bool parse_args( const int, const char * const * );
    bool parse_args( const std::string& );
    bool init( const int i, const char * const * a ){
      return parse_args( i, a );
    }
    bool init( const std::string& s ){
      return parse_args( s );
    }
    void set_short_options( const std::string& s );
    void set_long_options( const std::string& s );
    std::string prog_name() const { return _prog_name; };
    std::string get_short_options() const;
    std::string get_long_options() const;
    bool find( const char c, std::string& s, bool& b ) const {
      return is_present_internal( c, s, b );
    }
    bool is_present( const char c, std::string& v, bool& b ) const {
      return is_present_internal( c, v, b );
    }
    bool is_present( const char c ) const {
      bool b;
      std::string v;
      return is_present_internal( c, v, b );
    }
    template <class T>
      inline bool is_present( const char c, T& val ) const {
      std::string v;
      bool b;
      if ( is_present_internal( c, v, b ) ){
	if ( TiCC::stringTo( v, val ) )
	  return true;
	std::string msg = "wrong type for value of -";
	msg += c;
	msg += " " + v;
	throw OptionError( msg );
      }
      return false;
    }

    bool find( const std::string& w, std::string& s ) const {
      return is_present_internal( w, s );
    }
    bool is_present( const std::string& s ) const {
      std::string v;
      return is_present_internal( s, v );
    }
    template <class T>
      inline bool is_present( const std::string& s, T& val ) const {
      std::string v;
      if ( is_present_internal( s, v ) ){
	if ( TiCC::stringTo( v, val ) )
	  return true;
	throw OptionError( "wrong type for value of --" + s + "=" + v );
      }
      return false;
    }
    bool extract( const char c, std::string& v, bool& b){
      return extract_internal( c, v, b);
    }
    bool extract( const char c, std::string& s ){
      bool b;
      return extract_internal( c, s, b );
    };
    bool extract( const char c ){
      bool b;
      std::string v;
      return extract_internal( c, v, b );
    };
    template <class T>
      inline bool extract( const char c, T& val ){
      std::string v;
      bool b;
      if ( extract_internal( c, v, b ) ){
	if ( TiCC::stringTo( v, val ) )
	  return true;
	std::string msg = "wrong type for value of -";
	msg += c;
	msg += " " + v;
	throw OptionError( msg );
      }
      return false;
    }

    bool extract( const std::string& s ){
      std::string v;
      return extract_internal( s, v );
    }
    template <class T>
      inline bool extract( const std::string& s, T& val ){
      std::string v;
      if ( extract_internal( s, v ) ){
	if ( TiCC::stringTo( v, val ) )
	  return true;
	throw OptionError( "wrong type for value of --" + s + "=" + v );
      }
      return false;
    }

    bool remove( const char, bool = false );
    bool remove( const std::string&, bool = false );
    void insert( const char, const std::string&, bool );
    void insert( const std::string&, const std::string& );
    bool empty() const { return Opts.empty(); };
    void set_debug( bool b ) { debug = b; };
    std::string toString() const;
    std::ostream& dump( std::ostream& );
    const std::vector<std::string>& getMassOpts() const { return MassOpts; };
  private:
    bool Parse_Command_Line( const int, const char * const * );
    bool is_present_internal( const char, std::string&, bool& ) const;
    bool is_present_internal( const std::string&, std::string& ) const;
    bool extract_internal( const char, std::string&, bool& );
    bool extract_internal( const std::string&, std::string& );
    std::vector<CL_item> Opts;
    std::vector<std::string> MassOpts;
    CL_Options( const CL_Options& );
    CL_Options& operator=( const CL_Options& );
    std::set<char> valid_chars;
    std::set<char> valid_chars_par;
    std::set<char> valid_chars_opt;
    std::set<std::string> valid_long;
    std::set<std::string> valid_long_par;
    std::set<std::string> valid_long_opt;
    std::string _prog_name;
    bool is_init;
    bool debug;
  };

  template <>
    inline bool CL_Options::is_present( const char c, std::string& s ) const {
    bool b;
    return is_present_internal( c, s, b );
  }

  template <>
    inline bool CL_Options::is_present( const std::string& s, std::string& v ) const {
    return is_present_internal( s, v );
  }

  template <>
    inline bool CL_Options::extract( const char c, std::string& v ){
    bool b;
    return extract_internal( c, v, b );
  }

  template <>
    inline bool CL_Options::extract( const std::string& s, std::string& v ){
    return extract_internal( s, v );
  }

}
#endif
