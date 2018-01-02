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
#ifndef TICC_LOGSTREAM_H
#define TICC_LOGSTREAM_H

#include <iostream>
#include <string>
#include "ticcutils/LogBuffer.h"

#if defined __GNUC__ || __IBMCPP__
#define LTGT <>
#else
#define LTGT
#endif

namespace TiCC {

  template<class T> class o_manip;
  template <class T> std::ostream& operator << (std::ostream&, const o_manip<T>& );

  template<class T> class o_manip{
    typedef std::ostream& (*FRM)(std::ostream&, T );
    FRM func;
    T l;
  public:
    o_manip<T>( FRM f, T ll ): func(f), l(ll) {};
    friend std::ostream& operator << LTGT (std::ostream&, const o_manip<T>& );
    o_manip<T>( const o_manip<T>& o ):
    func(o.func), l(o.l){};
  private:
    o_manip<T> operator=( const o_manip<T>& );
  };

  template<class T>
    inline std::ostream& operator << (std::ostream& os,
				      const o_manip<T>& m ){
    return m.func( os, m.l );
  }

  class LogStream : public std::ostream {
    friend o_manip<LogLevel> setlevel( LogLevel );
    friend o_manip<LogLevel> setthreshold( LogLevel );
    friend o_manip<LogFlag> setstamp( LogFlag );
    friend o_manip<const std::string& > setmessage( const std::string& );
    friend o_manip<const std::string&> addmessage( const std::string& );
    friend o_manip<const std::string&> addmessage( const int );
    friend o_manip<const std::string&> write_buf(const std::string& );
    friend bool IsActive( LogStream & );
    friend bool IsActive( LogStream * );
  public:
    explicit LogStream();
    explicit LogStream( int );
    LogStream( const std::string&, LogFlag = StampBoth );
    LogStream( std::ostream&, const std::string& = "",
	       LogFlag = StampBoth );
    LogStream( const LogStream&, const std::string&, LogFlag );
    LogStream( const LogStream&, const std::string& );
    LogStream( const LogStream * );
    bool set_single_threaded_mode();
    bool single_threaded() const { return single_threaded_mode; };
    void setthreshold( LogLevel t ){ buf.Threshold( t ); };
    LogLevel getthreshold() const { return buf.Threshold(); };
    void setlevel( LogLevel l ){ buf.Level( l ); };
    LogLevel getlevel() const{ return buf.Level(); };
    void associate( std::ostream& os ) { buf.AssocStream( os ); };
    //  std::ostream& associate() const { return buf.AssocStream(); };
    void setstamp( LogFlag f ){ buf.StampFlag( f ); };
    LogFlag getstamp() const { return buf.StampFlag(); };
    void message( const std::string& s ){ buf.Message( s.c_str() ); };
    void addmessage( const std::string& );
    void addmessage( const int );
    const std::string& message() const { return buf.Message(); };
    static bool Problems();
  private:
    LogBuffer buf;
    // prohibit assignment
    LogStream& operator=( const LogStream& );
    bool IsBlocking();
    bool single_threaded_mode;
  };

  bool IsActive( LogStream & );
  bool IsActive( LogStream * );

  class Log{
  public:
    explicit Log( LogStream * );
    explicit Log( LogStream& l );
    ~Log();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    Log( const Log& );
    Log& operator=( const Log& );
  };

  class Dbg{
  public:
    explicit Dbg( LogStream * );
    explicit Dbg( LogStream& l );
    ~Dbg();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    Dbg( const Dbg& );
    Dbg& operator=( const Dbg& );
  };

  class xDbg{
  public:
    explicit xDbg( LogStream * );
    explicit xDbg( LogStream& l );
    ~xDbg();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    xDbg( const xDbg& );
    xDbg& operator=( const xDbg& );
  };

  class xxDbg{
  public:
    explicit xxDbg( LogStream * );
    explicit xxDbg( LogStream& l );
    ~xxDbg();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    xxDbg( const xxDbg& );
    xxDbg& operator=( const xxDbg& );
  };

}
#endif
