bin_PROGRAMS = xcrun

xcrun_SOURCES = xcrun.m
xcrun_CFLAGS = -I$(top_srcdir)/lib -Wall -Werror -O2
xcrun_OBJCFLAGS = -I$(top_srcdir)/lib -Wall -Werror -O2
xcrun_LDADD = $(top_builddir)/lib/libmicroxcode.la -lSystem -lobjc
xcrun_LDFLAGS = -framework Foundation -framework AppKit