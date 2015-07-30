# Run `./bootstrap` to generate the "configure" script.

# Define the package version numbers, bug reporting link, project homepage.
m4_define([MYSQLDUMPTAB_VERSION_MAJOR_], [0])
m4_define([MYSQLDUMPTAB_VERSION_MINOR_], [1])
m4_define([MYSQLDUMPTAB_VERSION_PATCH_], [1])
m4_define([MYSQLDUMPTAB_VERSION_STRING_], [MYSQLDUMPTAB_VERSION_MAJOR_.MYSQLDUMPTAB_VERSION_MINOR_.MYSQLDUMPTAB_VERSION_PATCH_])
m4_define([MYSQLDUMPTAB_BUGREPORT_URL_], [https://github.com/tzvetkoff/mysqldumptab/issues])
m4_define([MYSQLDUMPTAB_PROJECT_URL_], [https://github.com/tzvetkoff/mysqldumptab])

# Initialize autoconf & automake.
AC_PREREQ(2.59)
AC_INIT([MySQLDumpTab], [MYSQLDUMPTAB_VERSION_STRING_], [MYSQLDUMPTAB_BUGREPORT_URL_], [mysqldumptab], [MYSQLDUMPTAB_PROJECT_URL_])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
# AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.9 foreign])

# Define macro variables for the package version numbers.
AC_DEFINE([MYSQLDUMPTAB_VERSION_MAJOR], [MYSQLDUMPTAB_VERSION_MAJOR_], [Define the major version number.])
AC_DEFINE([MYSQLDUMPTAB_VERSION_MINOR], [MYSQLDUMPTAB_VERSION_MINOR_], [Define the minor version number.])
AC_DEFINE([MYSQLDUMPTAB_VERSION_PATCH], [MYSQLDUMPTAB_VERSION_PATCH_], [Define the patch version number.])
AC_DEFINE([MYSQLDUMPTAB_VERSION_STRING], ["MYSQLDUMPTAB_VERSION_STRING_"], [Define the version string.])
AC_DEFINE([MYSQLDUMPTAB_BUGREPORT_URL], ["MYSQLDUMPTAB_BUGREPORT_URL_"], [Bug reporting url])
AC_DEFINE([MYSQLDUMPTAB_PROJECT_URL], ["MYSQLDUMPTAB_PROJECT_URL_"], [Project homepage])

# Checks for programs.
AC_PROG_CC()
AC_PROG_CPP()
AC_PROG_INSTALL()
AC_PROG_LN_S()
AC_PROG_MAKE_SET()
AC_PROG_LIBTOOL()

# Check for MySQL
AC_CHECK_MYSQL()

if test x"$ac_mysql" != "xyes"; then
  AC_MSG_ERROR([MySQL not found. Please install MySQL or use --with-mysql-config=path.])
fi

AC_SUBST([MYSQL_LIBS])
AC_SUBST([MYSQL_CFLAGS])

# Buffer size configuration
AC_ARG_WITH([buffer-block-size], AC_HELP_STRING([--with-buffer-block-size], [Set output buffer block size, @<:@default: 0x4000@:>@.]), [
  MYSQLDUMPTAB_BUFFER_BLOCK_SIZE="${withval}"
], [
  MYSQLDUMPTAB_BUFFER_BLOCK_SIZE="0x400000"
])

AC_DEFINE_UNQUOTED([MYSQLDUMPTAB_BUFFER_BLOCK_SIZE], [${MYSQLDUMPTAB_BUFFER_BLOCK_SIZE}], [Output buffer block size.])

# Makefiles.
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])

# Le grand finale.
AC_OUTPUT()

echo
echo "Configuration complete."
echo
echo "Now, run make and pray."
echo
