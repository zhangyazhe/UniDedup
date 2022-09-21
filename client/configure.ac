#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([DedupRC], [0.1], [yuchonghu@hust.edu.cn])
AC_CONFIG_SRCDIR([Agent.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([test], [0.1])

# Checks for programs.
AC_PROG_CXX
CXXFLAGS="-std=c++11 -g -O2 -Wall"

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([glib], [g_hash_table_new],[found_glib=yes],[found_glib=no])
    if test "$found_glib" = "no"; then
        AC_MSG_ERROR([*** Working glib library not found ***])
    fi
AC_CHECK_LIB([glib], [g_thread_init],[thread_safe=no],[thread_safe=yes])
    if test "$thread_safe" = "no"; then
        AC_MSG_ERROR([*** The version of glib is too low ***])
    fi
AC_CHECK_LIB([crypto],[SHA_Update],[found_crypto=yes],[found_crypto=no])
    if test "$found_crypto" = "no"; then
        AC_MSG_ERROR([*** Working crypto library not found ***])
    fi
AC_CHECK_LIB([hiredis],[redisCommand],[found_hiredis=yes],[found_hiredis=no])
    if test "$found_hiredis" = "no"; then
        AC_MSG_ERROR([*** Working hiredis library not found ***])
    fi
#AC_CHECK_LIB([memcached],[memcached_create],[found_memcached=yes],[found_memcached=no])
#    if test "$found_memcached" = "no"; then
#        AC_MSG_ERROR([*** Working memcahced library not found ***])
#    fi

AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([glib.h],[found_glib_h=yes],[found_glib_h=no])
    if test "$found_glib_h" = "no"; then
        AC_MSG_ERROR([*** Working glib.h header not found ***])
    fi
AC_CHECK_HEADERS([openssl/sha.h], [found_sha_h=yes],[found_sha_h=no])
    if test "$found_sha_h" = "no"; then
        AC_MSG_ERROR([*** Working sha.h header not found ***])
    fi
AC_CHECK_HEADERS([hiredis/hiredis.h], [found_hiredis_h=yes],[found_hiredis_h=no])
    if test "$found_hiredis_h" = "no"; then
        AC_MSG_ERROR([*** Working hiredis.h header not found ***])
    fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday inet_ntoa memmove memset])

AC_OUTPUT(Makefile
        src/common/Makefile
        src/protocol/Makefile
        src/util/Makefile)
