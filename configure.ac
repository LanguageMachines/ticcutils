#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([ticcutils],[0.18],[lamasoftware@science.ru.nl])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([.])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

if test x"${CXXFLAGS+set}" = xset; then
  # the user set CXXFLAGS; don't override it.
  cxx_flags_were_set=true
else
  cxx_flags_were_set=false
fi
if $cxx_flags_were_set; then
  CXXFLAGS=$CXXFLAGS
fi

# Checks for programs.
AC_PROG_CXX([g++] )
AC_PROG_LIBTOOL
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
	       [AC_MSG_NOTICE([libzlib1g-dev not found. gz support disabled.])] )

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/socket.h unistd.h sys/time.h])

AC_CHECK_HEADERS([bzlib.h],
		[LIBS="$LIBS -lbz2"],
		[AC_MSG_NOTICE([libbz2-dev not found. bz2 support disabled])] )

AC_CHECK_HEADERS([libtar.h],
		[LIBS="$LIBS -ltar"],
		[AC_MSG_NOTICE([libtar-dev not found. tar support disabled.])] )

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_PID_T
AX_LIB_SOCKET_NSL
AX_TYPE_SOCKLEN_T

# Checks for library functions.
AC_CHECK_FUNCS([localtime_r gettimeofday mkdir getaddrinfo gethostbyaddr gethostbyname inet_ntoa memset socket strerror dup2 memmove ])
AC_FUNC_FORK
# AC_FUNC_MALLOC seems to drive clang nuts

AX_PTHREAD([],[AC_MSG_ERROR([We need pthread support!])])

if test x"$ax_pthread_ok" = xyes; then
   LIBS="$PTHREAD_LIBS $LIBS" \
   CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
fi

# only require boost for OLD gcc compilers AND als stdlib++
# for now we HAV to stick to boost regex :{

# AX_COMPILER_VENDOR
# if test x"$ax_cv_cxx_compiler_vendor" = xgnu; then
#   AX_COMPILER_VERSION
#   AX_COMPARE_VERSION( [$ax_cv_cxx_compiler_version], [lt], [4.9],
#                      [AX_BOOST_BASE( [1.50],
#                                     [AX_BOOST_REGEX
#                                     LIBS="$LIBS $BOOST_REGEX_LIB"
#                                     AC_DEFINE(HAVE_WORKING_REGEX, 2 , Define to 2 if you have boost regular expresions )],
#                                     [AC_MSG_ERROR([No Regular expression support available. Please install libboost-regex-dev, or update gcc])])],
#                       AC_DEFINE(HAVE_WORKING_REGEX, 1 , Define to 1 if you have working native C++ regular expresions ) )
# else
#   AC_DEFINE(HAVE_WORKING_REGEX, 1 , Define to 1 if you have native C++ regular expresions)
# fi

AX_BOOST_BASE( [1.50],
	       [AX_BOOST_REGEX
	        LIBS="$LIBS $BOOST_REGEX_LIB"
                AC_DEFINE(HAVE_WORKING_REGEX, 2 , Define to 2 if you have boost regular expresions )],
               [AC_MSG_ERROR([No Regular expression support available. Please install libboost-regex-dev])])

PKG_PROG_PKG_CONFIG

if test "x$prefix" = "xNONE"; then
   prefix="/usr/local"
fi

if test "x$PKG_CONFIG_PATH" = x; then
   export PKG_CONFIG_PATH="$prefix/lib/pkgconfig"
else
   export PKG_CONFIG_PATH="$prefix/lib/pkgconfig:$PKG_CONFIG_PATH"
fi

AC_CANONICAL_HOST
AC_MSG_NOTICE([host OS:$(host_os)])
if test "$(host_os)" = "osx"; then
for i in `ls /usr/local/opt/`;
do
    if [ -d "/usr/local/opt/$i/lib/pkgconfig" ];
    then export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:"/usr/local/opt/$i/lib/pkgconf\
ig";
    fi;
done

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
