#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "config.h"
#include "libtar.h"
#include <sys/fcntl.h>
#include "ticcutils/StringOps.h"
#ifdef HAVE_BOOST_REGEX
#include <boost/regex.hpp>
#endif
#include "ticcutils/Tar.h"

using namespace std;

namespace TiCC {

  tar::tar(){
    tar_file = 0;
  }

  tar::~tar(){
    close();
  }

  bool tar::open( const string& name ){
    if ( !tarname.empty() ){
      cerr << "tar already opened!" << endl;
      return false;
    }
    TAR *tar_file;
    int res = tar_open( &tar_file, (char*)name.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( res < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    tar_close( tar_file );
    tarname = name;
    return true;
  }

  bool tar::extract_file_names( vector<string>& result,
				const string& ext ){
    result.clear();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    TAR local_tar_blob;
    TAR *local_tar = &local_tar_blob;
    int stat = tar_open( &local_tar, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( stat < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    stat = th_read( local_tar );
    while ( stat == 0  ) {
      if ( TH_ISREG( local_tar ) ){
	string name = local_tar->th_buf.name;
	if ( ext.empty() ||
	     name.rfind( ext ) != string::npos ){
	  result.push_back( name );
	}
	tar_skip_regfile( local_tar );
      }
      stat = th_read( local_tar );
    }
    tar_close( local_tar );
    return true;
  }

#ifdef HAVE_BOOST_REGEX
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

  bool tar::extract_file_names_match( vector<string>& result,
				      const string& wild ){
    result.clear();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    TAR local_tar_blob;
    TAR *local_tar = &local_tar_blob;
    int stat = tar_open( &local_tar, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( stat < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    string pat = wildToRegExp( wild );
    try {
      boost::regex rx( pat );
      stat = th_read( local_tar );
      while ( stat == 0  ) {
	if ( TH_ISREG( local_tar ) ){
	  string name = local_tar->th_buf.name;
	  if ( boost::regex_search( name, rx ) ){
	    result.push_back( name );
	  }
	  tar_skip_regfile( local_tar );
	}
	stat = th_read( local_tar );
      }
      tar_close( local_tar );
    }
    catch( boost::regex_error& e ){
      cerr << "invalid regexp: " << e.what() << endl;
      exit(EXIT_FAILURE);
    }
    return true;
  }
#else
  bool tar::extract_file_names_match( vector<string>& result,
				      const string& pat ){
    cerr << "tar::extract() REGEXP support not available" << endl;
    cerr << "  attempting lame extension matching instead" << endl;
    return extract_file_names( result, pat );
  }
#endif

  bool tar::extract_ifstream( const string& name, ifstream& result ){
    result.close();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    pid_t pid = getpid();
    string tmpfile = "/tmp/ticc-tar-" + toString( pid ) + ".tmp";
    TAR local_tar_blob;
    TAR *local_tar = &local_tar_blob;
    int stat = tar_open( &local_tar, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
    if ( stat < 0 ){
      cerr << "tar_open(): " << strerror(errno) << endl;
      return false;
    }
    stat = th_read(local_tar);
    while ( stat == 0  ) {
      if ( TH_ISREG( local_tar ) ){
	if ( string(local_tar->th_buf.name) == name ){
	  int res = tar_extract_regfile( local_tar, (char*)tmpfile.c_str() );
	  if ( res < 0 ){
	    cerr << "tar_extract_regfile(): " << strerror(errno) << endl;
	    return false;
	  }
	  result.open(tmpfile.c_str() );
	  if ( result.good() ){
	    return true;
	  }
	  else {
	    cerr << "problem with " << tmpfile << endl;
	    return false;
	  }
	}
	else {
	  tar_skip_regfile( local_tar );
	}
      }
      stat = th_read( local_tar );
    }
    tar_close( local_tar );
    return false;
  }

  bool tar::next_ifstream( ifstream& result, string& name ){
    result.close();
    if ( tarname.empty() ){
      cerr << "no tar opened yet" << endl;
      return false;
    }
    int stat;
    if ( !tar_file ){
      stat = tar_open( &tar_file, (char*)tarname.c_str(), 0, O_RDONLY, 0, TAR_GNU );
      if ( stat < 0 ){
	cerr << "tar_open(): " << strerror(errno) << endl;
	return false;
      }
    }
    pid_t pid = getpid();
    string tmpfile = "/tmp/ticc-tar-" + toString( pid ) + ".tmp";
    stat = th_read(tar_file);
    if ( stat < 0 ){
      cerr << "th_read(): " << strerror(errno) << endl;
      return false;
    }
    //    th_print( local_tar );
    while ( stat == 0  ) {
      if ( TH_ISREG( tar_file ) ){
	stat = tar_extract_regfile( tar_file, (char*)tmpfile.c_str() );
	if ( stat < 0 ){
	  cerr << "tar_extract_regfile(): " << strerror(errno) << endl;
	  return false;
	}
	result.open(tmpfile.c_str() );
	if ( result.good() ){
	  name = tar_file->th_buf.name;
	  return true;
	}
	else {
	  cerr << "problem with " << tmpfile << endl;
	  return false;
	}
      }
      stat = th_read( tar_file );
    }
    return false;
  }

  bool tar::close() {
    if ( tar_file != 0 ){
      int res = tar_close( tar_file );
      if ( res < 0 ){
	cerr << "tar_close() failed" << endl;
	return false;
      }
    }
    return true;
  }
}
