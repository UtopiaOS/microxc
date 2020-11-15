lib_LTLIBRARIES = libmicroxcode.la

libmicroxcode_la_CFLAGS =
libmicroxcode_la_SOURCES = developer_path.c verbose_printf.c stripext.c validators.c logging_printf.c getters.m runners.c typedefs.c
libmicroxcode_la_LIBADD = -lSystem -lobjc
libmicroxcode_la_LDFLAGS = -framework Foundation -dynamiclib -flat_namespace -current-version 0.0.2 -compatibility_version 0.0.2

EXTRA_DIST = developer_path.h verbose_printf.h stripext.h
