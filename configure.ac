AC_PREREQ([2.68])
AC_INIT([clane], [0.0.0-dev], [c.m.brandenburg@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES()
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AM_PROG_AR

LT_PREREQ([2.4.2])
LT_INIT()

AC_CONFIG_FILES(
	Makefile
	src/Makefile
	src/check/Makefile
	src/net/Makefile
	src/net/tests/Makefile
	)
AC_OUTPUT

