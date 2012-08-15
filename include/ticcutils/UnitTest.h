#ifndef UNITTEST_H
#define UNITTEST_H

const std::string OK = "\033[1;32m OK  \033[0m";
const std::string FAIL = "\033[1;31m  FAILED  \033[0m";

bool inSub = false;

#define assertEqual( XX, YY ) test_eq<typeof XX, typeof YY>( __func__, __LINE__, XX, YY )
#define assertThrow( XX, EE )						\
  do { 									\
    if ( !inSub )							\
      std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch( const EE& ){							\
      if ( !inSub )							\
	std::cerr << OK << endl;					\
      break;								\
    }									\
    catch ( const std::exception& e ){					\
      ++fails;								\
      if ( !inSub )							\
	std::cout << FAIL << std::endl;					\
      else								\
	std::cerr << "\t";						\
      std::cerr << __func__ << "(" << __LINE__ << ") : wrong exception, what='" << e.what() << "'" << std::endl; \
      break;								\
    }									\
    ++fails;								\
    if ( !inSub )							\
      std::cout << FAIL << std::endl;					\
    else								\
      std::cerr << "\t";						\
    std::cerr << __func__ << "(" << __LINE__ << ") : no exception thrown" << std::endl; \
  }									\
  while( false )							\
    
#define assertNoThrow( XX )						\
  do { 									\
    if ( !inSub )							\
      std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch ( const std::exception& e ){					\
      ++fails;								\
      if ( !inSub )							\
	std::cout << FAIL << std::endl;					\
      else								\
	std::cerr << "\t";						\
      std::cerr << __func__ << "(" << __LINE__ << ") : throws, what='" << e.what() << "'" << std::endl; \
      break;								\
    }									\
    if ( !inSub )							\
      std::cout << OK << std::endl;					\
  }									\
  while( false )							\
  
#define assertTrue( YY ) test_true( __func__, __LINE__, (YY) )
#define assertMessage( MM, YY ) test_true_message( __func__, __LINE__, (MM), (YY) )

#define startTest( SS ) start_serie( __func__, (SS) )
#define endTest() end_serie( __func__ )

int tests;
int fails;

void init_tests(){
  tests = 0;
  fails = 0;
}

int tempfails;

void start_serie( const std::string& fun, const std::string& line ){
  tempfails = fails;
  std::cout << "testseries: " << fun << " (" << line << ")" << std::endl;
  inSub = true;
}

void end_serie( const std::string& fun ){
  if ( tempfails < fails ){
    std::cout << "testseries: " << fun << "(): " << fails - tempfails 
	      << " tests " << FAIL << std::endl;
  }
  else
    std::cout << "testseries: " << fun << "():" << OK << std::endl;
  inSub = false;
}

template <typename T1, typename T2>
inline void test_eq( const char* F, int L, 
		     const T1& s1, const T2& s2 ){
  if ( !inSub )
    std::cout << "test: " << F << "(" << L << "): ";
  ++tests;
  if ( s1 != s2 ){
    ++fails;
    if ( !inSub )
      std::cout << FAIL << std::endl;
    else 
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '" << s1 << "' != '" 
	      << s2 << "'" << std::endl;
  }
  else {
    if ( !inSub )
      std::cout << OK << std::endl;
  }
}

inline void test_true( const char* F, int L, bool b ){
  if ( !inSub )
    std::cout << "test: " << F << "(" << L << "): ";
  ++tests;
  if ( !b ){
    ++fails;
    if ( !inSub )
      std::cout << FAIL << std::endl;
    else
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '"  << b << "' != TRUE" << std::endl;
  }
  else {
    if ( !inSub )
      std::cout << OK << std::endl;
  }
}

inline void test_true_message( const char* F, int L, const std::string& m, bool b ){
  if ( !inSub )
    std::cout << "test: " << F << "(" << L << "): ";
  ++tests;
  if ( !b ){
    ++fails;
    if ( !inSub )
      std::cout << FAIL << std::endl;
    else
      std::cerr << "\t";
    std::cerr << F << "(" << L << ") : '"  << m << "'" << std::endl;
  }
  else {
    if ( !inSub )
      std::cout << OK << std::endl;
  }
}

inline int summarize_tests( int expected=0 ){
  std::cout << "performed " << tests << " tests, " 
	    << fails << " failures.";
  int diff = fails - expected;
  if ( diff > 0 ){
    std::cout << " We expected " << expected << " failures." << std::endl;
    std::cout << "overall " << FAIL << std::endl;
  }
  else if ( diff < 0 ){
    std::cout << " This is less than the " << expected << " we expected." << std::endl;
    std::cout << "overall " << FAIL << std::endl;
  }
  else {
    std::cout << " that was what we expected." << std::endl;
    std::cout << "overall " << OK << std::endl;
  }
  return diff;
}

#endif
