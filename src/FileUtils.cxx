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
#include <glob.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "ticcutils/StringOps.h"
#include "ticcutils/FileUtils.h"

using namespace std;
namespace TiCC {

  vector<string> glob( const string& pat ){
    glob_t glob_result;
    glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
    vector<string> ret;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
      ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
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

  vector<string> scanDir( const string& dirName, const string& ext, 
			  bool fullPath ){   
    vector<string> result;
    if ( !isDir( dirName ) ){
      cerr << "the name '" << dirName 
	   << "' doesn't seem te be a directory." << endl;
      exit(EXIT_FAILURE);
    }
    cout << "Searching dir '" << dirName << "' for *" << ext << endl;
    DIR *dir = opendir( dirName.c_str() );
    if ( !dir ){
      cerr << "unable to open dir" << dirName << endl;
      exit(EXIT_FAILURE);
    }
    struct stat sb;
    struct dirent *entry = readdir( dir );
    while ( entry ){
      if (entry->d_name[0] != '.') {
	string name = entry->d_name;
	if ( ext.empty() ){
	  if ( fullPath )
	    name = dirName  + "/" + name;
	  result.push_back( name );
	}
	else if ( TiCC::match_back( name, ext ) ){
	  if ( fullPath )
	    name = dirName  + "/" + name;
	  result.push_back( name );
	}
      }
      entry = readdir( dir );
    }
    closedir( dir );
    return result;
  } 

} // namespace TiCC
