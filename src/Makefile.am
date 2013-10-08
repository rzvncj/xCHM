AM_CPPFLAGS = -I$(top_srcdir)/art

bin_PROGRAMS = xchm

xchm_SOURCES = chmapp.cpp chmfile.cpp chmframe.cpp chmfshandler.cpp \
	chminputstream.cpp chmfontdialog.cpp chmhtmlnotebook.cpp \
	chmsearchpanel.cpp chmhtmlwindow.cpp chmfinddialog.cpp \
	chmindexpanel.cpp chmlistctrl.cpp hhcparser.cpp

noinst_HEADERS = chmapp.h chmfile.h chmframe.h chmfshandler.h \
	chminputstream.h chmfontdialog.h chmhtmlnotebook.h \
	chmsearchpanel.h chmhtmlwindow.h bitfiddle.inl \
	chmfinddialog.h chmindexpanel.h chmlistctrl.h hhcparser.h \
	xchm_chm_lib.h lzx.h

if ENABLE_BUILTIN_CHMLIB
xchm_SOURCES += chm_lib.c lzx.c
endif

if ENABLE_APP_INSTALL
install-exec-hook:
	mkdir -p $(top_srcdir)/xCHM.app/Contents/MacOS
	mkdir -p $(top_srcdir)/xCHM.app/Contents/Resources
	cp -f $(top_srcdir)/mac/Info.plist $(top_srcdir)/xCHM.app/Contents/
	cp -f $(top_srcdir)/mac/xCHM.icns $(top_srcdir)/xCHM.app/Contents/Resources
endif

xchm_LDADD = @LINKOPT@
#xchm_LDFLAGS=-pg

#all-local:
#	@REZ_COMMAND@

