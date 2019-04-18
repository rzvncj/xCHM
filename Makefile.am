
SUBDIRS = m4 po src art mac data man
EXTRA_DIST = config.rpath Doxyfile README.xmlrpc rc/xchm.rc rc/xchm.ico

ACLOCAL_AMFLAGS = -I m4 
#-I /usr/local/share/aclocal

macbundle: all
	mkdir -p $(top_srcdir)/xCHM.app/Contents/MacOS
	mkdir -p $(top_srcdir)/xCHM.app/Contents/Resources
	cp -f $(top_srcdir)/mac/Info.plist $(top_srcdir)/xCHM.app/Contents/
	cp -f $(top_srcdir)/mac/xCHM.icns $(top_srcdir)/xCHM.app/Contents/Resources

clean-local:
	-rm -rf $(top_srcdir)/xCHM.app

.PHONY: bundle

