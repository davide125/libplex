MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libplex.la

libplex_la_SOURCES = plex.c

include_HEADERS = plex.h

libplex_la_LDFLAGS = -version-info @version_info@

libplex_la_LIBADD = ${CURL_LIBS} ${XML2_LIBS}

libplex_la_CPPFLAGS = ${CURL_CFLAGS} ${XML2_CFLAGS}
