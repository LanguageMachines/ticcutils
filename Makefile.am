# $Id: Makefile.am 7708 2011-01-05 13:47:18Z sloot $
# $URL: https://ilk.uvt.nl/svn/trunk/sources/Timbl6/Makefile.am $

ACLOCAL_AMFLAGS =-I m4 --install

SUBDIRS = src include docs m4

EXTRA_DIST = bootstrap AUTHORS TODO NEWS ticcutils.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ticcutils.pc

ChangeLog: NEWS
	svn update; svn2cl --include-rev
