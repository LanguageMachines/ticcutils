#include <string>
#include <iostream>
#include <fstream>
#include "ticcutils/zipper.h"
#include "ticcutils/bz2stream.hpp"

using namespace std;

namespace TiCC {

  bool BZ2compress( const string& inName, const string& outName ){
    std::ifstream infile( inName.c_str(), std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    string outname = outName;
    if ( outname.empty() )
      outname = inName + ".bz2";
    std::ofstream outfile( outname.c_str(), std::ios::binary);
    if ( !infile ){
      cerr << "bz2: unable to open inputfile: " << inName << endl;
      return false;
    }
    bz2ostream bzout(outfile.rdbuf());
    bzout << infile.rdbuf();
  }

  bool BZ2decompress( const string& inName, const string& outName ){
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
  
}
