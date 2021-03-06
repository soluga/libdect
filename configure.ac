#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

AC_COPYRIGHT([Copyright (c) 2008 Patrick McHardy <kaber@trash.net>])
AC_INIT([libdect], [0.0.1], [kaber@trash.net])
AC_DEFINE([RELEASE_NAME], ["libdect"], [Release name])

AC_CONFIG_SRCDIR([src/s_msg.c])
AC_CONFIG_HEADER([config.h])

AC_DEFINE([_GNU_SOURCE], [], [Enable various GNU extensions])
AC_DEFINE([_STDC_FORMAT_MACROS], [], [printf-style format macros])

AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug], [enable debugging [yes]])],
	      [CONFIG_DEBUG="$(echo $enableval | cut -b1)"],
	      [CONFIG_DEBUG="y"])
AC_SUBST([CONFIG_DEBUG])

# Checks for programs.
AC_PROG_CC
AC_PROG_MKDIR_P
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED

AC_ARG_ENABLE([doc],
	      [AS_HELP_STRING([--enable-doc], [build documentation [no]])],
	      [AC_CHECK_PROG(DOXYGEN, doxygen, doxygen)],
	      [AC_MSG_NOTICE([Documentation build disabled])]
	      [DOXYGEN=/bin/true])

# Checks for libraries.
AC_CHECK_LIB([nl-3], [nl_socket_alloc], ,
	     [AC_MSG_ERROR([No suitable version of libnl-3 found])])

AC_CHECK_LIB([nl-dect-3], [nl_dect_cluster_alloc], ,
	     [AC_MSG_ERROR([No suitable version of libnl-dect-3 found])])

AC_CHECK_LIB([ev], [event_init],
	     [EVENTLIB="-lev"],
	     [AC_CHECK_LIB([event], [event_init],
			   [EVENTLIB="-levent"],
			   [AC_ERROR([libev or libevent not found])])])
EVENT_LDFLAGS=$EVENTLIB
AC_SUBST(EVENT_LDFLAGS)

if test "$CONFIG_DEBUG" = "y";
then
	AC_CHECK_LIB([bfd], [bfd_init],
		     [CONFIG_BACKTRACE="y"],
		     [CONFIG_BACKTRACE="n";
		      AC_MSG_NOTICE([libbfd not found, backtrace support disabled])])
else
	CONFIG_BACKTRACE="n"
fi

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_ASSERT
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h malloc.h \
		  stddef.h stdint.h stdlib.h string.h unistd.h \
		  sys/socket.h], ,
		 [AC_MSG_ERROR([Header file not found])])
AC_CHECK_HEADERS([linux/dect.h linux/dect_netlink.h], ,
		 [AC_MSG_ERROR([DECT kernel header files not found])],
		 [#include <sys/socket.h>])

AC_CHECK_HEADER([event.h],
		[EVENTINC="-include event.h"],
		[AC_CHECK_HEADER([libev/event.h],
				 [EVENTINC="-include libev/event.h"],
				 [AC_MSG_ERROR([event.h not found])])])
EVENT_CFLAGS=$EVENTINC
AC_SUBST(EVENT_CFLAGS)

if test "$CONFIG_BACKTRACE" = "y";
then
	AC_CHECK_HEADER([bfd.h], ,
			[CONFIG_BACKTRACE="";
			 AC_MSG_NOTICE([bfd.h not found, backtrace support disabled])])
fi
AC_SUBST(CONFIG_BACKTRACE)

CFLAGS_KEEP=$CFLAGS
CFLAGS=$EVENT_CFLAGS
if [[ "$EVENT_LDFLAGS" == "-levent" ]]; then
	AC_MSG_CHECKING([checking whether event.h is compatible with libevent])
	AC_COMPILE_IFELSE(
			  [AC_LANG_SOURCE([[
	#ifdef EV_H_
	#error
	#endif
			  ]])],
			  [AC_MSG_RESULT([yes])],
			  [AC_MSG_ERROR([incompatible event.h header for libevent])]
	)
else
	AC_MSG_CHECKING([checking whether event.h is compatible with libev])
	AC_COMPILE_IFELSE(
			  [AC_LANG_SOURCE([[
	#ifndef EV_H_
	#error
	#endif
			  ]])],
			  [AC_MSG_RESULT([yes])],
			  [AC_MSG_ERROR([incompatible event.h header for libev])]
	)
fi
CFLAGS=$CFLAGS_KEEP

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_TYPEOF

AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
# AC_FUNC_MALLOC
# AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset strchr strdup strerror strtoull])

AC_CONFIG_FILES([Makefile Makefile.defs Makefile.rules])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([example/Makefile])
AC_CONFIG_FILES([doc/Makefile doc/Doxyfile])
AC_CONFIG_FILES([libdect-0.0.1.pc])
AC_OUTPUT
