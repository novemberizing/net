dnl ----------------------------------------------------------
dnl INITIALIZE CONFIGURE
dnl ----------------------------------------------------------
AC_INIT([novemberizing], [0.0.1], [novemberizing@gmail.com])
LT_INIT([dlopen])

dnl ----------------------------------------------------------
dnl CREATE CONFIGURATION FILES
dnl ----------------------------------------------------------
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/x/Makefile
    src/x/net/Makefile
    src/x/event/Makefile
    src/example/Makefile
    src/example/x/Makefile
])

dnl ----------------------------------------------------------
dnl CONFIGURATION HEADER FILES
dnl ----------------------------------------------------------
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

dnl ----------------------------------------------------------
dnl AUTOMAKE
dnl ----------------------------------------------------------
AM_INIT_AUTOMAKE
#AC_PROG_RANLIB
AC_PROG_CC



dnl ----------------------------------------------------------
dnl OUTPUTING FILES
dnl ----------------------------------------------------------
AC_OUTPUT
