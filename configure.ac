AC_INIT(11pm, 0.1)
AC_CONFIG_SRCDIR(src/11pm/11pm.c)
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC
AM_PROG_AR
LT_INIT

missing_libs=''
missing () {
  # TODO: use vendored libs instead
  missing_libs="$missing_libs $1,"
}

AC_CHECK_HEADER([yajl/yajl_parse.h], [], [missing yajl])

if test -n "$missing_libs"
then
  $as_echo "
============ ERROR: some libraries were missing =============
  Please install the following libraries and their headers:
   $missing_libs

=============================================================
"
  exit -1
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
  lib/Makefile
  lib/lib23package/Makefile
  lib/lib23repo/Makefile
  lib/lib23core/Makefile
  lib/lib11pm/Makefile
  src/11pm/Makefile
])
AC_OUTPUT
