#ifndef UNITTEST_H
#define UNITTEST_H

#define assert_eq( XX, YY ) test_eq<typeof XX, typeof YY>( __func__, __LINE__, XX, YY )
#define assert_true( YY ) test_true( __func__, __LINE__, (YY) )

const std::string OK = "\033[1;32m OK  \033[0m";
const std::string FAIL = "\033[1;31m  FAILED  \033[0m";

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
