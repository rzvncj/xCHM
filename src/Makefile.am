INCLUDES = -I$(top_srcdir)/art

bin_PROGRAMS = xchm
xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp contenttaghandler.cpp chmfontdialog.cpp \
	chmsearchpanel.cpp chmhtmlwindow.cpp chmfinddialog.cpp \
	chmindexpanel.cpp

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h contenttaghandler.h chmfontdialog.h \
	chmsearchpanel.h chmhtmlwindow.h bitfiddle.inl \
	chmfinddialog.h chmindexpanel.h

xchm_LDADD= @LINKOPT@ 

all-local:
	@REZ_COMMAND@
