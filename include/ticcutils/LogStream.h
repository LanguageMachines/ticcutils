/*
  Copyright (c) 2006 - 2024
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

namespace TiCC {

  /// \brief Logstream is a class to synchronize output in Multi-Threaded
  /// programs.
  ///
  /// LogStream used mutexes to assure that output from different threads is
  /// not mangled
  class LogStream : public std::ostream {
    friend bool IsActive( LogStream & );
    friend bool IsActive( LogStream * );
  public:
    explicit LogStream();
    explicit LogStream( int );
    LogStream( std::ostream&,
	       LogFlag = StampBoth );
    LogStream( const LogStream * );
    LogStream *create( const std::string&, std::ios_base::openmode = std::ios::out );
    bool set_single_threaded_mode();
    bool single_threaded() const { return single_threaded_mode; };
    void set_threshold( LogLevel t ){ buf.Threshold( t ); };
    LogLevel get_threshold() const { return buf.Threshold(); };
    void set_level( LogLevel l ){ buf.Level( l ); };
    LogLevel get_level() const{ return buf.Level(); };
    void associate( std::ostream& os ) { buf.AssocStream( os ); };
    void set_stamp( LogFlag f ){ buf.StampFlag( f ); };
    LogFlag get_stamp() const { return buf.StampFlag(); };
    void set_message( const std::string& s ){ buf.Message( s ); };
    void add_message( const std::string& );
    void add_message( const int );
    const std::string& get_message() const { return buf.Message(); };
    static bool Problems();
  private:
    LogBuffer buf;
    // prohibit assignment
    LogStream& operator=( const LogStream& ) = delete;
    bool IsBlocking();
    bool single_threaded_mode;
  };

  bool IsActive( LogStream & );
  bool IsActive( LogStream * );

  /// \brief create a LogStream
  class Log {
  public:
    explicit Log( LogStream * );
    explicit Log( LogStream& l );
    ~Log();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    Log( const Log& ) = delete;
    Log& operator=( const Log& ) = delete;
  };

  /// \brief create a debugging LogStream
  class Dbg{
  public:
    explicit Dbg( LogStream * );
    explicit Dbg( LogStream& l );
    ~Dbg();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    Dbg( const Dbg& ) = delete;
    Dbg& operator=( const Dbg& ) = delete;
  };

  /// \brief a debugging LogStream for heavy output
  class xDbg{
  public:
    explicit xDbg( LogStream * );
    explicit xDbg( LogStream& l );
    ~xDbg();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    xDbg( const xDbg& ) = delete;
    xDbg& operator=( const xDbg& ) = delete;
  };

  /// \brief a debugging LogStream for extreme output
  class xxDbg{
  public:
    explicit xxDbg( LogStream * );
    explicit xxDbg( LogStream& l );
    ~xxDbg();
    LogStream& operator *();
  private:
    LogStream *my_stream;
    LogLevel my_level;
    xxDbg( const xxDbg& ) = delete;
    xxDbg& operator=( const xxDbg& ) = delete;
  };

}
#endif
