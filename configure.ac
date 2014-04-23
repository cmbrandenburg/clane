AC_PREREQ([2.68])
AC_INIT([clane], [master], [c.m.brandenburg@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER(config.h)

AC_PROG_CXX
AM_PROG_AR

LT_PREREQ([2.4.2])
LT_INIT()

# http://stackoverflow.com/questions/11802727/libtool-doesnt-provide-library-dependencies-to-link
# http://git.lrde.epita.fr/?p=spot.git;a=commitdiff;h=0e74b76521341f670f6b76f8ef24a6dcf6e3813b
link_all_deplibs=yes
link_all_deplibs_CXX=yes

AC_ARG_ENABLE([examples], AS_HELP_STRING([--enable-examples], [Build application examples]))
AM_CONDITIONAL([BUILD_EXAMPLES], [test x$enable_examples == xyes ])

AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"], [AX_CXX_COMPILE_STDCXX_11(noext,optional)])

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for std::thread support for movable arguments])
AC_TRY_COMPILE(
	[#include <thread>
struct alpha {
	alpha() {}
	alpha(alpha const &) = delete;
	alpha &operator=(alpha const &) = delete;
	alpha(alpha &&) {}
	alpha &operator=(alpha &&) { return *this; }
};

void bravo(alpha &&) {}],
	[alpha a;
std::thread(bravo, std::move(a));],
	[AC_MSG_RESULT(yes) AC_DEFINE([HAVE_STD_THREAD_MOVE_ARG], 1,
	[Define to 1 if the standard library std::thread supports movable
	 arguments])],
	[AC_MSG_RESULT(no)]
	)
AC_LANG_POP([C++])

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for std::string::shrink_to_fit])
AC_TRY_COMPILE(
	[#include <string>],
	[std::string s;
	 s.shrink_to_fit();],
	[AC_MSG_RESULT(yes) AC_DEFINE([HAVE_STD_STRING_SHRINK_TO_FIT], 1,
	[Define to 1 if the standard library supports
	 std::string::shrink_to_fit])],
	[AC_MSG_RESULT(no)]
	)
AC_LANG_POP([C++])

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for std::multimap::emplace])
AC_TRY_COMPILE(
	[#include <map>],
	[std::multimap<int, int> m;
m.emplace(1, 2);],
	[AC_MSG_RESULT(yes) AC_DEFINE([HAVE_STD_MULTIMAP_EMPLACE], 1,
	[Define to 1 if the standard library supports std::multimap::emplace])],
	[AC_MSG_RESULT(no)]
	)
AC_LANG_POP([C++])

AX_BOOST_BASE([1.49], [], [AC_MSG_ERROR(need boost libraries >= 1.49)])
AX_BOOST_SYSTEM()
AX_BOOST_FILESYSTEM()
AX_BOOST_REGEX()
AC_CHECK_FUNCS([eventfd])

AC_CONFIG_FILES(
	Makefile
	examples/Makefile
	)
AC_OUTPUT

