#ifndef UNITTEST_H
#define UNITTEST_H

const std::string OK = "\033[1;32m OK  \033[0m";
const std::string FAIL = "\033[1;31m  FAILED  \033[0m";


#define assertEqual( XX, YY ) test_eq<typeof XX, typeof YY>( __func__, __LINE__, XX, YY )
#define assertThrow( XX, EE ) \
  do { 									\
    std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch( const EE& ){							\
      std::cerr << OK << endl;						\
      break;								\
    }									\
    catch ( const std::exception& e ){					\
      std::cout << FAIL << std::endl;					\
      std::cerr << __func__ << "(), line " << __LINE__ << " : wrong exception, what='" << e.what() << "'" << std::endl; \
      break;								\
    }									\
    std::cout << FAIL << std::endl;					\
    std::cerr << __func__ << "(), line " << __LINE__ << " : no exception thrown" << std::endl; \
  }									\
  while( false )							\
    
#define assertNoThrow( XX )						\
  do { 									\
    std::cout << "test: " << __func__ << "(" << __LINE__ << "): ";	\
    try {								\
      XX; }								\
    catch ( const std::exception& e ){					\
      std::cout << FAIL << std::endl;					\
      std::cerr << __func__ << "(), line " << __LINE__ << " : thrwos, what='" << e.what() << "'" << std::endl; \
      break;								\
    }									\
    std::cout << OK << std::endl;					\
  }									\
  while( false )							\
  
#define assertTrue( YY ) test_true( __func__, __LINE__, (YY) )

int tests;
int fails;

void init_tests(){
  tests = 0;
  fails = 0;
}

template <typename T1, typename T2>
inline void test_eq( const char* F, int L, 
		     const T1& s1, const T2& s2 ){
  std::cout << "test: " << F << "(" << L << "): ";
  ++tests;
  if ( s1 != s2 ){
    ++fails;
    std::cout << FAIL << std::endl;
    std::cerr << F << "(), line " << L << " : '" << s1 << "' != '" 
	      << s2 << "'" << std::endl;
  }
  else {
    std::cout << OK << std::endl;
  }
}

inline void test_true( const char* F, int L, bool b ){
  std::cout << "test: " << F << "(" << L << "): ";
  ++tests;
  if ( !b ){
    ++fails;
    std::cout << FAIL << std::endl;
    std::cerr << F << "(), line " << L << " : '"  << b << "' != TRUE" << std::endl;
  }
  else {
    std::cout << OK << std::endl;
  }
}

inline int summarize_tests(){
  std::cout << "performed " << tests << " tests, " 
	    << fails << " failures." << std::endl;
  return fails;
}

#endif
