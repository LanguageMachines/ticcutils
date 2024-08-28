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

#include "ticcutils/FileUtils.h"

#include <system_error>
#include <cassert>
#include <cstring> // for strdup()
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <regex>
#include "ticcutils/StringOps.h"

using namespace std;
namespace TiCC {

  bool isDir( const string& name ){
    /// check if 'name' is a directory
    filesystem::path the_path(name);
    return filesystem::is_directory(the_path);
  }

  bool isWritableDir( const string& name ){
    /// check if 'name' is a writable directory
    if ( isDir( name ) ){
      return access( name.c_str(), W_OK ) == 0;
    }
    return false;
  }

  bool isFile( const string& name ){
    /// check if 'name' an accessible file
    filesystem::path the_path(name);
    return filesystem::is_regular_file(the_path);
  }

  bool create_dir( const filesystem::path& p ){
    /// create a directory using 'name'
    /*!
      \param path the path description
      \return true if the path is created and a directory
      This function attempts to open a path /a/b/c/ from an expression like:
      \verbatim
      /a/b/c/
      ./a/b/c/
      a/b/c/
      \endverbatim
      It will recursively create all intermediate directories when needed
    */
    error_code ec;
    filesystem::create_directories( p, ec );
    if ( ec.value() != EEXIST
	 && ec.value() != 0 ){
      throw runtime_error( "create_dir(" + string(p) + ") failed: "
			   + ec.message() + " (" + std::to_string( ec.value())
			   + ")" );
    }
    return true;
  }

  bool createPath( const string& name ){
    /// create a path (directory OR file) using 'name'
    /*!
      \param name path description
      \return true if the file is created and available for writing
    */
    string::size_type pos = name.rfind('/');
    if ( pos == name.length()-1 ){
      // a directory for sure
      filesystem::path path(name);
      try {
	create_dir( path );
      }
      catch ( const exception& e ){
	cerr << e.what() << endl;
	return false;
      }
    }
    else if ( pos != string::npos ){
      // chop of the possible filename
      string dir_path = name.substr( 0, pos+1 );
      filesystem::path path(dir_path);
      try {
	create_dir( path );
      }
      catch ( const exception& e ){
	cerr << e.what() << endl;
	return false;
      }
      ofstream os( name );
      if ( !os.good() ){
	return false;
      }
    }
    return true;
  }

  void erase( const std::string& name ){
    /// remove a file
    filesystem::path p(name);
    error_code ec;
    if ( !filesystem::remove( p, ec ) ){
      // error OR file doesn't exist
      if ( ec.value() != 0 ) {
	// so a real error
	throw runtime_error( "could not erase file/path '" + name + "': "
			     + ec.message() );
      }
    }
  }

  vector<string> gather_files_ext( const string& dir_name,
				   const string& ext,
				   bool recurse ){
    vector<string> result;
    filesystem::path dir_path( dir_name );
    if ( recurse ){
      for ( const auto& entry : filesystem::recursive_directory_iterator(dir_path) ){
	auto p = entry.path();
	if ( ext.empty() ||
	     TiCC::match_back( p, ext ) ){
	  result.push_back( p );
	}
      }
    }
    else {
      for ( const auto& entry : filesystem::directory_iterator(dir_path) ){
	auto p = entry.path();
	if ( ext.empty() ||
	     TiCC::match_back( p, ext ) ){
	  result.push_back( p );
	}
      }
    }
    return result;
  }

  vector<string> searchFilesExt( const string& name,
				 const string& ext,
				 bool recurse ){
    /// collect all files with a given extension
    /*!
      \param name the files/path to search to search
      \param ext a file extension to match each file
      \param recurse if true recurse into all subdirs
      \return a list of matching filenames.
     */
    if ( isFile( name ) ){
      // it is just 1 file
      vector<string> result;
      if ( ext.empty() ||
	   TiCC::match_back( name, ext ) ){
	result.push_back( name );
      }
      return result;
    }
    else if ( !isDir( name ) ){
      string mess = "TiCC::searchFilesExt: the name '" + name
	+ "' doesn't match a file or directory.";
      throw runtime_error( mess );
    }
    return gather_files_ext( name, ext, recurse );
  }

  vector<string> gather_files_match( const string& dir_name,
				     const regex& match,
				     bool recurse ){
    /// collect all files matching a regular expression
    /*!
      \param dir_name path to search
      \param match a regular expression to match each file
      \param recurse if true recurse into all subdirs
      \return  a list of matching filenames.
    */
    vector<string> result;
    filesystem::path dir_path( dir_name );
    if ( recurse ){
      for ( const auto& entry : filesystem::recursive_directory_iterator(dir_path) ){
	string p = entry.path();
	if ( regex_search( p, match ) ){
	  result.push_back( p );
	}
      }
    }
    else {
      for ( const auto& entry : filesystem::directory_iterator(dir_path) ){
	string p = entry.path();
	if ( regex_search( p, match ) ){
	  result.push_back( p );
	}
      }
    }
    return result;
  }

  static string wild2regex( const string& wild ){
    // convert 'shell'-like wildcards into a regexp
    string result;
    for ( const auto& c : wild ){
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

  vector<string> searchFilesMatch( const string& name,
				   const string& wild,
				   bool recurse ){
    /// search all files matching a wildcard search
    /*!
      \param name a path/filename
      \param wild a wildcard pattern
      \param recurse if true, and name is a path, recurse into all subdirs
      \return a list of matching filenames
    */
    vector<string> result;
    string reg = wild2regex( wild );
    try {
      regex rx( reg );
      if ( isFile( name ) ){
	// it is just 1 file
	string::size_type pos = name.rfind( "/" );
	string fname;
	if ( pos != string::npos ){
	  fname = name.substr( pos+1 );
	}
	else {
	  fname = name;
	}
	if ( regex_search( fname, rx ) ){
	  result.push_back( name );
	}
	return result;
      }
      else if ( !isDir( name ) ){
	string mess = "TiCC::searchFilesMatch: the name '" + name
	  + "' doesn't match a file or directory.";
	throw runtime_error( mess );
      }
      result = gather_files_match( name, rx, recurse );
    }
    catch( regex_error& e ){
      string mess = "TiCC::searchFilesMatch: invalid regexp: ";
      mess += e.what();
      throw runtime_error( mess );
    }
    catch( ... ){
      throw;
    }
    return result;
  }

  string tempname( const string& label, const string& tmp_dir){
    /// create a temporary directory
    /*!
      \param label a prefix to use
      \param tmp_dir a path to use . (default /tmp)
      \return the name of the created file
      The file will be added to \e tmp_dir with the label as the first part
      of the name, and 6 random characters added.
    */
    string path = tmp_dir + "/" + label;
    string temp = path + "XXXXXX";
    char *filename = strdup(temp.c_str());

    int temp_file = mkstemp(filename);
    if ( temp_file < 0 ){
      throw runtime_error( "unable to create a temporary file under path="
			   + path );
    }
    //  cerr << "created temporary file: " << filename << endl;
    string result = filename;
    free( filename );
    // Prevent hitting open files limit in some cases
    close( temp_file );
    return result;
  }

  tmp_stream::tmp_stream( const string& prefix,
			  const string& tempdir,
			  bool keep ):
    _temp_name( TiCC::tempname( prefix, tempdir ) )
  {
    /// create a tmp_stream object
    /*!
      \param prefix a prefix for the name of a temporary file to create in
      in \e tempdir
      \param tempdir the directory to insert the file in. (default /tmp )
      \param keep when true, the file will be permanent. (e.g. for
      debugging). Default is \e false.
      An unique filename will be generated the prefix as the first part
      of the name, and 6 random characters added. It will be inserted in /tmp
      The file will be deleted on destruction of the tmp_stream object, except
      when keep is \e true
    */
    _os = new ofstream( _temp_name );
    _keep = keep;
  }

  tmp_stream::~tmp_stream(){
    /// destruct a tmp_stream object
    close();
    delete _os;
    if ( !_keep ){
      remove( _temp_name.c_str() );
    }
  }

} // namespace TiCC
