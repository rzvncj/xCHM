INCLUDES = -I$(top_srcdir)/art `wx-config --cppflags`

bin_PROGRAMS = xchm
xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp contenttaghandler.cpp

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h contenttaghandler.h endianmacros.h

xchm_LDADD= @LINKOPT@ 

