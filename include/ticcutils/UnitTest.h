/*
  $Id$
  $URL$

  Copyright (c) 1998 - 2015
  ILK   - Tilburg University
  CLiPS - University of Antwerp

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
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/

#ifndef TICC_UNITTEST_H
#define TICC_UNITTEST_H

#include <cstdlib>

const std::string OK = "\033[1;32m OK  \033[0m";
const std::string FAIL = "\033[1;31m  FAILED  \033[0m";

class MyTSerie {
 public:
  MyTSerie( const std::string& fun, int lineno, const std::string& line ){
    start( fun, lineno, line );
  }
  ~MyTSerie(){
    stop( _fun, 0 );
  }
  bool isDefault() const {return _fun =="default"; };
  int _fails;
  int _tests;
  int _start_line;
  std::string _fun;
 private:
  void start( const std::string& fun, int lineno, const std::string& line ){
    _fun = fun;
    _fails = 0;
    _tests = 0;
    _start_line = lineno;
    if ( !isDefault() )
      std::cout << "Serie:\t" << fun << " (" << line << ")" << std::endl;
  };
  void stop( const std::string& fun, int line );
};

static MyTSerie currentTestContext( "default", 0, "default" );

int exit_status = 0;
bool summarized = false;
bool testSilent = false;

#define TEST_SILENT_ON() testSilent = true;
#define TEST_SILENT_OFF() testSilent = false;

inline void summarize_tests( int expected=0 ){
  summarized = true;
  std::cout << "TiCC tests performed " << currentTestContext._tests
	    << " tests, with " << currentTestContext._fails << " failures.";
  int diff = currentTestContext._fails - expected;
  if ( diff > 0 ){
    std::cout << " We expected " << expected << " failures." << std::endl;
    std::cout << "overall " << FAIL << std::endl;
  }
  else if ( diff < 0 ){
    std::cout << " This is less than the " << expected << " we expected." << std::endl;
    std::cout << "overall: " << FAIL << std::endl;
  }
  else {
    std::cout << " that was what we expected." << std::endl;
    std::cout << "overall: " << OK << std::endl;
  }
  exit_status = diff;
}

void MyTSerie::stop( const std::string& fun, int ){
  if ( isDefault() ){
    if ( !summarized ){
      summarize_tests( 0 );
    }
    exit( exit_status );
  }
  else {
    currentTestContext._tests += _tests;
    if ( _fails ){
      std::cout << "\t" << fun << "(): " << _fails
		<< " out of " << _tests << " tests" << FAIL << std::endl;
      currentTestContext._fails += _fails;
    }
    else
      std::cout << "\t" << fun << "(): all " << _tests << " tests" << OK << std::endl;
  };
}

#define assertEqual( XX , YY ) test_eq<typeof XX, typeof YY>( __func__, __LINE__, (XX), (YY), currentTestContext )
#define assertThrow( XX, EE )						\
  do { 									\
    ++currentTestContext._tests;					\
    if ( !testSilent && currentTestContext.isDefault() )		\
      std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch( const EE& ){							\
      if (  !testSilent && currentTestContext.isDefault() )		\
	std::cerr << OK << std::endl;					\
      break;								\
    }									\
    catch ( const std::exception& e ){					\
      ++currentTestContext._fails;					\
      if ( currentTestContext.isDefault() )				\
	std::cout << FAIL << std::endl;					\
      else								\
	std::cerr << "\t";						\
      std::cerr << __func__ << "(" << __LINE__ << ") : wrong exception, what='" << e.what() << "'" << std::endl; \
      break;								\
    }									\
    ++currentTestContext._fails;					\
    if ( currentTestContext.isDefault() )				\
      std::cout << FAIL << std::endl;					\
    else								\
      std::cerr << "\t";						\
    std::cerr << __func__ << "(" << __LINE__ << ") : no exception thrown" << std::endl; \
  }									\
  while( false )

#define assertNoThrow( XX )						\
  do { 									\
    ++currentTestContext._tests;					\
    if (  !testSilent && currentTestContext.isDefault() )		\
      std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch ( const std::exception& e ){					\
      ++currentTestContext._fails;					\
      if ( currentTestContext.isDefault() )				\
	std::cout << FAIL << std::endl;					\
      else								\
	std::cerr << "\t";						\
      std::cerr << __func__ << "(" << __LINE__ << ") error: '" << e.what() << "'" << std::endl; \
      break;								\
    }									\
    if (  !testSilent && currentTestContext.isDefault() )		\
      std::cout << OK << std::endl;					\
  }									\
  while( false )

#define assertTrue( YY ) 			                        \
  try {                                                                 \
    test_true( __func__, __LINE__, (YY), currentTestContext );		\
  }									\
  catch( const std::exception& e ){					\
    std::cerr << __func__ << "(" << __LINE__ << ") error: '" << e.what() << "'" << std::endl; \
  }

#define assertFalse( YY )						\
  try {									\
  test_false( __func__, __LINE__, (YY), currentTestContext );		\
  }									\
  catch( const std::exception& e ){					\
    std::cerr << __func__ << "(" << __LINE__ << ") error:'" << e.what() << "'" << std::endl; \
  }

#define assertMessage( MM, YY )						\
    try {								\
      test_true_message( __func__, __LINE__, (MM), (YY), currentTestContext ); \
    }									\
    catch( const std::exception& e ){					\
      std::cerr << __func__ << "(" << __LINE__ << ") error: '" << e.what() << "'" << std::endl; \
    }


#define startTestSerie( SS ) MyTSerie currentTestContext( __func__, __LINE__, (SS) )

template <typename T1, typename T2>
  inline void test_eq( const char* F, int L,
		       const T1& s1, const T2& s2, MyTSerie& T ){
  if ( !testSilent && T.isDefault() )
    std::cout << "test: " << F << "(" << L << "): ";
  ++T._tests;
  if ( s1 != s2 ){
    ++T._fails;
    if ( T.isDefault() )
      std::cout << FAIL << std::endl;
    else
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '" << s1 << "' != '"
	      << s2 << "'" << std::endl;
  }
  else {
    if ( !testSilent && T.isDefault() )
      std::cout << OK << std::endl;
  }
}

inline void test_true( const char* F, int L, bool b, MyTSerie& T ){
  if ( !testSilent && T.isDefault() )
    std::cout << "test: " << F << "(" << L << "): ";
  ++T._tests;
  if ( !b ){
    ++T._fails;
    if ( T.isDefault() )
      std::cout << FAIL << std::endl;
    else
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '"  << b << "' != TRUE" << std::endl;
  }
  else {
    if ( !testSilent && T.isDefault() )
      std::cout << OK << std::endl;
  }
}

inline void test_false( const char* F, int L, bool b, MyTSerie& T ){
  if ( !testSilent && T.isDefault() )
    std::cout << "test: " << F << "(" << L << "): ";
  ++T._tests;
  if ( b ){
    ++T._fails;
    if ( T.isDefault() )
      std::cout << FAIL << std::endl;
    else
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '"  << b << "' != TRUE" << std::endl;
  }
  else {
    if ( !testSilent && T.isDefault() )
      std::cout << OK << std::endl;
  }
}

inline void test_true_message( const char* F, int L, const std::string& m,
			       bool b, MyTSerie& T ){
  if ( !testSilent && T.isDefault() )
    std::cout << "test: " << F << "(" << L << "): ";
  ++T._tests;
  if ( !b ){
    ++T._fails;
    if ( T.isDefault() )
      std::cout << FAIL << std::endl;
    else
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '"  << m << "'" << std::endl;
  }
  else {
    if ( !testSilent && T.isDefault() )
      std::cout << OK << std::endl;
  }
}

#endif
