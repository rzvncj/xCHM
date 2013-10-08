AM_CPPFLAGS = -I$(top_srcdir)/art

bin_PROGRAMS = xchm

if ENABLE_BUILTIN_CHMLIB

xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp chmfontdialog.cpp chmhtmlnotebook.cpp \
	chmsearchpanel.cpp chmhtmlwindow.cpp chmfinddialog.cpp \
	chmindexpanel.cpp chmlistctrl.cpp hhcparser.cpp \
    chm_lib.c lzx.c

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h chmfontdialog.h chmhtmlnotebook.h \
	chmsearchpanel.h chmhtmlwindow.h bitfiddle.inl \
	chmfinddialog.h chmindexpanel.h chmlistctrl.h hhcparser.h \
    chm_lib.h lzx.h

else

xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp chmfontdialog.cpp chmhtmlnotebook.cpp \
	chmsearchpanel.cpp chmhtmlwindow.cpp chmfinddialog.cpp \
	chmindexpanel.cpp chmlistctrl.cpp hhcparser.cpp

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h chmfontdialog.h chmhtmlnotebook.h \
	chmsearchpanel.h chmhtmlwindow.h bitfiddle.inl \
	chmfinddialog.h chmindexpanel.h chmlistctrl.h hhcparser.h

endif

xchm_LDADD= @LINKOPT@ 
#xchm_LDFLAGS=-pg

#all-local:
#	@REZ_COMMAND@

