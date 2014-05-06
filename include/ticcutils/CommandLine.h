/*
  $Id: CommandLine.h 16820 2014-01-06 10:12:30Z sloot $
  $URL: https://ilk.uvt.nl/svn/trunk/sources/Timbl6/include/timbl/CommandLine.h $

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
#ifndef TICC_COMMANDLINE_H
#define TICC_COMMANDLINE_H

#include <vector>
#include <iosfwd>
#include <stdexcept>

namespace TiCC {

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
  private:
    std::string opt_word;
    std::string option;
    bool mood;
    bool longOpt;
  };

  class CL_Options {
    friend std::ostream& operator<<( std::ostream&, const CL_Options& );
  public:
    CL_Options( const int, const char * const * );
    CL_Options( const std::string& );
    ~CL_Options();
    bool present( const char ) const;
    bool find( const char, std::string&, bool& ) const;
    bool find( const std::string&, std::string& ) const;
    bool remove( const char, bool = false );
    bool remove( const std::string& );
    void insert( const char, const std::string&, bool );
    void insert( const std::string&, const std::string& );
    const std::vector<std::string>& getMassOpts() const { return MassOpts; };
  private:
    void Split_Command_Line( const int, const char * const * );
    std::vector<CL_item> Opts;
    std::vector<std::string> MassOpts;
    CL_Options( const CL_Options& );
    CL_Options& operator=( const CL_Options& );
  };

  class OptionError: public std::runtime_error {
  public:
  OptionError( const std::string& s ): std::runtime_error( "option-error: " + s ){};
  };

}
#endif
