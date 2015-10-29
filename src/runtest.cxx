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
#include "ticcutils/Configuration.h"

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

void test_opts_basic(){
  startTestSerie( "we testen basic commandline opties." );
  CL_Options opts1( "t:fh" );
  // -t mist een optie
  assertThrow( opts1.init( "-t -f -h" ), OptionError );
  // onbekende optie
  assertThrow( opts1.init( "-a" ), OptionError );
  // -f heeft optie --> massOpts.
  assertNoThrow( opts1.init( "-t1 -f bla -h") );
  CL_Options opts2( "", "true:,false" );
  // --true mist een optie
  assertThrow( opts2.init( "--true --false"), OptionError );
  // --false heeft optie --> massOpts.
  assertNoThrow( opts2.init( "--true=1 --false 2")  );
  CL_Options opts3( "", "true:,false" );
  // - te weinig
  assertThrow( opts3.init( "-true=false"), OptionError );
  // onbekende optie
  assertThrow( opts3.init( "--magniet"), OptionError );
  CL_Options opts4( "", "true:,false" );
  // --true heeft optie, OK en
  // --false heeft geen optie --> massOpts.
  assertNoThrow( opts4.init( "--true 1 --false 2")  );
  string value;
  opts4.is_present( "true", value );
  assertEqual( value, "1" );
  CL_Options opts5( "", "false:,true::" );
  // --true heeft optionele optie,
  // --false heeft optie
  assertNoThrow( opts5.init( "--true --false 2")  );
  opts5.is_present( "true", value );
  assertEqual( value, "" );
  string lo6 = "false:,true::";
  CL_Options opts6( "", lo6 );
  assertEqual( opts6.get_long_options(), lo6 );
  // --true heeft optionele optie,
  // --false heeft optie
  assertNoThrow( opts6.init( "--true ok --false=6")  );
  opts6.is_present( "true", value );
  assertEqual( value, "ok" );
  opts6.is_present( "false", value );
  assertEqual( value, "6" );
  string so7 = "f:t::";
  CL_Options opts7( so7 );
  assertEqual( opts7.get_short_options(), so7 );
  // -t heeft optionele optie,
  // -f heeft optie
  assertNoThrow( opts7.init( "-t ok -f6")  );
  bool mood;
  opts7.is_present( 't', value, mood );
  assertEqual( value, "ok" );
  opts7.is_present( 'f', value, mood );
  assertEqual( value, "6" );
  CL_Options opts8( "t::,f:" );
  // -t heeft optionele optie,
  // -f heeft optie
  assertNoThrow( opts8.init( "-t -f6")  );
  opts8.is_present( 't', value, mood );
  assertEqual( value, "" );
  opts8.is_present( 'f', value, mood );
  assertEqual( value, "6" );
  CL_Options opts9( "t::qp:r:" );
  // -t heeft optionele optie. q is een stoorzender
  assertNoThrow( opts9.init( "-t 1 -t2 -t3 -q -t -t4 -p5 -r appel ")  );
  vector<string> ts;
  while ( opts9.extract( 't', value, mood ) ){
    ts.push_back( value );
  }
  assertEqual( ts.size() , 5 );
  assertEqual( ts[0], "1" );
  assertEqual( ts[1], "2" );
  assertEqual( ts[2], "3" );
  assertEqual( ts[3], "" );
  assertEqual( ts[4], "4" );
  assertTrue( opts9.is_present('q') );
  assertTrue( opts9.extract('q') );
  assertFalse( opts9.extract('q') );
  int myint = -1;
  assertTrue( opts9.is_present('p', myint ) );
  assertEqual( myint, 5 );
  myint = -1;
  assertTrue( opts9.extract('p', myint ) );
  assertEqual( myint, 5 );
  assertThrow( opts9.extract('r', myint ), OptionError );
  CL_Options opts10( "", "test::,qed,data:" );
  //  opts10.set_debug(true);
  // --test heeft optionele optie. qed is een stoorzender
  assertNoThrow( opts10.init( "--test 1 --test=2 --qed --test --test=3 --data=5.6 --data=appel")  );
  ts.clear();
  while ( opts10.extract( "test", value ) ){
    ts.push_back( value );
  }
  assertEqual( ts.size() , 4 );
  assertEqual( ts[0], "1" );
  assertEqual( ts[1], "2" );
  assertEqual( ts[2], "" );
  assertEqual( ts[3], "3" );
  assertTrue( opts10.is_present("qed") );
  assertTrue( opts10.extract("qed") );
  assertFalse( opts10.extract("q") );
  double mydouble = -3.14;
  assertTrue( opts10.is_present("data", mydouble ) );
  assertEqual( mydouble, 5.6 );
  mydouble = -3.14;
  assertTrue( opts10.extract("data", mydouble ) );
  assertEqual( mydouble, 5.6 );
  assertThrow( opts10.extract("data", mydouble ), OptionError );
  CL_Options opts11( "", "test:" );
  opts11.init( "--test=test/a arg1" );
  string ex;
  opts11.extract( "test", ex );
  assertEqual( ex, "test/a" );
  CL_Options opts12( "a:", "a:" );
  opts12.init( "-a 1 a --a=2 aa" );
  opts12.extract( 'a', ex );
  assertEqual( ex, "1" );
  opts12.extract( "a", ex );
  assertEqual( ex, "2" );
  vector<string> mo = opts12.getMassOpts();
  assertEqual( mo.size(), 2 );
  assertEqual( mo[0], "a" );
  assertEqual( mo[1], "aa" );
}

void test_opts( CL_Options& opts ){
  startTestSerie( "we testen nog meer commandline opties." );
  // opts.dump(cerr);
  // cerr << endl;
  string value;
  bool pol;
  opts.is_present( 't', value, pol );
  assertEqual( value, "true" );
  assertEqual( pol, true );
  opts.is_present( 'f', value, pol );
  assertEqual( value, "false" );
  assertEqual( pol, false );
  opts.is_present( 'd', value, pol );
  assertTrue( value != "" );
  opts.is_present( "test", value );
  assertEqual( value, "test" );
  opts.is_present( "raar", value );
  assertEqual( value, "" );
  vector<string> mo = opts.getMassOpts();
  assertTrue( mo.size() == 3 );
  assertEqual( mo[0], "blaat" );
  assertEqual( mo[1], "arg1" );
  assertEqual( mo[2], "arg2" );
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

void test_base_dir(){
  assertEqual( basename("/foo/bar" ), "bar" );
  assertEqual( dirname("/foo/bar" ), "/foo" );
  assertEqual( basename("foo/bar" ), "bar" );
  assertEqual( dirname("foo/bar" ), "foo" );
  assertEqual( basename("foobar" ), "foobar" );
  assertEqual( dirname("foobar" ), "." );
  assertEqual( basename("/" ), "/" );
  assertEqual( dirname("/" ), "/" );
  assertEqual( basename("." ), "." );
  assertEqual( dirname("." ), "." );
}

void test_bz2compression( const string& path ){
  assertTrue( bz2Compress( path + "small.txt", "bzout.bz2" ) );
  assertTrue( bz2Decompress( "bzout.bz2", "bzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = bz2ReadFile( "bzout.bz2" ) );
  assertEqual( buffer.substr(0,4), "This" );
  string cmd = "diff " + path + "small.txt bzout.txt";
  assertEqual( system( cmd.c_str() ), 0 );
  assertTrue( gzDecompress( path + "nasty.bz2", "nasty.txt" ) );
  cmd = "diff " + path + "nasty.bz2 nasty.txt";
  assertEqual( system(cmd.c_str()), 0 );
}

void test_gzcompression( const string& path ){
  assertTrue( gzCompress( path + "small.txt", "gzout.gz" ) );
  assertTrue( gzDecompress( "gzout.gz", "gzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = gzReadFile( "gzout.gz" ) );
  assertEqual( buffer.substr(0,4), "This" );
  string cmd = "diff " + path + "small.txt gzout.txt";
  assertEqual( system(cmd.c_str()), 0 );
  assertTrue( gzDecompress( path + "nasty.gz", "nasty.txt" ) );
  cmd = "diff " + path + "nasty.gz nasty.txt";
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
  assertNoThrow( mytar.extract_ifstream( "sub1/sub.txt", tmp ) );
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
  assertEqual( res.size(), 3 );
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

void test_configuration( const string& path ){
  Configuration c;
  assertTrue( c.fill( path + "testconfig.cfg" ) );
  assertTrue( c.hasSection("test") );
  string att = c.lookUp( "jan" );
  assertEqual( att, "gek" );
  att = c.lookUp( "piet" );
  assertEqual( att, "" );
  att = c.lookUp( "piet", "test" );
  assertEqual( att, "ook gek" );
  att = c.lookUp( "kees", "test" );
  assertEqual( att, "een jongen" );
  att = c.lookUp( "klara", "test" );
  assertEqual( att, "speciaal=raar" );
}

int main( const int argc, const char* argv[] ){
  cerr << BuildInfo() << endl;
  test_opts_basic();
  CL_Options opts1;
  opts1.set_short_options( "t:qf:d:" );
  opts1.set_long_options( "test:,raar" );
  opts1.init( argc, argv );
  test_opts( opts1 );
  CL_Options opts2( "t:qf:d:", "test:,raar" );
  opts2.init( "-ffalse +t true --test=test --raar  blaat -d iets arg1 -q arg2" );
  test_opts( opts2 );
  CL_Options opts3( "t:qf:d:", "test:,raar" );
  opts3.init( "-ffalse +t true --test=test --raar  blaat -d iets arg1 -q arg2" );
  test_opts( opts3 );
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
  opts1.is_present( 'd', testdir, dummy );
  test_bz2compression( testdir );
  test_gzcompression( testdir );
  test_tar( testdir );
  test_fileutils( testdir );
  test_configuration( testdir );
  summarize_tests(3);
}
