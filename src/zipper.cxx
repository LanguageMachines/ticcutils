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

#include "ticcutils/zipper.h"

#include <stdexcept>
#include <fstream>
#include "config.h"
#include "bzlib.h"
#include "ticcutils/bz2stream.h"
#include "ticcutils/gzstream.h"

using namespace std;

namespace TiCC {

  bool bz2Compress( const string& inName, const string& outName ){
    std::ifstream infile( inName, std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() )
      outname = inName + ".bz2";
    std::ofstream outfile( outname, std::ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outname << endl;
      return false;
    }
    bz2ostream bzout(outfile.rdbuf());
    bzout << infile.rdbuf();
    return true;
  }

  bool bz2Decompress( const string& inName, const string& outName ){
    std::ifstream infile( inName, std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() ){
      outname = inName;
      string::size_type pos = outname.rfind( ".bz2" );
      if ( pos == string::npos ){
	cerr << "bz2: expected an inputfile name with .bz2: " << inName << endl;
	return false;
      }
      else {
	outname = outname.substr( 0, pos );
      }
    }
    std::ofstream outfile( outname, std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    bz2istream bz2in(infile.rdbuf());
    outfile << bz2in.rdbuf();
    return true;
  }

  string bz2ReadStream( istream& is ){
    bz2istream bz2in(is.rdbuf());
    istreambuf_iterator<char> inpos( bz2in.rdbuf() );
    istreambuf_iterator<char> endpos;
    string result;
    while ( inpos != endpos ){
      result += *inpos;
      ++inpos;
    }
    return result;
  }

  string bz2ReadFile( const string& in_name ){
    string::size_type pos = in_name.rfind( ".bz2" );
    if ( pos == string::npos ){
      throw runtime_error( "bz2: expected an inputfile name with .bz2 extension, not '" + in_name + "'" );
    }
    std::ifstream infile( in_name, std::ios::binary);
    if ( !infile ){
      throw runtime_error( "bz2: unable to open inputfile: " + in_name );
    }
    return bz2ReadStream( infile );
  }

  bool bz2WriteStream( ostream& outfile, const string& buffer ){
    bz2ostream bzout(outfile.rdbuf());
    bzout << buffer;
    return true;
  }

  bool bz2WriteFile( const string& outName, const string& buffer ){
    std::ofstream outfile( outName, std::ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outName << endl;
      return false;
    }
    return bz2WriteStream( outfile, buffer );
  }

  string gzReadStream( istream& is ){
    string result;
    char c;
    while ( is.get(c) )
      result += c;
    return result;
  }

  string gzReadFile( const string& inName ){
    string inname = inName;
    string::size_type pos = inname.rfind( ".gz" );
    if ( pos == string::npos ){
      throw runtime_error( "gz: expected an inputfile name with .gz extension" );
    }
    igzstream infile( inname, ios::binary|ios::in );
    if ( !infile ){
      throw runtime_error( "gz: unable to open inputfile: " + inName );
    }
    return gzReadStream( infile );
  }

  bool gzWriteStream( ostream& outfile, const string& buffer ){
    outfile << buffer;
    return true;
  }

  bool gzWriteFile( const string& outName, const string& buffer ){
    ogzstream outfile( outName, ios::binary|ios::out );
    if ( !outfile ){
      cerr << "gz: unable to open outputfile: " << outName << endl;
      return false;
    }
    return gzWriteStream( outfile, buffer );
  }

  bool gzCompress( const string& inName, const string& outName ){
    ifstream infile( inName );
    if ( !infile ){
      cerr << "gz: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() )
      outname = inName + ".gz";
    ogzstream outfile( outname, ios::binary|ios::out );
    if ( !outfile ){
      cerr << "gz: unable to open outputfile: " << outname << endl;
      return false;
    }
    char c;
    while ( infile.get(c) ){
      outfile << c;
    }
    infile.close();
    outfile.flush();
    outfile.close();
    return true;
  }

  bool gzDecompress( const string& inName, const string& outName ){
    igzstream infile( inName, std::ios::binary|ios::in);
    if ( !infile ){
      cerr << "gz: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() ){
      outname = inName;
      string::size_type pos = outname.rfind( ".gz" );
      if ( pos == string::npos ){
	cerr << "gz: expected an inputfile name with .gz: " << inName << endl;
	return false;
      }
      else {
	outname = outname.substr( 0, pos );
      }
    }
    std::ofstream outfile( outname, std::ios::binary);
    if ( !outfile ){
      cerr << "gz: unable to open outputfile: " << outName << endl;
      return false;
    }
    char c;
    while ( infile.get(c))
      outfile << c;
    return true;
  }

}
