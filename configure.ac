#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([userbindmount], [0.1], [info@v2.cs.unibo.it])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AC_CONFIG_SRCDIR([userbindmount.c])
AC_CONFIG_HEADERS([config.h])
CFLAGS="$CFLAGS -Wall"
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

LT_INIT

# Checks for libraries.
AC_CHECK_LIB([cap], [cap_get_proc])

# Checks for header files.
AC_CHECK_HEADERS([stdio.h fcntl.h stdlib.h unistd.h sched.h limits.h errno.h string.h \
	sys/wait.h sys/stat.h sys/mount.h sys/capability.h sys/prctl.h getopt.h libgen.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strcmp])

AC_OUTPUT([Makefile])
