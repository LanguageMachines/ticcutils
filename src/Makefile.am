# $Id$
# $URL$

AM_CPPFLAGS = -I@top_srcdir@/include

check_PROGRAMS = test
TESTS = $(check_PROGRAMS)

test_SOURCES = test.cxx


LDADD = libticcutils.la

lib_LTLIBRARIES = libticcutils.la
libticcutils_la_LDFLAGS= -version-info 1:0:0

libticcutils_la_SOURCES = LogStream.cxx StringOps.cxx TreeHash.cxx \
	Configuration.cxx
