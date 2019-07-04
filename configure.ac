# -*- mode: Autoconf; -*-

AC_PREREQ([2.69])

AC_INIT([dtwmcmd], [1.0], [andrewbass@gmail.com], , [https://github.com/cdesktopenv/dtwmcmd])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/dtwmcmd.c])

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Check for X11. Exports X_CFLAGS and X_LIBS.
AC_PATH_XTRA
# AC_PATH_XTRA doesn't error out by default, hence we need to do it manually.
AS_IF([test "X$no_x" = "Xyes"], [AC_MSG_ERROR([cannot find X11])])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])
AC_CHECK_HEADER([Xm/MwmUtil.h], [], [AC_MSG_ERROR([cannot find Xm/MwmUtil.h])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
