#include <string>
#include "config.h"
#include <iostream>
#include <unistd.h>
#include <stdexcept>

#include "ticcutils/StringOps.h"
#include "ticcutils/PrettyPrint.h"
#include "ticcutils/zipper.h"
#include "ticcutils/Tar.h"
#include "ticcutils/Version.h"
#include "ticcutils/UnitTest.h"
#include "ticcutils/FileUtils.h"
#include "ticcutils/CommandLine.h"

using namespace std;
using namespace TiCC;

void helper(){
  throw runtime_error("fout");
}

void test_throw(){
  assertThrow( helper(), runtime_error );
  assertNoThrow( 4==7 );
}

void test_nothrow(){
  assertNoThrow( helper() );
  assertThrow( helper(), runtime_error );
}

void test_opts( CL_Options& opts ){
  startTestSerie( "we testen commandline opties." );
  string value;
  bool pol;
  opts.find( 't', value, pol );
  assertEqual( value, "true" );
  assertEqual( pol, true );
  opts.find( 'f', value, pol );
  assertEqual( value, "false" );
  assertEqual( pol, false );
  opts.find( "test", value );
  assertEqual( value, "test" );
  vector<string> mo = opts.getMassOpts();
  assertTrue( mo.size() == 3 );
  assertTrue( mo[0] == "blaat" );
  assertTrue( mo[2] == "arg2" );
}

void test_subtests_fail(){
  startTestSerie( "we testen subtests, met faal." );
  assertThrow( helper(), range_error );
  assertEqual( (1 + 2), (2 + 1) );
  assertEqual( 4, 5 );
}

void test_subtests_ok(){
  startTestSerie( "we testen subtests, allemaal OK." );
  assertThrow( helper(), runtime_error );
  assertEqual( 4, 4 );
  assertTrue( true );
}

void test_trim(){
  string val = " aha ";
  string res = trim(val);
  assertEqual( res, "aha" );
  assertEqual( "", trim(" \r ") );
  assertTrue( trim(" \r ").empty() );
  assertEqual( "A", trim("A") );
  assertEqual( "AHA", trim("AHA") );
  assertEqual( "AHA", trim("AHA\r\n") );
}

void test_trim_front(){
  string val = " aha ";
  string res = trim_front(val);
  assertEqual( res, "aha " );
}

void test_trim_back(){
  string val = " aha ";
  string res = trim_back(val);
  assertEqual( res, " aha" );
}

void test_match_front(){
  assertTrue( match_front("janklaassenenkatrien", "janklaassen" ) );
  assertFalse( match_front("janklaassenenkatrien", "anklaassen" ) );
}

void test_match_back(){
  assertTrue( match_back("janklaassenenkatrien", "katrien" ) );
  assertFalse( match_back("janklaassenenkatrien", "katrie" ) );
}

void test_format_non_ascii(){
  string val = "ø en €";
  string res = format_nonascii(val);
  assertEqual( res, "-0xffc3--0xffb8- en -0xffe2--0xff82--0xffac-" );
}

void test_split(){
  string line = "De kat krabt de krullen\n van de   trap.";
  vector<string> res;
  int cnt = split( line, res );
  //  cerr << "after split: " << res << endl;
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
}

void test_split_at(){
  string line = "Derarekatrarekrabtrarederarekrullen\nrarevanrarederaretrap.";
  vector<string> res;
  int cnt = split_at( line, res, "rare" );
  assertEqual( cnt, 8 );
  //  cerr << "after split: " << res << endl;
  assertEqual( res[5], "van" );
}

void test_split_at_first(){
  string line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<string> res;
  int cnt = split_at_first_of( line, res, ".,?!:;" );
  assertEqual( cnt, 8 );
  //  cerr << "after split: " << res << endl;
  assertEqual( res[5], "van" );
}

void test_to_upper(){
  string line = "Een CamelCapped Zin.";
  to_upper( line );
  assertEqual( line, "EEN CAMELCAPPED ZIN." );
}

void test_to_lower(){
  string line = "Een CamelCapped Zin.";
  to_lower( line );
  assertEqual( line, "een camelcapped zin." );
}

void test_uppercase(){
  string line = "Een CamelCapped Zin.";
  string res = uppercase( line );
  assertEqual( res, "EEN CAMELCAPPED ZIN." );
}

void test_lowercase(){
  string line = "Een CamelCapped Zin.";
  string res = lowercase( line );
  assertEqual( res, "een camelcapped zin." );
}

void test_bz2compression( const string& path ){
  assertTrue( bz2Compress( path + "small.txt", "bzout.bz2" ) );
  assertTrue( bz2Decompress( "bzout.bz2", "bzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = bz2ReadFile( "bzout.bz2" ) );
  assertEqual( buffer.substr(0,4), "This" );
  string cmd = "diff " + path + "small.txt bzout.txt";
  assertEqual( system( cmd.c_str() ), 0 );
}

void test_gzcompression( const string& path ){
  assertTrue( gzCompress( path + "small.txt", "gzout.gz" ) );
  assertTrue( gzDecompress( "gzout.gz", "gzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = gzReadFile( "gzout.gz" ) );
  assertEqual( buffer.substr(0,4), "This" );
  string cmd = "diff " + path + "small.txt gzout.txt";
  assertEqual( system(cmd.c_str()), 0 );
}

void test_tar( const string& path ){
  tar mytar;
  assertNoThrow( mytar.open( path + "test.tar" ) );
  vector<string> res;
  assertNoThrow( mytar.extract_file_names( res ) );
  //  cerr << res << endl;
  assertEqual( res.size(), 4 );
  ifstream tmp;
  string name;
  assertTrue( mytar.next_ifstream( tmp, name ) );
  assertEqual( name, "small.txt" );
  assertTrue( mytar.next_ifstream( tmp, name ) );
  assertTrue( mytar.next_ifstream( tmp, name ) );
  assertEqual( name, "sub1/sub.txt" );
  mytar.extract_ifstream( "sub1/sub.txt", tmp );
  string line;
  assertTrue( getline( tmp, line ) );
  assertEqual( line, "a testfile." );
  assertNoThrow( mytar.extract_file_names( res, ".xml" ) );
  assertEqual( res.size(), 1 );
#ifdef HAVE_BOOST_REGEX
  assertNoThrow( mytar.extract_file_names_match( res, "s*b" ) );
  assertEqual( res.size(), 3 );
#else
  assertNoThrow( mytar.extract_file_names_match( res, "txt" ) );
  assertEqual( res.size(), 4 );
#endif
}

void test_fileutils( const string& path ){
  vector<string> res;
  assertNoThrow( res = searchFilesExt( path, ".txt", false ) );
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFilesExt( path, ".txt" ) );
  assertEqual( res.size(), 2 );
  assertNoThrow( res = searchFilesExt( path+"small.txt", ".txt" ) );
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFiles( path+"small.txt" ) );
  assertEqual( res.size(), 1 );
  assertTrue( res[0] == path+"small.txt" );
  unlink(  "/tmp/test/silly/files/file" );
  assertTrue( createPath( "/tmp/test/silly/files/file" ) );
  unlink(  "/tmp/test/silly/" );
  assertTrue( createPath( "/tmp/test/silly/files/path/" ) );
  assertTrue( createPath( "/tmp/test/silly/files/path/raar" ) );
  assertFalse( createPath( "/tmp/test/silly/files/path/raar/sub" ) );

#ifdef HAVE_BOOST_REGEX
  assertNoThrow( res = searchFilesMatch( path, "*.txt", false ) );
  // non recursive. should match small.txt
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFilesMatch( path, "*.txt" ) );
  // recursive should match small.txt and sub1/sub.txt
  assertEqual( res.size(), 2 );
  assertNoThrow( res = searchFilesMatch( path, "small" ) );
  // should match small.txt
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFilesMatch( path, "s*[lb].txt" ) );
  // should match small.txt and sub1/sub.txt
  assertEqual( res.size(), 2 );
#endif
}

int main( const int argc, const char* argv[] ){
  cerr << BuildInfo() << endl;
  CL_Options opts( argc, argv );
  test_opts( opts );
  test_subtests_fail();
  test_subtests_ok();
  test_throw();
  test_nothrow();
  test_trim();
  test_trim_front();
  test_trim_back();
  test_match_front();
  test_match_back();
  test_format_non_ascii();
  test_split();
  test_split_at();
  test_split_at_first();
  test_to_upper();
  test_to_lower();
  test_uppercase();
  test_lowercase();
  string testdir;
  bool dummy;
  opts.find( 'd', testdir, dummy );
  test_bz2compression( testdir );
  test_gzcompression( testdir );
  test_tar( testdir );
  test_fileutils( testdir );
  summarize_tests(3);
}
