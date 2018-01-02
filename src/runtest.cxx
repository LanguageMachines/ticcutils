/*
  Copyright (c) 2006 - 2018
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
#include "ticcutils/LogStream.h"
#include "ticcutils/Unicode.h"

using namespace std;
using namespace TiCC;

void helper(){
  throw runtime_error("fout");
}

int helper2(){
  throw runtime_error("mis");
  return 8;
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
  CL_Options opts1;
  opts1.allow_args( "t:fh" );
  // -t mist een optie
  assertThrow( opts1.parse_args( "-t -f -h" ), OptionError );
  // onbekende optie
  assertThrow( opts1.init( "-a" ), OptionError );
  // -f heeft onterecht een parameter
  assertThrow( opts1.parse_args( "-t1 -f bla -h"), OptionError );
  // parameters aan einde ==> massopts.
  assertNoThrow( opts1.parse_args( "-t1 -f -h bla") );
  CL_Options opts2;
  opts2.allow_args( "", "true:,false" );
  // --true mist een optie
  assertThrow( opts2.parse_args( "--true --false"), OptionError );
  // --false heeft optie --> massOpts.
  assertNoThrow( opts2.parse_args( "--true=1 --false 2")  );
  CL_Options opts3;
  opts3.allow_args( "", "true:,false" );
  // - te weinig
  assertThrow( opts3.parse_args( "-true=false"), OptionError );
  // onbekende optie
  assertThrow( opts3.parse_args( "--magniet"), OptionError );
  CL_Options opts4;
  opts4.allow_args( "", "true:,false" );
  // --true heeft optie, OK en
  // --false heeft geen optie --> massOpts.
  assertNoThrow( opts4.parse_args( "--true 1 --false 2")  );
  string value;
  opts4.is_present( "true", value );
  assertEqual( value, "1" );
  CL_Options opts5( "", "false:,true::" );
  // --true heeft optionele optie,
  // --false heeft optie
  assertNoThrow( opts5.parse_args( "--true --false 2")  );
  opts5.is_present( "true", value );
  assertEqual( value, "" );
  string lo6 = "false:,true::";
  CL_Options opts6;
  opts6.allow_args( "", lo6 );
  assertEqual( opts6.get_long_options(), lo6 );
  // --true heeft optionele optie,
  // --false heeft optie
  assertNoThrow( opts6.parse_args( "--true ok --false=6")  );
  opts6.is_present( "true", value );
  assertEqual( value, "ok" );
  opts6.is_present( "false", value );
  assertEqual( value, "6" );
  string so7 = "f:t::";
  CL_Options opts7;
  opts7.allow_args( so7 );
  assertEqual( opts7.get_short_options(), so7 );
  // -t heeft optionele optie,
  // -f heeft optie
  assertNoThrow( opts7.parse_args( "-t ok -f6")  );
  bool mood;
  opts7.is_present( 't', value, mood );
  assertEqual( value, "ok" );
  opts7.is_present( 'f', value, mood );
  assertEqual( value, "6" );
  CL_Options opts8( "t::,f:", "" );
  // -t heeft optionele optie,
  // -f heeft optie
  assertNoThrow( opts8.parse_args( "-t -f6")  );
  opts8.is_present( 't', value, mood );
  assertEqual( value, "" );
  opts8.is_present( 'f', value, mood );
  assertEqual( value, "6" );
  CL_Options opts9;
  opts9.allow_args( "t::qp:r:" );
  // -t heeft optionele optie. q is een stoorzender
  assertNoThrow( opts9.parse_args( "-t 1 -t2 -t3 -q -t -t4 -p5 -r appel ")  );
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
  CL_Options opts10;
  opts10.allow_args( "", "test::,qed,data:" );
  //  opts10.set_debug(true);
  // --test heeft optionele optie. qed is een stoorzender
  assertNoThrow( opts10.parse_args( "--test 1 --test=2 --qed --test --test=3 --data=5.6 --data=appel")  );
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
  CL_Options opts11;
  opts11.allow_args( "", "test:" );
  opts11.parse_args( "--test=test/a arg1" );
  string ex;
  opts11.extract( "test", ex );
  assertEqual( ex, "test/a" );
  CL_Options opts12;
  opts12.allow_args( "a:", "a:" );
  opts12.parse_args( "-a 1 --a=2 a aa" );
  opts12.extract( 'a', ex );
  assertEqual( ex, "1" );
  opts12.extract( "a", ex );
  assertEqual( ex, "2" );
  vector<string> mo = opts12.getMassOpts();
  assertEqual( mo.size(), 2 );
  assertEqual( mo[0], "a" );
  assertEqual( mo[1], "aa" );
  CL_Options opts13;
  opts13.parse_args( "-a b -a c oke -dfiets --appel peer --fout=goed toch" );
  assertEqual( opts13.toString(), "-ab -ac -dfiets --appel=peer --fout=goed" );
  auto v = opts13.getMassOpts();
  assertEqual( v.size(), 2 );
  CL_Options opts14;
  opts14.set_debug(true);
  opts14.parse_args( "-a b -a c oke -d\"-fiets --appel peer \" --fout=goed toch" );
  assertEqual( opts14.toString(), "-ab -ac -d-fiets --appel peer  --fout=goed" );
  v = opts14.getMassOpts();
  assertEqual( v.size(), 2 );
  CL_Options opts15;
  opts15.set_debug(true);
  opts15.parse_args( "--fout=goed\\mis --jan=gek" );
  assertEqual( opts15.toString(), "--fout=goed\\mis --jan=gek" );
  string res;
  opts15.extract("fout", res );
  assertEqual( res, "goed\\mis" );
  opts15.extract("jan", res );
  assertEqual( res, "gek" );
  CL_Options opts16;
  opts16.allow_args( "", "test:" );
  opts16.set_debug(true);
  opts16.parse_args( "--test goed --test=prima --test niet=eens --test=wel=eens" );
  opts16.extract("test", res );
  assertEqual( res, "goed" );
  opts16.extract("test", res );
  assertEqual( res, "prima" );
  opts16.extract("test", res );
  assertEqual( res, "niet=eens" );
  opts16.extract("test", res );
  assertEqual( res, "wel=eens" );
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
  assertEqual( helper2(), 4 );
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
  res = trim(val,"a");
  assertEqual( res, " aha " );
  res = trim(val,"a ");
  assertEqual( res, "h" );
  res = trim( " \"test\""," \"");
  assertEqual( res, "test" );
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
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
  vector<string> res2 = split( line );
  assertEqual( res2.size(), 8 );
  assertEqual( res2[6], "de" );
  vector<string> res3 = split( line, 3 );
  assertEqual( res3.size(), 3 );
  assertEqual( res3[1], "kat" );
  assertEqual( res3[2], "krabt de krullen\n van de   trap." );
  vector<string> res4 = split( line, 24 );
  assertEqual( res4.size(), 8 );
  assertEqual( res4[1], "kat" );
  assertEqual( res4[2], "krabt" );
  vector<string> res5 = split( line, -1 );
  assertEqual( res5.size(), 8 );
  assertEqual( res5[1], "kat" );
  assertEqual( res5[2], "krabt" );
  vector<string> res6 = split( line, 0 );
  assertEqual( res6.size(), 8 );
  assertEqual( res6[1], "kat" );
  assertEqual( res6[2], "krabt" );
  vector<string> res7 = split( "APPELTAART", 2 );
  assertEqual( res7.size(), 1 );
  assertEqual( res7[0], "APPELTAART" );
}

void test_split_exact(){
  string line = "1 2  4    8  10";
  vector<string> res;
  int cnt = split( line, res, true );
  assertEqual( cnt, 10 );
  assertEqual( res[5], "" );
}

void test_split_at_exact(){
  string line = "1/2//4////8//10";
  vector<string> res;
  int cnt = split_at( line, res, "/", true );
  assertEqual( cnt, 10 );
  assertEqual( res[5], "" );
}

void test_split_at(){
  string line = "Derarekatrarekrabtrarederarekrullen\nrarevanrarederaretrap.";
  vector<string> res;
  int cnt = split_at( line, res, "rare" );
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
  vector<string> res2 = split_at( line, "rare" );
  assertEqual( res2.size(), 8 );
  assertEqual( res2[6], "de" );
  vector<string> res3 = split_at( line, "rare", 4 );
  assertEqual( res3.size(), 4 );
  assertEqual( res3[2], "krabt" );
  assertEqual( res3[3], "derarekrullen\nrarevanrarederaretrap." );
}

void test_split_at_first(){
  string line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<string> res;
  int cnt = split_at_first_of( line, res, ".,?!:;" );
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
  vector<string> res2 = split_at_first_of( line, ".,?!:;" );
  assertEqual( res2.size(), 8 );
  assertEqual( res2[6], "de" );
  vector<string> res3 = split_at_first_of( line, ".,?!:;", 7 );
  assertEqual( res3.size(), 7 );
  assertEqual( res3[4], "krullen" );
  assertEqual( res3[6], ",;.;de!trap." );
}

void test_split_at_first_exact(){
  string line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<string> res;
  int cnt = split_at_first_of( line, res, ".,?!:;", true );
  assertEqual( cnt, 13 );
  //  cerr << "after split: " << res << endl;
  assertEqual( res[5], "van" );
  assertEqual( res[9], "" );
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
  assertEqual( TiCC::basename("/foo/bar" ), "bar" );
  assertEqual( TiCC::dirname("/foo/bar" ), "/foo" );
  assertEqual( TiCC::basename("foo/bar" ), "bar" );
  assertEqual( TiCC::dirname("foo/bar" ), "foo" );
  assertEqual( TiCC::basename("foobar" ), "foobar" );
  assertEqual( TiCC::dirname("foobar" ), "." );
  assertEqual( TiCC::basename("/" ), "" );
  assertEqual( TiCC::dirname("/" ), "" );
  assertEqual( TiCC::basename("." ), "." );
  assertEqual( TiCC::dirname("." ), "." );
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
  assertTrue( getline( tmp, line ).good() );
  assertEqual( line, "a testfile." );
  assertNoThrow( mytar.extract_file_names( res, ".xml" ) );
  assertEqual( res.size(), 1 );
  assertNoThrow( mytar.extract_file_names_match( res, "s*b" ) );
  assertEqual( res.size(), 3 );
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
  string oud = c.setatt( "jan", "normaal" );
  assertEqual( oud, "gek" );
  c.setatt( "pief", "paf", "poef" );
  c.clearatt( "piet", "test" );
  att = c.lookUp( "Truus", "test" );
  assertEqual( att, "met een \n er in en een \r!" );
  assertNoThrow( c.create_configfile( "/tmp/test.cfg" ) );
  Configuration c2;
  assertTrue( c2.fill( "/tmp/test.cfg" ) );
  att = c2.lookUp( "jan" );
  assertEqual( att, "normaal" );
  att = c2.lookUp( "klara", "test" );
  assertEqual( att, "speciaal=raar" );
  att = c2.lookUp( "kees", "test" );
  assertEqual( att, "een jongen" );
  att = c2.lookUp( "pief", "poef" );
  assertEqual( att, "paf" );
  att = c2.lookUp( "piet", "test" );
  assertEqual( att, "" );
  att = c.lookUp( "Truus", "test" );
  assertEqual( att, "met een \n er in en een \r!" );
  Configuration c3;
  assertTrue( c3.fill( path + "testconfig.cfg" ) );
  att = c3.lookUp( "jan" );
  assertEqual( att, "gek" );
  att = c3.lookUp( "pief", "poef" );
  assertEqual( att, "" );
  att = c3.lookUp( "piet", "test" );
  assertEqual( att, "ook gek" );
  c3.merge( c2 ); //default is NOT to override what is already there
  att = c3.lookUp( "jan" );
  assertEqual( att, "gek" ); // not overridden ?
  att = c3.lookUp( "pief", "poef" );
  assertEqual( att, "paf" );
  att = c3.lookUp( "piet", "test" ); // empty in c2, not changed in c3
  assertEqual( att, "ook gek" );
  c3.merge( c2, true ); // Now override
  att = c3.lookUp( "jan" );
  assertEqual( att, "normaal" ); // is overridden now?
  att = c3.lookUp( "piet", "test" ); // empty in c2, still not changed in c3
  assertEqual( att, "ook gek" );
}

void test_logstream( const string& path ){
  ofstream uit( "/tmp/testls.1" );
  LogStream ls( uit );
  ls.setstamp( NoStamp );
  *Log( ls ) << "test 1 level=" << ls.getlevel() << " threshold=" << ls.getthreshold() << endl;
  *Dbg( ls ) << "debug 1" << endl;
  *xDbg( ls ) << "x_debug 1" << endl;
  *xxDbg( ls ) << "xx_debug 1" << endl;
  ls.setlevel( LogSilent );
  *Log( ls ) << "test 2 level=" << ls.getlevel() << " threshold=" << ls.getthreshold() << endl;
  *Dbg( ls ) << "debug 2" << endl;
  *xDbg( ls ) << "x_debug 2" << endl;
  *xxDbg( ls ) << "xx_debug 2" << endl;
  ls.setlevel( LogDebug );
  *Log( ls ) << "test 3 level=" << ls.getlevel() << " threshold=" << ls.getthreshold() << endl;
  *Dbg( ls ) << "debug 3" << endl;
  *xDbg( ls ) << "x_debug 3" << endl;
  *xxDbg( ls ) << "xx_debug 3" << endl;
  ls.setlevel( LogExtreme );
  *Log( ls ) << "test 4 level=" << ls.getlevel() << " threshold=" << ls.getthreshold() << endl;
  *Dbg( ls ) << "debug 4" << endl;
  *xDbg( ls ) << "x_debug 4" << endl;
  *xxDbg( ls ) << "xx_debug 4" << endl;
  ls.setlevel( LogHeavy );
  *Log( ls ) << "test 5 level=" << ls.getlevel() << " threshold=" << ls.getthreshold() << endl;
  *Dbg( ls ) << "debug 5" << endl;
  *xDbg( ls ) << "x_debug 5" << endl;
  *xxDbg( ls ) << "xx_debug 5" << endl;
  string cmd = "diff /tmp/testls.1 " + path + "testls.1.ok";
  assertEqual( system( cmd.c_str() ), 0 );
}

void test_unicode( const string& path ){
  UnicodeString u1 = L'私';
  UChar32 uc1 = U'\U00007981';
  UChar32 uc2 = U'\U00007982';
  UnicodeString u2 = uc1;
  u2 += UnicodeString( uc2 );
  string s1 = UnicodeToUTF8( u1 );
  assertEqual( s1 , "私" );
  string s2 = UnicodeToUTF8( u2 );
  assertEqual( s2 , "禁禂" );
  ifstream in( path + "utf16bom.nl" );
  string line;
  getline( in, line );
  assertFalse( line == "Hier staat een BOM voor. æ en ™ om te testen." );
  UnicodeString u3 = UnicodeFromEnc( line, "UTF16" );
  string s3 = UnicodeToUTF8(  u3 );
  assertEqual( s3, "Hier staat een BOM voor. æ en ™ om te testen." );
}

int main( const int argc, const char* argv[] ){
  cerr << BuildInfo() << endl;
  test_opts_basic();
  CL_Options opts1;
  opts1.set_short_options( "t:qf:d:" );
  opts1.set_long_options( "test:,raar" );
  opts1.init( argc, argv );
  test_opts( opts1 );
  CL_Options opts2;
  opts2.allow_args( "t:qf:d:", "test:,raar" );
  opts2.parse_args( "-ffalse +t true --test=test -d iets -q --raar blaat arg1 arg2" );
  test_opts( opts2 );
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
  test_split_exact();
  test_split_at();
  test_split_at_exact();
  test_split_at_first();
  test_split_at_first_exact();
  test_to_upper();
  test_to_lower();
  test_uppercase();
  test_lowercase();
  string testdir;
  bool dummy;
  opts1.is_present( 'd', testdir, dummy );
#ifdef HAVE_BZLIB_H
  test_bz2compression( testdir );
#else
  cerr << "no BZ2 support available" << endl;
#endif
#ifdef HAVE_LIBZ
  test_gzcompression( testdir );
#else
  cerr << "no GZ support available" << endl;
#endif
#ifdef HAVE_LIBTAR_H
  test_tar( testdir );
#else
  cerr << "no tar support available" << endl;
#endif
  test_base_dir();
  test_fileutils( testdir );
  test_configuration( testdir );
  test_logstream( testdir );
  test_unicode( testdir );
  summarize_tests(4);
}
