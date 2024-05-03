// ============================================================================
// gzstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : gzstream.h
// Revision      : $Revision: 16571 $
// Revision_date : $Date: 2013-10-09 10:33:53 +0200 (Wed, 09 Oct 2013) $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// Modernized    : Ko vd Sloot (12-01-2017)
// Standard streambuf implementation following Nicolai Josuttis, "The
// Standard C++ Library".
// ============================================================================

#ifndef GZSTREAM_H
#define GZSTREAM_H 1

// standard C++ with new header file names and std:: namespace
#include <cstring>
#include <iostream>
#include <fstream>
#include <zlib.h>

#ifdef GZSTREAM_NAMESPACE
namespace GZSTREAM_NAMESPACE {
#endif

  /// \brief Internal class to implement gzstream. See below for user classes.
  class gzstreambuf : public std::streambuf {
  private:
    gzstreambuf( const gzstreambuf& ) =delete; // no copies please
    gzstreambuf& operator=( const gzstreambuf& ) =delete; // no copies please

    static const int bufferSize = 47+256;    // size of data buff
    // totals 512 bytes under g++ for igzstream at the end.

    gzFile           file;               // file handle for compressed file
    char             buffer[bufferSize]; // data buffer
    char             opened;             // open/close state of stream
    int              mode;               // I/O mode

    int flush_buffer();
  public:
  gzstreambuf() : file(0), opened(0), mode(-1) {
      setp( buffer, buffer + (bufferSize-1));
      setg( buffer + 4,     // beginning of putback area
	    buffer + 4,     // read position
	    buffer + 4);    // end position
      // ASSERT: both input & output capabilities will not be used together
    }
    int is_open() { return opened; }
    gzstreambuf* open( const std::string &name, int open_mode );
    gzstreambuf* close();
    ~gzstreambuf() { close(); }

    virtual int     overflow( int c = EOF);
    virtual int     underflow();
    virtual int     sync();
  };

  /// \brief Internal class to implement gzstream. See below for user classes.
  class gzstreambase : virtual public std::ios {
  protected:
    gzstreambuf buf;
  public:
    gzstreambase() { init(&buf); }
    gzstreambase( const std::string&, int );
    ~gzstreambase();
    void my_open( const std::string&, int );
    void close();
    virtual gzstreambuf* rdbuf() { return &buf; }
  };

  /// \brief A stream class to support .gz files
  ///
  /// Use igzstream analogously to ifstream. It reads files based on the gz*
  /// function interface of the zlib. Files are compatible with gzip
  /// compression.
  class igzstream : public gzstreambase, public std::istream {
  public:
    igzstream() : std::istream( &buf) {}
    explicit igzstream( const std::string& name, int open_mode = std::ios::in )
      : gzstreambase( name, open_mode ), std::istream( &buf ) {}
    gzstreambuf* rdbuf() override { return gzstreambase::rdbuf(); }
    void open( const std::string& name,
	       int open_mode = std::ios::in ) {
      gzstreambase::my_open( name, open_mode);
    }
  };

  /// \brief A stream class to support .gz files
  ///
  /// Use ogzstream analogously to ofstream. It writes files based on the gz*
  /// function interface of the zlib. Files are compatible with gzip
  /// compression.
  class ogzstream : public gzstreambase, public std::ostream {
  public:
    ogzstream() : std::ostream( &buf) {}
    explicit ogzstream( const std::string& name, int mode = std::ios::out )
      : gzstreambase( name, mode ), std::ostream( &buf ) {}
    gzstreambuf* rdbuf() override { return gzstreambase::rdbuf(); }
    void open( const std::string& name,
	       int open_mode = std::ios::out ) {
      gzstreambase::my_open( name, open_mode );
    }
  };

  gzstreambuf* gzstreambuf::open( const std::string& name, int open_mode) {
    if ( is_open())
      return (gzstreambuf*)0;
    mode = open_mode;
    // no append nor read/write mode
    if ((mode & std::ios::ate) || (mode & std::ios::app)
	|| ((mode & std::ios::in) && (mode & std::ios::out)))
      return (gzstreambuf*)0;
    std::string fmode;
    if ( mode & std::ios::in)
      fmode += 'r';
    else if ( mode & std::ios::out)
      fmode += 'w';
    fmode += 'b';
    file = gzopen( name.c_str(), fmode.c_str() );
    if (file == 0){
      return (gzstreambuf*)0;
    }
    opened = 1;
    return this;
  }

  gzstreambuf * gzstreambuf::close() {
    if ( is_open()) {
      sync();
      opened = 0;
      if ( gzclose( file) == Z_OK)
	return this;
    }
    return (gzstreambuf*)0;
  }

  int gzstreambuf::underflow() { // used for input buffer only
    if ( gptr() && ( gptr() < egptr()))
      return * reinterpret_cast<unsigned char *>( gptr());

    if ( ! (mode & std::ios::in) || ! opened)
      return EOF;
    // Josuttis' implementation of inbuf
    int n_putback = gptr() - eback();
    if ( n_putback > 4)
      n_putback = 4;
    memcpy( buffer + (4 - n_putback), gptr() - n_putback, n_putback);

    int num = gzread( file, buffer+4, bufferSize-4);
    if (num <= 0) // ERROR or EOF
      return EOF;

    // reset buffer pointers
    setg( buffer + (4 - n_putback),   // beginning of putback area
	  buffer + 4,                 // read position
	  buffer + 4 + num);          // end of buffer

    // return next character
    return * reinterpret_cast<unsigned char *>( gptr());
  }

  int gzstreambuf::flush_buffer() {
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    int w = pptr() - pbase();
    if ( gzwrite( file, pbase(), w) != w)
      return EOF;
    pbump( -w);
    return w;
  }

  int gzstreambuf::overflow( int c) { // used for output buffer only
    if ( ! ( mode & std::ios::out) || ! opened)
      return EOF;
    if (c != EOF) {
      *pptr() = c;
      pbump(1);
    }
    if ( flush_buffer() == EOF)
      return EOF;
    return c;
  }

  int gzstreambuf::sync() {
    // Changed to use flush_buffer() instead of overflow( EOF)
    // which caused improper behavior with std::endl and flush(),
    // bug reported by Vincent Ricard.
    if ( pptr() && pptr() > pbase()) {
      if ( flush_buffer() == EOF)
	return -1;
    }
    return 0;
  }

  // --------------------------------------
  // class gzstreambase:
  // --------------------------------------

  gzstreambase::gzstreambase( const std::string& name, int mode ) {
    init( &buf );
    my_open( name, mode );
  }

  gzstreambase::~gzstreambase() {
    buf.close();
  }

  void gzstreambase::my_open( const std::string& name, int open_mode ) {
    if ( ! buf.open( name, open_mode ) )
      clear( rdstate() | std::ios::badbit );
  }

  void gzstreambase::close() {
    if ( buf.is_open() )
      if ( ! buf.close() )
	clear( rdstate() | std::ios::badbit);
  }

#ifdef GZSTREAM_NAMESPACE
} // namespace GZSTREAM_NAMESPACE
#endif

#endif // GZSTREAM_H
// ============================================================================
// EOF //
