#include <string>
#include <iostream>
#include <stdexcept>

#include "ticcutils/StringOps.h"
#include "ticcutils/PrettyPrint.h"
#include "ticcutils/zipper.h"

#include "ticcutils/UnitTest.h"

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

void test_bz2compression(){
  assertTrue( bz2compress( "small.txt", "bzout.bz2" ) );
  assertTrue( bz2decompress( "bzout.bz2", "bzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = bz2readFile( "bzout.bz2" ) );
  assertEqual( buffer.substr(0,4), "This" );
  assertEqual( system("diff small.txt bzout.txt"), 0 );
}

int main(){
  test_subtests_fail();
  test_subtests_ok();
  test_throw();
  test_nothrow();
  test_trim();
  test_trim_front();
  test_trim_back();
  test_format_non_ascii();
  test_split();
  test_split_at();
  test_split_at_first();
  test_to_upper();
  test_to_lower();
  test_uppercase();
  test_lowercase();
  test_bz2compression();
  summarize_tests(3);
}
