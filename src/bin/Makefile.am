MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\"

bin_PROGRAMS = plexq

plexq_DEPENDENCIES = $(top_builddir)/src/lib/libplex.la
plexq_LDADD = $(top_builddir)/src/lib/libplex.la

plexq_SOURCES = plexq.c
