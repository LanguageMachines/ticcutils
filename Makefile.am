ACLOCAL_AMFLAGS =-I m4 --install

SUBDIRS = src include docs m4 tests

EXTRA_DIST = bootstrap.sh AUTHORS TODO NEWS ticcutils.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ticcutils.pc

ChangeLog: NEWS
	svn update; git2cl > ChangeLog
