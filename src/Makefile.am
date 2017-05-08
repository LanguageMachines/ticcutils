AM_CPPFLAGS = -I@top_srcdir@/include
AM_CXXFLAGS = -std=c++11 # -Weffc++

LDADD = libticcutils.la

lib_LTLIBRARIES = libticcutils.la
libticcutils_la_LDFLAGS = -version-info 2:0:0

libticcutils_la_SOURCES = LogStream.cxx StringOps.cxx TreeHash.cxx \
	Configuration.cxx Timer.cxx XMLtools.cxx zipper.cxx Tar.cxx \
	FileUtils.cxx CommandLine.cxx SocketBasics.cxx ServerBase.cxx \
	FdStream.cxx


check_PROGRAMS = runtest testlogstream
runtest_SOURCES = runtest.cxx
testlogstream_SOURCES =testlogstream.cxx

TESTS_ENVIRONMENT = topsrcdir=$(top_srcdir)
TESTS = tst.sh
EXTRA_DIST = tst.sh
CLEANFILES = bzout.txt gzout.txt bzout.bz2 gzout.gz nasty.txt
