
AC_INIT(src/chmfile.cpp)
AM_CONFIG_HEADER(config.h)

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.14.3])

AM_OPTIONS_WXCONFIG

AM_PATH_WXCONFIG(2.8.0, wxWin=1, wxWin=0, [std,aui])
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
           wxWidgets must be installed on your system
           but the wx-config script couldn't be found.
     
     	   Please check that wx-config is in path, the directory
     	   where wxWidgets libraries are installed (returned by
     	   'wx-config --libs' command) is in LD_LIBRARY_PATH or
     	   equivalent variable and wxWidgets version is 2.3.4 or above.
        ])
fi

#REZ_FLAGS="`$WX_CONFIG_PATH --rezflags`"
#if test "x$REZ_FLAGS" = "x" -o "x$REZ_FLAGS" = "x#"; then
#	REZ_COMMAND=""
#else
#	REZ_COMMAND="$REZ_FLAGS xchm"
#fi
#AC_SUBST(REZ_COMMAND)

wx_config_major_version=`echo $WX_VERSION | \
	sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
wx_config_minor_version=`echo $WX_VERSION | \
	sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`

CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY -W -Wall"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"

     
AC_ARG_ENABLE(static, 
	      [  --enable-static         link statically to wxWidgets.], 
	      LINKOPT="$WX_LIBS_STATIC",
	      LINKOPT="$WX_LIBS")
AC_SUBST(LINKOPT)

AC_ARG_ENABLE(debug,
	      [  --enable-debug          compile with gdb debug information.], 
	      CXXFLAGS="$CXXFLAGS -g")

AC_ARG_ENABLE(optimize,
	      [  --enable-optimize       optimize compiled code (-O2).], 
	      CXXFLAGS="$CXXFLAGS -O2")
AC_ARG_ENABLE(xmlrpc,
	      [  --enable-xmlrpc         Enable XmlRpc functionality in xchm.],

	      [ enable_xmlrpc=yes ])

AM_INIT_AUTOMAKE(xchm, 1.22)
AC_PROG_CXX
AC_PROG_INSTALL

AC_CHECK_TYPE(int32_t, int)
AC_CHECK_TYPE(int16_t, short)
AC_CHECK_TYPE(uint16_t, unsigned short)
AC_CHECK_TYPE(uint32_t, unsigned int)
AC_CHECK_TYPE(uint64_t, unsigned long long)

AC_CHECK_HEADER(chm_lib.h,,AC_MSG_ERROR([Can't find the CHMLIB header.]))
AC_CHECK_LIB(
    chm, chm_open,,
    AC_MSG_ERROR([Can't find/use -lchm. Please install CHMLIB first.])
)

if test "x$enable_xmlrpc" = "xyes" ; then
	AC_LANG_PUSH(C++)
	AC_CHECK_HEADERS(XmlRpc.h,,
	  AC_MSG_ERROR([Can't find the XmlRpc++ header.]))

	LIBS="$LIBS -lXmlRpc -lssl"	
	AC_TRY_LINK([#include<XmlRpc.h> ],
		[XmlRpc::XmlRpcServer svr;],
    	  [AC_DEFINE(WITH_LIBXMLRPC,1,[Compile with XmlRpc functionality.])],
	  AC_MSG_ERROR([Can't find/use -lXmlRpc. Please install XmlRpc++ first.])
	  )
	AC_LANG_POP()
fi

case "$host" in
	*-*-mingw*)
	LINKOPT="$LINKOPT -lwxjpeg-2.6 -lwxpng-2.6 -lwxtiff-2.6 -lwxzlib-2.6"
    WINDRES="windres.exe $(top_srcdir)/"
	;;
esac

AC_OUTPUT(Makefile src/Makefile art/Makefile po/Makefile.in m4/Makefile )
