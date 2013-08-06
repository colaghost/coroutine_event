#lib_LIBRARIES=libctevent.a
lib_LTLIBRARIES=libctevent.la
libctevent_la_SOURCES=channel.c coroutine_event.c iomap.c task.c
libctevent_la_LDFLAGS=-module
#libctevent_a_SOURCES=channel.c coroutine_event.c iomap.c task.c
INCLUDES=-I$(top_srcdir)/include -I$(LIBEVENT_PREFIX)/include
cteventincludedir=$(includedir)
cteventinclude_HEADERS=$(top_srcdir)/include/channel.h $(top_srcdir)/include/coroutine_event.h
