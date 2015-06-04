dnl Check for libmysql, based on version found at libdbi-drivers.sf.net (GPLv2-licensed)

AC_DEFUN([AC_FIND_FILE], [
  $3=no
  for i in $2; do
    for j in $1; do
      if test -r "$i/$j"; then
        $3=$i
        break 2
      fi
    done
  done
])

AC_DEFUN([AC_CHECK_MYSQL], [
  # Local variables
  have_mysql="no"
  ac_mysql="no"
  ac_mysql_incdir="no"
  ac_mysql_libdir="no"

  # Exported variables
  MYSQL_LIBS=""
  MYSQL_CFLAGS=""

  AC_ARG_WITH([mysql-config], AC_HELP_STRING([--with-mysql-config],  [Provide path to mysql_config.]), [
    ac_mysql="${withval}"
    if test "x${withval}" != "xno" -a "x${withval}" != "xyes"; then
        ac_mysql="yes"
        ac_mysql_incdir="${withval}"/include
        ac_mysql_libdir="${withval}"/lib
    fi
  ], [
    ac_mysql="auto"
  ])

  AC_MSG_CHECKING([for MySQL])

  # Try to find mysql_config in common directories.
  if test "x$ac_mysql" = "xauto"; then
    mysql_config_dirs="/usr/bin /usr/local/bin /usr/local/mysql/bin /opt/mysql/bin"
    AC_FIND_FILE(mysql_config, ${mysql_config_dirs}, ac_mysql_config_dir)

    if test "${ac_mysql_config_dir}" = "no"; then
      ac_mysql="no"
    else
      MYSQL_LIBS=`${ac_mysql_config_dir}/mysql_config --libs 2>/dev/null`
      MYSQL_CFLAGS=`${ac_mysql_config_dir}/mysql_config --cflags 2>/dev/null`
      if test "x$MYSQL_LIBS" = "x" -a "x$MYSQL_CFLAGS" = "x"; then
        AC_CHECK_LIB([mysqlclient], [mysql_init], [ac_mysql="yes"], [ac_mysql="no"])
      else
        ac_mysql="yes"
      fi
    fi
  else
    MYSQL_LIBS=`${ac_mysql} --libs 2>/dev/null`
    MYSQL_CFLAGS=`${ac_mysql} --cflags 2>/dev/null`

    if test "x$MYSQL_LIBS" = "x" -a "x$MYSQL_CFLAGS" = "x"; then
      AC_CHECK_LIB([mysqlclient], [mysql_init], [ac_mysql="yes"], [ac_mysql="no"])
    else
      ac_mysql="no"
    fi
  fi

  AC_MSG_RESULT([$ac_mysql])
])
