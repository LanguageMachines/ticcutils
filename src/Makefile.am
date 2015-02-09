# $Id$
# $URL$

AM_CPPFLAGS = -I@top_srcdir@/include
AM_CXXFLAGS = -std=c++11

LDADD = libticcutils.la

lib_LTLIBRARIES = libticcutils.la
libticcutils_la_LDFLAGS = -version-info 2:0:0

libticcutils_la_SOURCES = LogStream.cxx StringOps.cxx TreeHash.cxx \
	Configuration.cxx Timer.cxx XMLtools.cxx zipper.cxx Tar.cxx \
	FileUtils.cxx CommandLine.cxx


check_PROGRAMS = runtest
runtest_SOURCES = runtest.cxx

TESTS_ENVIRONMENT = topsrcdir=$(top_srcdir)
TESTS = tst.sh
EXTRA_DIST = tst.sh
CLEANFILES = bzout.txt gzout.txt bzout.bz2 gzout.gz
