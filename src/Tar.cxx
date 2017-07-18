/*
  Copyright (c) 2006 - 2017
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

#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "config.h"
#include <sys/fcntl.h>
#include "ticcutils/StringOps.h"
#if HAVE_WORKING_REGEX == 2
#  include <boost/regex.hpp>
#  define regex boost::regex
#  define regex_error boost::regex_error
#  define regex_search boost::regex_search
#else
#  include <regex>
#endif
#include "ticcutils/Tar.h"

using namespace std;

namespace TiCC {

#ifndef HAVE_LIBTAR_H

  void tarfail( const string& s ){
    throw runtime_error( "unable to execute '" + s + " 'tar support not enabled" );
  }

  tar::tar() {
  }

  tar::~tar(){
  }

  bool tar::open( const string& ){
    tarfail( "open()" );
    return false;
  }

  bool tar::close() {
    return false;
  }

  bool tar::extract_file_names( vector<string>&,
				const string& ){
    tarfail( "extract_file_names()" );
    return false;
  }

  bool tar::extract_file_names_match( vector<string>&,
				      const string& ){
    tarfail( "extract_file_names_match()" );
    return false;
  }

  bool tar::extract_ifstream( const string&, ifstream& ){
    tarfail( "extract_ifstream()" );
    return false;
  }

  bool tar::next_ifstream( ifstream&, string&){
    tarfail( "next_ifstream()" );
    return false;
  }

#else
  tar::tar() : tar_file(0)
  {}

  tar::~tar(){
    close();
  }

  bool tar::open( const string& name ){
    if ( !tarname.empty() ){
      cerr << "tar already opened!" << endl;
      return false;
    }
    TAR *tar_file;
    int res = tar_open( &tar_file, (char*)name.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( res < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    tar_close( tar_file );
    tarname = name;
    return true;
  }

  bool tar::extract_file_names( vector<string>& result,
				const string& ext ){
    result.clear();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    TAR local_tar_blob;
    TAR *local_tar = &local_tar_blob;
    int stat = tar_open( &local_tar, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( stat < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    stat = th_read( local_tar );
    while ( stat == 0  ) {
      if ( TH_ISREG( local_tar ) ){
	string name = local_tar->th_buf.name;
	if ( ext.empty() ||
	     name.rfind( ext ) != string::npos ){
	  result.push_back( name );
	}
	tar_skip_regfile( local_tar );
      }
      stat = th_read( local_tar );
    }
    tar_close( local_tar );
    return true;
  }

  static string wildToRegExp( const string& wild ){
    // convert 'shell'-like wildcards into a regexp
    string result;
    for ( auto const& c : wild ){
      switch( c ){
      case '*':
	result += ".*";
	break;
      case '?':
	result += ".";
	break;
      case '.':
	result += "\\.";
	break;
      default:
	result += c;
      }
    }
    //    cerr << "wild to regexp: " << wild << " ==> " << result << endl;
    return result;
  }

  bool tar::extract_file_names_match( vector<string>& result,
				      const string& wild ){
    result.clear();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    TAR local_tar_blob;
    TAR *local_tar = &local_tar_blob;
    int stat = tar_open( &local_tar, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( stat < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    string pat = wildToRegExp( wild );
    try {
      regex rx( pat );
      stat = th_read( local_tar );
      while ( stat == 0  ) {
	if ( TH_ISREG( local_tar ) ){
	  string name = local_tar->th_buf.name;
	  if ( regex_search( name, rx ) ){
	    result.push_back( name );
	  }
	  tar_skip_regfile( local_tar );
	}
	stat = th_read( local_tar );
      }
      tar_close( local_tar );
    }
    catch( regex_error& e ){
      string mess = "tar:extract_file_names_match() invalid regexp: ";
      mess += e.what();
      throw runtime_error( mess );
    }
    return true;
  }

  bool tar::extract_ifstream( const string& name, ifstream& result ){
    result.close();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    pid_t pid = getpid();
    string tmpfile = "/tmp/ticc-tar-" + toString( pid ) + ".tmp";
    TAR local_tar_blob;
    TAR *local_tar = &local_tar_blob;
    int stat = tar_open( &local_tar, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( stat < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    stat = th_read(local_tar);
    while ( stat == 0  ) {
      if ( TH_ISREG( local_tar ) ){
	if ( string(local_tar->th_buf.name) == name ){
	  int res = tar_extract_regfile( local_tar, (char*)tmpfile.c_str() );
	  if ( res < 0 ){
	    cerr << "tar_extract_regfile(): " << strerror(errno) << endl;
	    return false;
	  }
	  result.open( tmpfile );
	  if ( result.good() ){
	    return true;
	  }
	  else {
	    cerr << "problem with " << tmpfile << endl;
	    return false;
	  }
	}
	else {
	  tar_skip_regfile( local_tar );
	}
      }
      stat = th_read( local_tar );
    }
    tar_close( local_tar );
    return false;
  }

  bool tar::next_ifstream( ifstream& result, string& name ){
    result.close();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    int stat;
    if ( !tar_file ){
      stat = tar_open( &tar_file, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
      if ( stat < 0 ){
	cerr << "tar_open(): " << strerror(errno) << endl;
	return false;
      }
    }
    pid_t pid = getpid();
    string tmpfile = "/tmp/ticc-tar-" + toString( pid ) + ".tmp";
    stat = th_read(tar_file);
    if ( stat < 0 ){
      cerr << "th_read(): " << strerror(errno) << endl;
      return false;
    }
    //    th_print( local_tar );
    while ( stat == 0  ) {
      if ( TH_ISREG( tar_file ) ){
	stat = tar_extract_regfile( tar_file, (char*)tmpfile.c_str() );
	if ( stat < 0 ){
	  cerr << "tar_extract_regfile(): " << strerror(errno) << endl;
	  return false;
	}
	result.open( tmpfile );
	if ( result.good() ){
	  name = tar_file->th_buf.name;
	  return true;
	}
	else {
	  cerr << "problem with " << tmpfile << endl;
	  return false;
	}
      }
      stat = th_read( tar_file );
    }
    return false;
  }

  bool tar::close() {
    if ( tar_file != 0 ){
      int res = tar_close( tar_file );
      if ( res < 0 ){
	cerr << "tar_close() failed" << endl;
	return false;
      }
    }
    return true;
  }
#endif

}
