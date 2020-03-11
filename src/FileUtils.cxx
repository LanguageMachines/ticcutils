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

#include "ticcutils/FileUtils.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "config.h"
#include <regex>
#include "ticcutils/StringOps.h"

using namespace std;
namespace TiCC {

  vector<string> glob( const string& pat ){
    ///  return a 'globbed; list of files from 'pat'
    vector<string> result;
    glob_t glob_result;
    int res = glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
    if ( res == GLOB_NOMATCH ){
      return result;
    }
    else if ( res != 0 ){
      string mess = "TiCC::glob: ";
      mess += strerror( errno );
      throw runtime_error( mess );
    }
    for( unsigned int i=0; i<glob_result.gl_pathc; ++i ){
      result.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return result;
  }

  bool isDir( const string& name ){
    /// is 'name' an accessible directory
    struct stat st_buf;
    int status = stat( name.c_str(), &st_buf );
    if ( status < 0 )
      return false;
    return S_ISDIR (st_buf.st_mode);
  }

  bool isFile( const string& name ){
    /// is 'name' an accessible file
    struct stat st_buf;
    int status = stat( name.c_str(), &st_buf );
    if ( status < 0 )
      return false;
    return S_ISREG (st_buf.st_mode);
  }

  void gatherFilesExt( const string& dirName, const string& ext,
		       vector<string>& result, bool recurse ){
    /// collect all files matching a certain extension
    /*!
      \param dirName path to search
      \param ext the extension to match
      \param result a list of matching filenames. New finds will be added
      \param recurse if true recurse into all subdirs
    */
    DIR *dir = opendir( dirName.c_str() );
    if ( !dir ){
      string mess = "TiCC::gatherFilesExt: unable to open dir " + dirName;
      throw runtime_error( mess );
    }
    struct dirent *entry = readdir( dir );
    while ( entry ){
      string name = entry->d_name;
      string fullName = dirName + "/" + name;
      if ( isDir( fullName ) ){
	if ( recurse && name[0] != '.' ){
	  gatherFilesExt( fullName, ext, result, recurse );
	}
      }
      else if ( ext.empty() ||
		TiCC::match_back( fullName, ext ) ){
	result.push_back( fullName );
      }
      entry = readdir( dir );
    }
    closedir( dir );
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
    vector<string> result;
    if ( isFile( name ) ){
      // it is just 1 file
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
    gatherFilesExt( name, ext, result, recurse );
    return result;
  }

  void gatherFilesMatch( const string& dirName, const regex& match,
			 vector<string>& result, bool recurse ){
    /// collect all files matching a regular expression
    /*!
      \param dirName path to search
      \param match a regular expressiin to match each file
      \param result a list of matching filenames. New finds will be added
      \param recurse if true recurse into all subdirs
    */
    DIR *dir = opendir( dirName.c_str() );
    if ( !dir ){
      string mess = "TiCC::gatherFilesMatch: unable to open dir " + dirName;
      throw runtime_error( mess );
    }
    struct dirent *entry = readdir( dir );
    while ( entry ){
      string name = entry->d_name;
      string fullName = dirName + "/" + name;
      if ( isDir( fullName ) ){
	if ( recurse && name[0] != '.' ){
	  gatherFilesMatch( fullName, match, result, recurse );
	}
      }
      else if ( regex_search( name, match ) ){
	result.push_back( fullName );
      }
      entry = readdir( dir );
    }
    closedir( dir );
  }

  static string wildToRegExp( const string& wild ){
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
    string reg = wildToRegExp( wild );
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
      gatherFilesMatch( name, rx, result, recurse );
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

  bool createTruePath( const string& path ){
    /// create a path using 'name'
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
    ofstream os1( path );
    if ( !os1.good() ){
      // it fails
      // attempt to create the path
      vector<string> parts = split_at( path, "/" );
      if ( parts.size() > 0 ){
	//  at least one /
	string newpath;
	if ( path[0] == '/' )
	  newpath = "/";
	else
	  newpath = "./";
	for ( auto const& p : parts ){
	  newpath += p + "/";
	  //	  cerr << "mkdir path = " << newpath << endl;
	  int status = mkdir( newpath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	  if ( status != 0 && errno != EEXIST ){
	    return false;
	  }
	}
      }
      return isDir( path );
    }
    return true;
  }

  bool createPath( const string& name ){
    /// create a path (directory OR file) using 'name'
    /*!
      \param name path description
      \return true if the file is created and available for writing
    */
    string path;
    string::size_type pos = name.rfind('/');
    if ( pos == name.length()-1 ){
      return createTruePath( name );
    }
    else if ( pos != string::npos ){
      path = name.substr( 0, pos+1 );
      if ( !createTruePath( path ) )
	return false;
    }
    ofstream os( name );
    if ( !os.good() ){
      return false;
    }
    return true;
  }

  string tempname( const string& label ){
    /// create a temporary directory
    /*!
      \param label a prefix to use
      \return the name of the created file
      The file will be added to /tmp/ with the label as the first part
      of the name, and 6 random characters added.
    */
    string path = "/tmp/" + label;
    string temp = path + "XXXXXX";
    char *filename = strdup(temp.c_str());
    if ( mkstemp(filename) < 0 ){
      throw runtime_error( "unable to create a temporary file under path="
			   + path );
    }
    //  cerr << "created temporary file: " << filename << endl;
    string result = filename;
    free( filename );
    return result;
  }

  string tempdir( const string& label ){
    /// create a temporary directory
    /*!
      \param label a prefix to use
      \return the name of the created directory
      The directory will be added to /tmp/ with the label as the first part
      of the name, and 6 random characters added.
    */
    string path = "/tmp/" + label;
    string temp = path + "XXXXXX";
    char *dirname = strdup(temp.c_str());
    char *bla = mkdtemp(dirname);
    if ( bla == 0 ){
      throw runtime_error( "unable to create a temporary dir under path="
			   + path );
    }
    //  cerr << "created temporary file: " << filename << endl;
    string result = dirname;

    free( dirname );
    return result;
  }

  void erase( const std::string& name ){
    /// remove a file
    int stat = std::remove( name.c_str() );
    if ( stat != 0 && errno != ENOENT ){
      throw runtime_error( "could not erase file/path '" + name + "': "
			   + strerror(errno) );
    }
  }

} // namespace TiCC
