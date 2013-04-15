/*
  $Id$
  $URL$

  Copyright (c) 1998 - 2013
  ILK   - Tilburg University
  CLiPS - University of Antwerp
 
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
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/
#include <cerrno>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
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
      cerr << "TiCC::glob:" << strerror( errno ) << endl;
      exit(EXIT_FAILURE);
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
    return S_ISDIR (st_buf.st_mode);
  }

  bool isFile( const string& name ){
    // is 'name' a file in sight ?
    struct stat st_buf;
    int status = stat( name.c_str(), &st_buf );
    return S_ISREG (st_buf.st_mode);
  }

  void gatherFiles( const string& dirName, const string& ext, 
		    vector<string>& result, bool recurse ){   
    DIR *dir = opendir( dirName.c_str() );
    if ( !dir ){
      cerr << "unable to open dir" << dirName << endl;
      exit(EXIT_FAILURE);
    }
    struct stat sb;
    struct dirent *entry = readdir( dir );
    while ( entry ){
      string name = entry->d_name;
      string fullName = dirName + "/" + name;
      if ( isDir( fullName ) ){
	if ( recurse && name[0] != '.' ){
	  gatherFiles( fullName, ext, result, recurse );
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
      cerr << "the name '" << name 
	   << "' doesn't match a file or directory." << endl;
      exit(EXIT_FAILURE);
    }
    gatherFiles( name, ext, result, recurse );
    return result;
  } 

} // namespace TiCC
