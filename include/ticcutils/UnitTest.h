/*
  Copyright (c) 2006 - 2021
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

#ifndef TICC_UNITTEST_H
#define TICC_UNITTEST_H

#include <cstdlib>
#include <type_traits>

const std::string OK = "\033[1;32m OK  \033[0m";
const std::string FAIL = "\033[1;31m  FAILED  \033[0m";

/// \brief class that defines a series of tests
class MyTSerie {
 public:
  MyTSerie( const std::string& fun, int lineno, const std::string& line ){
    start( fun, lineno, line );
  }
  ~MyTSerie(){
    stop( _fun );
  }
  bool isDefault() const {return _fun =="default"; };
  int _fails;
  int _tests;
  int _series;
  int _start_line;
  std::string _fun;
 private:
  void start( const std::string& fun, int lineno, const std::string& line );
  void stop( const std::string& fun );
};

/// the default test serie where all other series run in
static MyTSerie currentTestContext( "default", 0, "default" );

int exit_status = 0;
bool summarized = false;
bool testSilent = false;
std::string last_what;      ///!< global variable to hold the last what() from
/// any exception

#define TEST_SILENT_ON() testSilent = true;
#define TEST_SILENT_OFF() testSilent = false;

void MyTSerie::start( const std::string& fun, int lineno, const std::string& line ){
  /// start a new TestSerie
  /*!
    \param fun the function name
    \param lineno the line number
    \param line a message to print
  */
  _fun = fun;
  _fails = 0;
  _tests = 0;
  _series = 0;
  _start_line = lineno;
  if ( !isDefault() ){
    ++currentTestContext._series;
    std::cout << "Serie:\t" << fun << " (" << line << ")" << std::endl;
  }
}

inline void summarize_tests( int expected=0 ){
  /// print a summary of all the executed tests so far
  /*!
    \param expected the number of expected failures

    This function prints a summary about all the tests and signals if
    a there were more or less failures then expected
  */
  summarized = true;
  std::cout << "TiCC tests performed " << currentTestContext._series
	    << " testseries, with a total of " << currentTestContext._tests
	    << " tests. " << std::endl
	    << "There were " << currentTestContext._fails << " failures.";
  int diff = currentTestContext._fails - expected;
  if ( diff != 0 ){
    std::cout << " Unfortunately, we expected " << expected << " failures."
	      << std::endl;
    std::cout << "overall result: " << FAIL << std::endl;
  }
  else {
    std::cout << " That was what we expected." << std::endl;
    std::cout << "overall: " << OK << std::endl;
  }
  exit_status = diff;
}

void MyTSerie::stop( const std::string& fun ){
  /// stop a Test Serie
  /*!
    \param fun a label for printing
  */
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

#define assertEqual( XX , YY )						\
  last_what.clear();							\
  try {									\
    test_eq<decltype(XX), decltype(YY)>( __func__, __LINE__, (XX), (YY), currentTestContext ); \
      }									\
  catch ( const std::exception& e ){					\
    last_what = e.what();						\
    ++currentTestContext._fails;					\
    if ( currentTestContext.isDefault() )				\
      std::cout << FAIL << std::endl;					\
    else								\
      std::cerr << "\t";						\
    std::cerr << __func__ << "(" << __LINE__ << ") : caucht exception, what='" << e.what() << "'" << std::endl; \
  }

#define assertThrow( XX, EE )						\
  do { 									\
    last_what.clear();							\
    ++currentTestContext._tests;					\
    if ( !testSilent && currentTestContext.isDefault() )		\
      std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch( const EE& e ){						\
      last_what = e.what();						\
      if (  !testSilent && currentTestContext.isDefault() )		\
	std::cerr << OK << std::endl;					\
      break;								\
    }									\
    catch ( const std::exception& e ){					\
      last_what = e.what();						\
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
    last_what.clear();							\
    ++currentTestContext._tests;					\
    if (  !testSilent && currentTestContext.isDefault() )		\
      std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      (void)(XX); }							\
    catch ( const std::exception& e ){					\
      last_what = e.what();						\
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
  last_what.clear();							\
  try {									\
    test_true( __func__, __LINE__, (YY), currentTestContext );		\
  }									\
  catch( const std::exception& e ){					\
    last_what = e.what();						\
    ++currentTestContext._fails;					\
    std::cerr << __func__ << "(" << __LINE__ << ") error:'" << e.what() << "' ";\
    if ( currentTestContext.isDefault() )				\
      std::cout << FAIL << std::endl;					\
    else								\
      std::cerr << "\t";						\
  }

#define assertFalse( YY )						\
  last_what.clear();							\
  try {									\
    test_false( __func__, __LINE__, (YY), currentTestContext );		\
  }									\
  catch( const std::exception& e ){					\
    last_what = e.what();						\
    ++currentTestContext._fails;					\
    std::cerr << __func__ << "(" << __LINE__ << ") error:'" << e.what() << "' "; \
    if ( currentTestContext.isDefault() )				\
      std::cout << FAIL << std::endl;					\
    else								\
      std::cerr << "\t";						\
  }

#define assertMessage( MM, YY )						\
  last_what.clear();							\
  try {									\
    test_true_message( __func__, __LINE__, (MM), (YY), currentTestContext ); \
  }									\
  catch( const std::exception& e ){					\
    last_what = e.what();						\
    std::cerr << __func__ << "(" << __LINE__ << ") error: '" << e.what() << "'" << std::endl; \
  }

std::string lastError() {
  /*!
    \return the value of the what() last exception
  */
  return last_what;
}
bool hasThrown() {
  /*!
    \return true when the last test has thrown, false otherwise
   */
  return !last_what.empty();
}

void decrementError() {
  /// decrement the error count
  --currentTestContext._fails;
}

#define startTestSerie( SS ) MyTSerie currentTestContext( __func__, __LINE__, (SS) )

template <typename T1, typename T2>
  inline void test_eq( const char* F, int L,
		       const T1& s1, const T2& s2, MyTSerie& T ){
  if ( !testSilent && T.isDefault() ){
    std::cout << "test: " << F << "(" << L << "): ";
  }
  ++T._tests;
  typename std::common_type<T1,T2>::type s11 = s1;
  typename std::common_type<T1,T2>::type s22 = s2;
  if ( s11 != s22 ){
    ++T._fails;
    if ( T.isDefault() ){
      std::cout << FAIL << std::endl;
    }
    else {
      std::cerr << "\t";
    }
    std::cerr << F << "(" << L << ") : '" << s1 << "' != '"
	      << s2 << "'" << std::endl;
  }
  else if ( !testSilent && T.isDefault() ){
    std::cout << OK << std::endl;
  }
}

inline void test_true( const char* F, int L, bool b, MyTSerie& T ){
  if ( !testSilent && T.isDefault() ){
    std::cout << "test: " << F << "(" << L << "): ";
  }
  ++T._tests;
  if ( !b ){
    ++T._fails;
    if ( T.isDefault() ){
      std::cout << FAIL << std::endl;
    }
    else {
      std::cerr << "\t";
    }
    std::cerr << F << "(" << L << ") : '"  << b << "' != TRUE" << std::endl;
  }
  else if ( !testSilent && T.isDefault() ){
    std::cout << OK << std::endl;
  }
}

inline void test_false( const char* F, int L, bool b, MyTSerie& T ){
  if ( !testSilent && T.isDefault() ){
    std::cout << "test: " << F << "(" << L << "): ";
  }
  ++T._tests;
  if ( b ){
    ++T._fails;
    if ( T.isDefault() ){
      std::cout << FAIL << std::endl;
    }
    else {
      std::cerr << "\t";
    }
    std::cerr << F << "(" << L << ") : '"  << b << "' != TRUE" << std::endl;
  }
  else if ( !testSilent && T.isDefault() ){
    std::cout << OK << std::endl;
  }
}

inline void test_true_message( const char* F, int L, const std::string& m,
			       bool b, MyTSerie& T ){
  if ( !testSilent && T.isDefault() ){
    std::cout << "test: " << F << "(" << L << "): ";
  }
  ++T._tests;
  if ( !b ){
    ++T._fails;
    if ( T.isDefault() ){
      std::cout << FAIL << std::endl;
    }
    else {
      std::cerr << "\t";
    }
    std::cerr << F << "(" << L << ") : '"  << m << "'" << std::endl;
  }
  else if ( !testSilent && T.isDefault() ){
    std::cout << OK << std::endl;
  }
}

#endif
