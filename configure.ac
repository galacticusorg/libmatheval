dnl Copyright (C) 1999, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011,
dnl 2012, 2013, 2026 Free Software Foundation, Inc.
dnl
dnl This file is part of GNU libmatheval
dnl
dnl GNU libmatheval is free software: you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation, either version 3 of the
dnl License, or (at your option) any later version.
dnl
dnl GNU libmatheval is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of 
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with GNU libmatheval.  If not, see
dnl <http://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([libmatheval],[1.1.11],[bug-libmatheval@gnu.org])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([lib/matheval.h])
AC_CONFIG_HEADERS([config.h])

dnl Initialize the test suite.
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_FILES([tests/matheval.sh], [chmod +x tests/matheval.sh])
AC_CONFIG_FILES([tests/Makefile])

dnl Checks for programs.
AC_PROG_CC
AM_PROG_LEX([noyywrap])
AC_PROG_YACC
LT_INIT
AM_MISSING_PROG([AUTOM4TE], [autom4te])

dnl Guile is needed by test suite only.
have_guile=no
PKG_PROG_PKG_CONFIG([0.9.0], [PKG_CONFIG=""])
if test -n "$PKG_CONFIG"; then
  PKG_CHECK_MODULES([GUILE], [guile-3.0], [have_guile=yes],
    [PKG_CHECK_MODULES([GUILE], [guile-2.2], [have_guile=yes],
      [PKG_CHECK_MODULES([GUILE], [guile-2.0], [have_guile=yes],
        [have_guile=no])])])
fi
AM_CONDITIONAL([HAVE_GUILE], [test "x$have_guile" = xyes])

dnl Checks for header files.
AC_CHECK_HEADERS([math.h strings.h stddef.h stdint.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS([bzero memset], [break])

AC_CONFIG_FILES([Makefile doc/Makefile lib/Makefile libmatheval.pc])
AC_OUTPUT
