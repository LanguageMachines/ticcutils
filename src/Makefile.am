# $Id$
# $URL$

AM_CPPFLAGS = -I@top_srcdir@/include

check_PROGRAMS = test
TESTS_ENVIRONMENT = topsrcdir=$(top_srcdir)
TESTS = $(check_PROGRAMS)
CLEANFILES = bzout.txt gzout.txt bzout.bz2 gzout.gz
test_SOURCES = test.cxx

LDADD = libticcutils.la

lib_LTLIBRARIES = libticcutils.la
libticcutils_la_LDFLAGS = -version-info 2:0:0

libticcutils_la_SOURCES = LogStream.cxx StringOps.cxx TreeHash.cxx \
	Configuration.cxx Timer.cxx XMLtools.cxx zipper.cxx tar.cxx \
	FileUtils.cxx
