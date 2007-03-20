INCLUDES = -I$(top_srcdir)/art

bin_PROGRAMS = xchm
xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp chmfontdialog.cpp \
	chmsearchpanel.cpp chmhtmlwindow.cpp chmfinddialog.cpp \
	chmindexpanel.cpp chmlistctrl.cpp spantag.cpp hhcparser.cpp

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h chmfontdialog.h \
	chmsearchpanel.h chmhtmlwindow.h bitfiddle.inl \
	chmfinddialog.h chmindexpanel.h chmlistctrl.h hhcparser.h

xchm_LDADD= @LINKOPT@ 
#xchm_LDFLAGS=-pg

all-local:
	@REZ_COMMAND@
