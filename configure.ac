#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ticcutils],[0.36],[lamasoftware@science.ru.nl]) #also adapt in codemeta.json!
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([.])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AX_REQUIRE_DEFINED([AX_CXX_COMPILE_STDCXX_17])

# Checks for programs.
AC_PROG_CXX([g++ clang++] )
AX_CXX_COMPILE_STDCXX_17

# libtool stuff
LT_INIT

# when running tests, use CXX
AC_LANG([C++])

# check OpenMP support
AC_OPENMP
if test "x$ac_cv_prog_cxx_openmp" != "xunsupported"; then
   CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
   AC_DEFINE(HAVE_OPENMP, 1 , Define to 1 if you have OpenMP )
else
   AC_MSG_NOTICE([We don't have OpenMP. Multithreaded operation is disabled])
fi

#checks for libraries.
AX_CHECK_ZLIB( [],
	       [AC_MSG_ERROR([zlib not found. Please install libzlib1g-dev.])] )

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/socket.h unistd.h sys/time.h stdint.h])

AC_CHECK_HEADERS([bzlib.h],
		[LIBS="$LIBS -lbz2"],
		[AC_MSG_ERROR([bz2lib not found. Please install libbz2-dev])] )

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_PID_T
AX_LIB_SOCKET_NSL
AX_TYPE_SOCKLEN_T
AC_CHECK_TYPES( [ptrdiff_t] )

# Checks for library functions.
AC_CHECK_FUNCS([localtime_r gettimeofday mkdir getaddrinfo gethostbyaddr gethostbyname inet_ntoa memset socket strerror dup2 memmove floor localeconv realpath strtoull strdup])
AC_FUNC_FORK
AC_FUNC_STRTOD
AC_FUNC_MALLOC

AX_PTHREAD([],[AC_MSG_ERROR([We need pthread support!])])

if test x"$ax_pthread_ok" = xyes; then
   LIBS="$PTHREAD_LIBS $LIBS" \
   CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
fi

PKG_PROG_PKG_CONFIG

if test "x$prefix" = "xNONE"; then
   prefix="/usr/local"
fi

if test "x$PKG_CONFIG_PATH" = x
then
   export PKG_CONFIG_PATH="$prefix/lib/pkgconfig"
else
   export PKG_CONFIG_PATH="$prefix/lib/pkgconfig:$PKG_CONFIG_PATH"
fi

AC_OSX_PKG( [icu4c] )
AC_MSG_NOTICE([pkg-config: PATH=$PKG_CONFIG_PATH])

PKG_CHECK_MODULES([XML2], [libxml-2.0 >= 2.6.16] )
CXXFLAGS="$CXXFLAGS $XML2_CFLAGS"
LIBS="$LIBS $XML2_LIBS"

PKG_CHECK_MODULES([ICU], [icu-uc >= 50 icu-io] )
CXXFLAGS="$CXXFLAGS $ICU_CFLAGS"
LIBS="$ICU_LIBS $LIBS"

AC_CONFIG_FILES([
  Makefile
  ticcutils.pc
  m4/Makefile
  src/Makefile
  docs/Makefile
  include/Makefile
  include/ticcutils/Makefile
  tests/Makefile
  tests/sub1/Makefile
])
AC_OUTPUT
