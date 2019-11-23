AC_INIT([xchm], [1.32])
AC_CONFIG_SRCDIR(src/chmfile.cpp)
AC_CONFIG_HEADERS(config.h)

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.14.3])

AM_OPTIONS_WXCONFIG

AM_PATH_WXCONFIG(3.0.0, wxWin=1, wxWin=0, [std,aui])
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
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY -std=c++14 -W -Wall -Wno-ignored-qualifiers -Wno-cast-function-type"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
XMLRPCDIR=""
     
AC_ARG_ENABLE(static, 
	      [  --enable-static         link statically to wxWidgets.], 
	      LINKOPT="$WX_LIBS_STATIC",
	      LINKOPT="$WX_LIBS")
AC_SUBST(LINKOPT)

AC_ARG_ENABLE(builtin-chmlib, 
              [  --enable-builtin-chmlib compile with included chmlib source.], 
              [case "${enableval}" in
                yes) builtin_chmlib=true ;;
                no)  builtin_chmlib=false ;;
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-builtin-chmlib]) ;;
              esac],[builtin_chmlib=false])
AM_CONDITIONAL([ENABLE_BUILTIN_CHMLIB], [test x$builtin_chmlib = xtrue])

AC_ARG_ENABLE(debug,
	      [  --enable-debug          compile with gdb debug information.], 
	      CXXFLAGS="$CXXFLAGS -g")

AC_ARG_ENABLE(optimize,
	      [  --enable-optimize       optimize compiled code (-O2).], 
	      CXXFLAGS="$CXXFLAGS -O2")

AC_ARG_ENABLE(xmlrpc,
	      [  --enable-xmlrpc         enable XmlRpc functionality in xchm.],
	      [ enable_xmlrpc=yes ])

AC_ARG_WITH(xmlrpc-dir,
    [  --with-xmlrpc-dir       specify XMLRPC++ includes and libraries parent directory],
XMLRPCDIR="$withval")

if test -n "$XMLRPCDIR" ; then
    CFLAGS="-I$XMLRPCDIR/include -I$XMLRPCDIR/include/xmlrpcpp $CFLAGS"
    CXXFLAGS="-I$XMLRPCDIR/include -I$XMLRPCDIR/include/xmlrpcpp $CXXFLAGS"
    CPPFLAGS="-I$XMLRPCDIR/include -I$XMLRPCDIR/include/xmlrpcpp $CPPFLAGS"
    LDFLAGS="-L$XMLRPCDIR/lib -Wl,-rpath,$XMLRPCDIR/lib $LDFLAGS"
fi

AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_INSTALL

AC_CHECK_TYPE(int32_t, int)
AC_CHECK_TYPE(int16_t, short)
AC_CHECK_TYPE(uint16_t, unsigned short)
AC_CHECK_TYPE(uint32_t, unsigned int)
AC_CHECK_TYPE(uint64_t, unsigned long long)

if test x$builtin_chmlib = xtrue ; then
        AC_DEFINE(ENABLE_BUILTIN_CHMLIB, 1, [Compile with included chmlib source.])

else
        AC_CHECK_HEADER(chm_lib.h,,AC_MSG_ERROR([Can't find the CHMLIB header.]))
        AC_CHECK_LIB(
            chm, chm_open,,
                AC_MSG_ERROR([Can't find/use -lchm. Please install CHMLIB first.])
)
fi

if test "x$enable_xmlrpc" = "xyes" ; then
	AC_LANG_PUSH([C++])

	AC_CHECK_HEADER(XmlRpc.h,,AC_MSG_ERROR([Can't find the XmlRpc++ header.]))

	LIBS="$LIBS -lxmlrpcpp -lssl"	
	AC_TRY_LINK([#include<XmlRpc.h> ],
		[XmlRpc::XmlRpcServer svr;],
    	  [AC_DEFINE(WITH_LIBXMLRPC, 1, [Compile with XmlRpc functionality.])],
	  AC_MSG_ERROR([Can't find/use -lXmlRpc. Please install XmlRpc++ first.])
	  )
	AC_LANG_POP([C++])
fi

case "$host" in
	*-*-mingw*)
	LINKOPT="$LINKOPT -lwxjpeg-2.6 -lwxpng-2.6 -lwxtiff-2.6 -lwxzlib-2.6"
    WINDRES="windres.exe $(top_srcdir)/"
	;;
esac

AC_OUTPUT(Makefile src/Makefile art/Makefile po/Makefile.in m4/Makefile
          mac/Makefile data/Makefile man/Makefile)

