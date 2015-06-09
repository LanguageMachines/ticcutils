/*
  $Id$
  $URL$

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
  OptionError( const std::string& s ): std::runtime_error( "option-error: " + s ){};
  };

  class CL_item {
    friend std::ostream& operator<<( std::ostream&, const CL_item& );
  public:
  CL_item( const std::string& s, const std::string& o ):
    opt_word(s), option( o ), mood( false ), longOpt(true) {};
  CL_item( char c, const std::string& o, bool m=false ):
    option( o ), mood( m ), longOpt(false){ opt_word = c; };
  CL_item( const CL_item& in ):
    opt_word( in.opt_word ), option(in.option),
      mood(in.mood), longOpt(in.longOpt){
    };
    CL_item& operator=( const CL_item& in ){
      if ( &in != this ){
	opt_word = in.opt_word;
	option = in.option;
	mood = in.mood;
	longOpt = in.longOpt;
      }
      return *this;
    }
    bool Mood() const { return mood; };
    char OptChar() const { return opt_word[0]; };
    const std::string& OptWord() const { return opt_word; };
    const std::string& Option() const { return option; };
    bool isLong() const { return longOpt; };
    bool getMood() const { return mood; };
    std::string toString( ) const;
  private:
    std::string opt_word;
    std::string option;
    bool mood;
    bool longOpt;
  };

  class CL_Options {
    friend std::ostream& operator<<( std::ostream&, const CL_Options& );
  public:
    CL_Options( const std::string& ="", const std::string& ="" );
    CL_Options( const int, const char * const *,
		const std::string& = "", const std::string& = "" );
    ~CL_Options();
    typedef std::vector<CL_item>::const_iterator const_iterator;
    const_iterator begin() const { return Opts.begin(); };
    const_iterator end() const { return Opts.end(); };
    bool init( const int, const char * const * );
    bool init( const std::string& );
    void set_short_options( const std::string& s );
    void set_long_options( const std::string& s );
    std::string prog_name() const { return _prog_name; };
    std::string get_short_options() const;
    std::string get_long_options() const;
    bool is_present( const char, std::string&, bool& ) const;
    bool find( const char c, std::string& s, bool& b ) const {
      return is_present( c, s, b );
    }
    bool is_present( const char c, std::string& s ) const {
      bool b;
      return is_present( c, s, b );
    }
    bool is_present( const char c ) const {
      bool b;
      std::string v;
      return is_present( c, v, b );
    }
    bool is_present( const std::string&, std::string& ) const;
    bool find( const std::string& w, std::string& s ) const {
      return is_present( w, s );
    }
    bool is_present( const std::string& s ) const {
      std::string v;
      return is_present( s, v );
    }
    bool extract( const char, std::string&, bool& );
    bool pull( const char c, std::string& s, bool& b){
      return extract( c, s, b ); };
    bool extract( const char c, std::string& s ){
      bool b;
      return extract( c, s, b );
    };
    bool extract( const char c ){
      bool b;
      std::string v;
      return extract( c, v, b );
    };
    template <class T>
      inline bool extract( const char c, T& val ){
      std::string v;
      if ( extract( c, v ) ){
	if ( TiCC::stringTo( v, val ) )
	  return true;
	std::string msg = "wrong type for value of -";
	msg += c;
	msg += " " + v;
	throw OptionError( msg );
      }
      return false;
    }
    bool extract( const std::string&, std::string& );
    bool pull( const std::string& w, std::string& s) {
      return extract( w, s ); };
    bool extract( const std::string& s ){
      std::string v;
      return extract( s, v );
    }
    template <class T>
      inline bool extract( const std::string& s, T& val ){
      std::string v;
      if ( extract( s, v ) ){
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


}
#endif
