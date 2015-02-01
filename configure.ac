#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libslp-lbs-plugin-replay], [0.1], [daejins.kim@samsung.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

###### Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_RANLIB

###### Checks for libraries.
PKG_CHECK_MODULES(DEVICED, deviced)
AC_SUBST(DEVICED_CFLAGS)
AC_SUBST(DEVICED_LIBS)

PKG_CHECK_MODULES(GPS_MANAGER_PLUGIN, lbs-server-plugin)
AC_SUBST(GPS_MANAGER_PLUGIN_CFLAGS)
AC_SUBST(GPS_MANAGER_PLUGIN_LIBS)

#PKG_CHECK_MODULES(GEOCLUE_XPS_PLUGIN, geoclue-xps-plugin)
#AC_SUBST(GEOCLUE_XPS_PLUGIN_CFLAGS)
#AC_SUBST(GEOCLUE_XPS_PLUGIN_LIBS)

PKG_CHECK_MODULES(VCONF, vconf)
AC_SUBST(VCONF_CFLAGS)
AC_SUBST(VCONF_LIBS)

PKG_CHECK_MODULES(DLOG, dlog)
AC_SUBST(DLOG_CFLAGS)
AC_SUBST(DLOG_LIBS)

####### Config files
AC_CONFIG_FILES([
	Makefile
	gps-plugin/Makefile
	])

AC_OUTPUT

