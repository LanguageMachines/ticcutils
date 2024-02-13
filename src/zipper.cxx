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

#include "ticcutils/zipper.h"

#include <stdexcept>
#include <fstream>
#include <iterator>
#include "ticcutils/bz2stream.h"
#include "ticcutils/gzstream.h"

using namespace std;

namespace TiCC {

  bool bz2Compress( const string& inName, const string& outName ){
    /// bz2 zip a file
    /*!
      \param inName the input file
      \param outName the bz2 zipped output file
      \return true on success, false otherwise
    */
    ifstream infile( inName, ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() ){
      outname = inName + ".bz2";
    }
    ofstream outfile( outname, ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outname << endl;
      return false;
    }
    bz2ostream bzout(outfile.rdbuf());
    bzout << infile.rdbuf();
    return true;
  }

  bool bz2Decompress( const string& inName, const string& outName ){
    /// bz2 unzip a file
    /*!
      \param inName the bz2 zipped input file
      \param outName the output file
      \return true on success, false otherwise
    */
    ifstream infile( inName, ios::binary);
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
	outname.resize( pos );
      }
    }
    ofstream outfile( outname, ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outname << endl;
      return false;
    }
    bz2istream bz2in(infile.rdbuf());
    outfile << bz2in.rdbuf();
    return true;
  }

  string bz2ReadStream( istream& is ){
    /// read a complete file from a bz2 stream
    /*!
      \param is the bz2 zipped input stream
      \return a string with the unzipped contents of the stream
    */
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
    /// read a complete file from a bz2 zipped file
    /*!
      \param in_name the bz2 zipped input file
      \return a string with the unzipped contents of the stream
    */
    string::size_type pos = in_name.rfind( ".bz2" );
    if ( pos == string::npos ){
      throw runtime_error( "bz2: expected an inputfile name with .bz2 extension, not '" + in_name + "'" );
    }
    ifstream infile( in_name, ios::binary);
    if ( !infile ){
      throw runtime_error( "bz2: unable to open inputfile: " + in_name );
    }
    return bz2ReadStream( infile );
  }

  bool bz2WriteStream( ostream& os, const string& buffer ){
    /// write a buffer to a bz2 stream
    /*!
      \param os the bz2 zipped output stream
      \param buffer the text to compress
      \return true on succes. May throw.
    */
    bz2ostream bzout(os.rdbuf());
    bzout << buffer;
    return true;
  }

  bool bz2WriteFile( const string& outName, const string& buffer ){
    /// write a buffer to a bz2 output file
    /*!
      \param outName the bz2 zipped output file
      \param buffer the text to compress
      \return true on succes, false on error. May throw.
    */
    ofstream outfile( outName, ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outName << endl;
      return false;
    }
    return bz2WriteStream( outfile, buffer );
  }

  string gzReadStream( istream& is ){
    /// read a complete file from a gz stream
    /*!
      \param is the gz zipped input stream
      \return the complete contents of the stream
    */
    string result;
    char c;
    while ( is.get(c) ){
      result += c;
    }
    return result;
  }

  string gzReadFile( const string& inName ){
    /// read a complete file from a gz zipped file
    /*!
      \param inName the gz zipped input file
      \return a string with the unzipped contents of the filr
    */
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

  bool gzWriteStream( ostream& os, const string& buffer ){
    /// write a buffer to a gz stream
    /*!
      \param os the bz2 zipped output stream
      \param buffer the text to compress
      \return true. May throw.
    */
    os << buffer;
    return true;
  }

  bool gzWriteFile( const string& outName, const string& buffer ){
    /// write a buffer to a gz file
    /*!
      \param outName the gz zipped output file
      \param buffer the text to compress
      \return true on succes, false on failure. May throw.
    */
    ogzstream outfile( outName, ios::binary|ios::out );
    if ( !outfile ){
      cerr << "gz: unable to open outputfile: " << outName << endl;
      return false;
    }
    return gzWriteStream( outfile, buffer );
  }

  bool gzCompress( const string& inName, const string& outName ){
    /// gz zip a file
    /*!
      \param inName the input file
      \param outName the gz zipped output file
      \return true on success, false otherwise
    */
    ifstream infile( inName );
    if ( !infile ){
      cerr << "gz: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() ){
      outname = inName + ".gz";
    }
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
    /// gz unzip a file
    /*!
      \param inName the gz zipped input file
      \param outName the unzipped output file
      \return true on success, false otherwise
    */
    igzstream infile( inName, ios::binary|ios::in);
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
	outname.resize( pos );
      }
    }
    ofstream outfile( outname, ios::binary);
    if ( !outfile ){
      cerr << "gz: unable to open outputfile: " << outName << endl;
      return false;
    }
    char c;
    while ( infile.get(c) ){
      outfile << c;
    }
    return true;
  }

}
