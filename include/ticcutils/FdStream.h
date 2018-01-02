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

#ifndef FD_STREAM_H
#define FD_STREAM_H

#include <iostream>
#include <streambuf>

class fdoutbuf: public std::streambuf {
 public:
  explicit fdoutbuf( int _fd );
  fdoutbuf();
  bool connect( int );
 protected:
  virtual int overflow( int );
  virtual std::streamsize xsputn( const char *, std::streamsize );
  int fd; // file descriptor
};

class fdostream: public std::ostream {
 protected:
  fdoutbuf buf;
 public:
 explicit fdostream( int fd ): std::ostream(&buf), buf(fd) {};
 fdostream(): std::ostream(&buf) {};
  bool open( int fd );
};

class fdinbuf: public std::streambuf {
 public:
  fdinbuf();
  explicit fdinbuf( int);
  bool connect( int );
 protected:
  virtual int underflow();
  int fd; // file descriptor
  static const int putbackSize = 4;
  static const int bufferSize = 512;
  char buffer[bufferSize];
};

class fdistream: public std::istream {
 protected:
  fdinbuf buf;
 public:
  explicit fdistream( int fd ): std::istream(&buf), buf(fd) {};
 fdistream(): std::istream(&buf) {};
  bool open( int fd );
};

bool nb_getline( std::istream&, std::string&, int& );
bool nb_putline( std::ostream&, const std::string&, int& );
#endif
