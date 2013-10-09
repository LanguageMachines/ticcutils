#include <string>
#include <iostream>
#include <fstream>
#include "bzlib.h"
#include "ticcutils/zipper.h"
#include "ticcutils/bz2stream.h"
#include "ticcutils/gzstream.h"

using namespace std;

namespace TiCC {

  bool bz2Compress( const string& inName, const string& outName ){
    std::ifstream infile( inName.c_str(), std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() )
      outname = inName + ".bz2";
    std::ofstream outfile( outname.c_str(), std::ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outname << endl;
      return false;
    }
    bz2ostream bzout(outfile.rdbuf());
    bzout << infile.rdbuf();
  }

  bool bz2Decompress( const string& inName, const string& outName ){
    std::ifstream infile( inName.c_str(), std::ios::binary);
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
    std::ofstream outfile( outname.c_str(), std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    bz2istream bz2in(infile.rdbuf());
    outfile << bz2in.rdbuf();
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

  string bz2ReadFile( const string& inName ){
    string inname = inName;
    string::size_type pos = inname.rfind( ".bz2" );
    if ( pos == string::npos ){
      throw runtime_error( "bz2: expected an inputfile name with .bz2 extension" );
    }
    std::ifstream infile( inname.c_str(), std::ios::binary);
    if ( !infile ){
      throw runtime_error( "bz2: unable to open inputfile: " + inName );
    }
    return bz2ReadStream( infile );
  }

  bool bz2WriteStream( ostream& outfile, const string& buffer ){
    bz2ostream bzout(outfile.rdbuf());
    bzout << buffer;
    return true;
  }

  bool bz2WriteFile( const string& outName, const string& buffer ){
    std::ofstream outfile( outName.c_str(), std::ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outName << endl;
      return false;
    }
    bz2ostream bzout(outfile.rdbuf());
    bzout << buffer;
    return true;
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
    igzstream infile( inname.c_str(), ios::binary|ios::in );
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
    ogzstream outfile( outName.c_str(), ios::binary|ios::out );
    if ( !outfile ){
      cerr << "gz: unable to open outputfile: " << outName << endl;
      return false;
    }
    outfile << buffer;
    return true;
  }

  bool gzCompress( const string& inName, const string& outName ){
    ifstream infile( inName.c_str() );
    if ( !infile ){
      cerr << "gz: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() )
      outname = inName + ".gz";
    ogzstream outfile( outname.c_str(), ios::binary|ios::out );
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
  }

  bool gzDecompress( const string& inName, const string& outName ){
    igzstream infile( inName.c_str(), std::ios::binary|ios::in);
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
    std::ofstream outfile( outname.c_str(), std::ios::binary);
    if ( !infile ){
      cerr << "gz: unable to open inputfile: " << inName << endl;
      return false;
    }
    char c;
    while ( infile.get(c))
      outfile << c;
  }


}
