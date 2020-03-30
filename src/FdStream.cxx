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

#include "ticcutils/FdStream.h"

#include <cstring>
#include <cstdio>
#include <cerrno>
#include <csignal>
#include <iostream>
#include <streambuf>
#include <stdexcept>
#include <unistd.h>
#include <cstdlib>

using namespace std;

void milli_wait( int m_secs ){
  /// sleep for some milli-seconds
  /*!
    \param m_secs the milliseconds to sleep
  */
  struct timespec tv;
  ldiv_t div = ldiv( m_secs, 1000 );
  tv.tv_sec = div.quot;               // seconds
  tv.tv_nsec = div.rem * 1000000;     // nanoseconds
  while ( nanosleep( &tv, &tv ) < 0 ){
    // continue when interrupted
  }
}

fdoutbuf::fdoutbuf(): fd(-1) {
  /// constructor for a non-initialized fd output buffer
}
fdoutbuf::fdoutbuf( int _fd ): fd(_fd) {
  /// constructor for a fd output buffer connected to a file descriptor
  /*!
    \param _fd the file descriptor
  */
}

bool fdoutbuf::connect( int _fd ){
  /// connect a fd output buffer to a file descriptor
  /*!
    \param _fd the file descriptor
    \return true on succes. Will throw otherwise
  */
  if ( fd >= 0 ){
    throw logic_error( "FDstream: output buffer already connected" );
  }
  fd = _fd;
  return true;
}

int fdoutbuf::overflow( int c ){
  /// overloaded version of streambuf::overflow()
  /*!
    \param c the character to write (integer value!)
    \return the character written, OR EOF when we are done
  */
  if ( c != EOF ){
    char z = c;
    if ( write( fd, &z, 1 ) != 1 ) {
      return EOF;
    }
  }
  return c;
}

streamsize fdoutbuf::xsputn( const char *s, streamsize num ){
  /// overloaded version of streambuf::xputn()
  /*!
    \param s the range of characters to write
    \param num the number of characters to write
    \return the number of characters actually written
  */
  return write( fd, s, num );
}


fdinbuf::fdinbuf(): fd(-1) {
  /// constructor for a non-initialized fd input buffer
  setg( buffer + putbackSize,
	buffer + putbackSize,
	buffer + putbackSize );
}

fdinbuf::fdinbuf( int _fd ): fd(_fd) {
  /// constructor for a fd input buffer connected to a file descriptor
  /*!
    \param _fd the file descriptor
  */
  setg( buffer + putbackSize,
	buffer + putbackSize,
	buffer + putbackSize );
}

bool fdinbuf::connect( int _fd ){
  /// connect a fd input buffer to a file descriptor
  /*!
    \param _fd the file descriptor
    \return true on succes. Will throw otherwise
  */
  if ( fd >= 0 ){
    throw logic_error( "FDstream: input buffer already connected" );
  }
  fd = _fd;
  return true;
}

int fdinbuf::underflow(){
  /// overloaded version of streambuf::underflow()
  /*!
    \return the next character in the input WITHOUT reading it. Might return EOF
  */
  if ( gptr() < egptr() ){
    return traits_type::to_int_type(*gptr());
  }
  int numPutBack = gptr() - eback();
  if ( numPutBack > putbackSize ) {
    numPutBack = putbackSize;
  }

  std::memmove( buffer + putbackSize - numPutBack,
	       gptr() - numPutBack,
	       numPutBack );
  int num = read( fd, buffer+putbackSize, bufferSize - putbackSize );
  if ( num <= 0 ){
    setg( 0, 0, 0 );
    return traits_type::eof();
  }
  setg( buffer + putbackSize - numPutBack,
	buffer + putbackSize,
	buffer + putbackSize + num );
  return traits_type::to_int_type(*gptr());
}

// #define DEBUG

bool nb_getline( istream& is, string& result, int& timeout ){
  /// a getline for nonblocking connections.
  /*!
    \param is the stream te read from. Should be non-blocking!
    \param result the string read
    \param timeout the retry time in seconds until failure
    \return false except when correctly terminated
    ( meaning \n or an EOF after at least some input)

    this function will retry the read for a few special cases until timeout
    is reached.
  */
  result = "";
  char c;
  int count = 0;
  while ( is && timeout > 0 ){
    if ( is.get( c ) ){
#ifdef DEBUG
      cerr << "-'" << c << "'-" << endl;
#endif
      if ( c == '\n' ){
	return true;
      }
      result += c;
    }
    else if ( errno == EAGAIN || errno == EWOULDBLOCK ){
#ifdef DEBUG
      cerr << "Blocked again" << endl;
#endif
      is.clear();
      errno = 0;
      milli_wait(100);
      if ( ++count == 10 ){
	--timeout;
	count = 0;
      }
    }
    else if ( is.eof() && !result.empty() ){
      return true;
    }
    else {
      return false;
    }
  }
  return false;
}

bool nb_putline( ostream& os, const string& what, int& timeout ){
  /// a putline for nonblocking connections.
  /*!
    \param os the output stream, must be NON-BLOCKING
    \param what the buffer to write
    \param timeout the time in seconds to wait until failure
    \return false except when correctly terminated.
    retry for a few special cases until timeout reached.

    Must handle SIGPIPE.
  */
  unsigned int i=0;
  int count = 0;
  bool result = true;
  typedef void (*sig_hndl)(int);
  sig_hndl sig;
  // specify that the SIGPIPE signal is to be ignored
  sig=signal(SIGPIPE,SIG_IGN);
  while ( result && i < what.length() && os && timeout > 0 ){
    if ( os.put( what[i] ) ){
      ++i;
    }
    else if ( errno == EAGAIN || errno == EWOULDBLOCK ){
#ifdef DEBUG
      cerr << "Blocked again" << endl;
#endif
      os.clear();
      errno = 0;
      milli_wait(100);
      if ( ++count == 10 ){
	--timeout;
	count = 0;
      }
    }
    else
      result = false;
    if ( os.eof() ){
      result = false;
    }
  }
  // restore old handler
  signal( SIGPIPE, sig );
  return result;
}

bool fdistream::open( int fd ){
  /// open an input stream connected to a file descriptor
  buf.connect( fd );
  return true;
}

bool fdostream::open( int fd ){
  /// open an output stream connected to a file descriptor
  buf.connect( fd );
  return true;
}
