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

#include <cerrno>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "config.h"
#ifdef HAVE_BOOST_REGEX
#include <boost/regex.hpp>
#endif
#include "ticcutils/StringOps.h"
#include "ticcutils/FileUtils.h"

using namespace std;
namespace TiCC {

  vector<string> glob( const string& pat ){
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
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
      result.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return result;
  }

  bool isDir( const string& name ){
    // is 'name' a directory in sight ?
    struct stat st_buf;
    int status = stat( name.c_str(), &st_buf );
    if ( status < 0 )
      return false;
    return S_ISDIR (st_buf.st_mode);
  }

  bool isFile( const string& name ){
    // is 'name' a file in sight ?
    struct stat st_buf;
    int status = stat( name.c_str(), &st_buf );
    if ( status < 0 )
      return false;
    return S_ISREG (st_buf.st_mode);
  }

  void gatherFilesExt( const string& dirName, const string& ext,
		       vector<string>& result, bool recurse ){
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

#ifdef HAVE_BOOST_REGEX
  void gatherFilesMatch( const string& dirName, const boost::regex& match,
			 vector<string>& result, bool recurse ){
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
      else if ( boost::regex_search( name, match ) ){
	result.push_back( fullName );
      }
      entry = readdir( dir );
    }
    closedir( dir );
  }

  static string wildToRegExp( const string& wild ){
    // convert 'shell'-like wildcards into a regexp
    string result;
    for ( size_t i=0; i < wild.length(); ++i ){
      switch( wild[i] ){
      case '*':
	result += ".*";
	break;
      case '?':
	result += ".";
	break;
      case '.':
	result += "\\";
	result += wild[i];
	break;
      default:
	result += wild[i];
      }
    }
    //    cerr << "wild to regexp: " << wild << " ==> " << result << endl;
    return result;
  }

  vector<string> searchFilesMatch( const string& name,
				   const string& wild,
				   bool recurse ){
    vector<string> result;
    string reg = wildToRegExp( wild );
    try {
      boost::regex rx( reg );
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
	if ( boost::regex_search( fname, rx ) )
	  result.push_back( name );
	return result;
      }
      else if ( !isDir( name ) ){
	string mess = "TiCC::searchFilesMatch: the name '" + name
	  + "' doesn't match a file or directory.";
	throw runtime_error( mess );
      }
      gatherFilesMatch( name, rx, result, recurse );
    }
    catch( boost::regex_error& e ){
      string mess = "TiCC::searchFilesMatch: invalid regexp: ";
      mess += e.what();
      throw runtime_error( mess );
    }
    catch( ... ){
      throw;
    }
    return result;
  }
#else
  vector<string> searchFilesMatch( const string& name,
				   const string& match,
				   bool recurse ){
    cerr << "REGEXP support not available" << endl;
    cerr << "  attempting lame extension matching instead" << endl;
    return searchFilesExt( name, match, recurse );
  }
#endif

  bool createTruePath( const string& path ){
    // attempt to open a path /a/b/c/ from an expression like:
    // /a/b/c/
    // or ./a/b/c/ from expressions like
    // a/b/c/

    ofstream os1( path.c_str() );
    if ( !os1.good() ){
      // it fails
      // attempt to create the path
      vector<string> parts;
      int num = split_at( path, parts, "/" );
      if ( num > 0 ){
	//  at least one /
	string newpath;
	if ( path[0] == '/' )
	  newpath = "/";
	else
	  newpath = "./";
	for ( size_t i=0; i < parts.size(); ++i ){
	  newpath += parts[i] + "/";
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
    ofstream os( name.c_str() );
    if ( !os.good() ){
      return false;
    }
    return true;
  }

} // namespace TiCC
