INCLUDES = -I$(top_srcdir)/art `$(WXCFG) --cxxflags`

bin_PROGRAMS = xchm
xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp contenttaghandler.cpp chmfontdialog.cpp

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h contenttaghandler.h endianmacros.h chmfontdialog.h

xchm_LDADD= @LINKOPT@ 

