man1_MANS = ticcutils.1 ticc_prettyprint.1 ticc_logstream.1 ticc_string.1 \
	ticc_unit_test.1 ticc_configuration.1 ticc_commandline.1 \
	ticc_fdstream.1

EXTRA_DIST = ticcutils.1 ticc_prettyprint.1 ticc_logstream.1 ticc_string.1 \
	ticc_unit_test.1 ticc_configuration.1 ticc_commandline.1 \
	ticc_fdstream.1 dox.cfg

# https://stackoverflow.com/questions/10682603/generating-and-installing-doxygen-documentation-with-autotools

if HAVE_DOXYGEN
directory = $(top_srcdir)/docs/html

$(directory): doxyfile.stamp

doxyfile.stamp: dox.cfg
	$(DOXYGEN) $^
	echo Timestamp > $@

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	-rm -rf $(top_srcdir)/docs/html
endif
