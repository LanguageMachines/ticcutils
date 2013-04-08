#include <string>
#include <iostream>
#include <fstream>
#include "ticcutils/zipper.h"
#include "ticcutils/bz2stream.h"

using namespace std;

namespace TiCC {

  bool bz2compress( const string& inName, const string& outName ){
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

  bool bz2decompress( const string& inName, const string& outName ){
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

  string bz2readFile( const string& inName ){
    string inname = inName;
    string::size_type pos = inname.rfind( ".bz2" );
    if ( pos == string::npos ){
      throw runtime_error( "bz2: expected an inputfile name with .bz2 extension" );
    }
    std::ifstream infile( inname.c_str(), std::ios::binary);
    if ( !infile ){
      throw runtime_error( "bz2: unable to open inputfile: " + inName );
    }
    bz2istream bz2in(infile.rdbuf());
    istreambuf_iterator<char> inpos( bz2in.rdbuf() );
    istreambuf_iterator<char> endpos;
    string result;
    while ( inpos != endpos ){
      result += *inpos;
      ++inpos;
    }
    return result;
  }
  
  bool bz2writeFile( const string& outName, const string& buffer ){
    std::ofstream outfile( outName.c_str(), std::ios::binary);
    if ( !outfile ){
      cerr << "bz2: unable to open outputfile: " << outName << endl;
      return false;
    }
    bz2ostream bzout(outfile.rdbuf());
    bzout << buffer;
    return true;
  }

}
