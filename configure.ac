#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([istatd], [0.5.5], [http://code.google.com/p/istatd/issues/entry])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST
AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

AC_MSG_CHECKING([if the compiler supports -Wall])
oldcppflags="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -Wall"
AC_COMPILE_IFELSE([int main(void) { return 0; }],[
	AC_MSG_RESULT([yes])
],[
	AC_MSG_RESULT([no])
	CPPFLAGS="$oldcppflags"
])

AC_MSG_CHECKING([if the linker supports -rdynamic])
oldldflags="$LDFLAGS"
LDFLAGS="$LDFLAGS -rdynamic"
AC_LINK_IFELSE([int main(void) { return 0; }],[
	AC_MSG_RESULT([yes])
],[
	AC_MSG_RESULT([no])
	LDFLAGS="$oldldflags"
])

# Checks for libraries.
# AC_CHECK_LIB([dl], [dlopen],,[
# 	AC_CHECK_FUNC([dlopen],,[
# 		AC_MSG_ERROR([cannot find a suitable dlopen() implementation])
# 	])
# ])

AC_CHECK_PROG([XML2_CONFIG],[xml2-config],[xml2-config])
if test x"$XML2_CONFIG" = x"" ; then
	AC_MSG_ERROR([cannot find xml2-config: depending on your operating system you may need to install a '-dev' package for libxml2])
fi
LIBS="$LIBS `$XML2_CONFIG --libs`"
CPPFLAGS="$CPPFLAGS `$XML2_CONFIG --cflags`"
AC_CHECK_HEADER([libxml/parser.h],,[
	AC_MSG_ERROR([xml2-config was located, but the header files could not be found])
])
AC_CHECK_FUNC([xmlFileOpen],[
	AC_DEFINE_UNQUOTED([HAVE_LIBXML2],[1],[Define to 1 if you have the 'xml2' library (-lxml2).])
],[
	AC_MSG_ERROR([xml2-config was located, but a test program linking against -lxml2 could not be built])
])

case "${host_os}" in
	linux|linux-*)
		have_procfs=procfs
		;;
	*)
		have_procfs=none
		;;
esac

dnl Fall back to procfs, if available

use_cpu=$have_procfs
use_net=$have_procfs
use_uptime=$have_procfs
use_mem=$have_procfs
use_swap=$have_procfs
use_load=$have_procfs
use_disk=none

dnl Specific tests for probe types

AC_CHECK_LIB([kstat],[kstat_open],[
AC_DEFINE_UNQUOTED([HAVE_LIBKSTAT],1,[Define if libkstat is available])
have_kstat=yes
LIBS="$LIBS -lkstat"
use_cpu=kstat
use_net=kstat
use_uptime=kstat
use_mem=kstat
],[have_kstat=no])

if test x"$have_kstat" = x"no" ; then
	AC_CHECK_LIB([kvm],[kvm_open],[
	AC_DEFINE_UNQUOTED([HAVE_LIBKVM],1,[Define if libkvm is available])
	LIBS="$LIBS -lkvm"
	use_mem=kvm
	use_swap=kvm
	])
fi
AC_CHECK_FUNC([getloadavg],[
use_load=getloadavg
])

AC_HEADER_TIME
AC_CHECK_FUNC([clock_gettime],[
AC_MSG_CHECKING([if CLOCK_UPTIME is available])
AC_COMPILE_IFELSE([
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

static clockid_t testuptime = CLOCK_UPTIME;
],[have_clock_uptime=yes ; use_uptime=gettime],[have_clock_uptime=no])
AC_MSG_RESULT([$have_clock_uptime])
])

dnl Use sysctl for cpu and net if procfs isn't available
AC_CHECK_FUNC([sysctl],[
AC_DEFINE_UNQUOTED([HAVE_SYSCTL],1,[Define if sysctl is available])
test x"$use_cpu" = x"none" && use_cpu=sysctl
test x"$use_net" = x"none" && use_net=sysctl
have_sysctl=yes
],[have_sysctl=no])

dnl Try to use sysctl on freebsd if there's no CLOCK_UPTIME
if test x"$use_uptime" = x"none" ; then
	case "$host_os" in
		freebsd*)
			if test x"$have_sysctl" = x"yes" ; then
				use_uptime=sysctl
			fi
			;;
	esac
fi

dnl We only use swapctl if procfs isn't available
AC_CHECK_FUNC([swapctl],[
test x"$use_swap" = x"none" && use_swap=swapctl
])

AC_CHECK_FUNC([statvfs],[
use_disk=statvfs
],[
	AC_CHECK_FUNC([statfs],[
		use_disk=statfs
	])
])

AC_CHECK_LIB([avahi-client],[avahi_client_new],[
AC_DEFINE_UNQUOTED([HAVE_LIBAVAHI_CLIENT],1,[Define if libavahi-client is available])
LIBS="$LIBS -lavahi-client"
])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h mntent.h netdb.h stdlib.h string.h paths.h sys/socket.h sys/statfs.h sys/statvfs.h sys/mnttab.h sys/loadavg.h kstat.h errno.h sys/sysinfo.h sys/processor.h sys/swap.h kvm.h alloca.h sys/resource.h netinet/in.h sys/sysctl.h sys/vmmeter.h sys/mount.h sys/param.h])

# check net/if.h seperately, as it requires other headers on atleast FreeBSD
AC_CHECK_HEADERS([net/if.h net/if_mib.h],[],[],
[[
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NET_IF_H
# include <net/if.h>
#endif
]])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_GETMNTENT
AC_CHECK_FUNCS([gethostbyname getmntent getmntinfo inet_ntoa memset mkdir select socket strerror statvfs setmntent])

# Try to figure out which type of mntent structure we're dealing with
AC_CHECK_MEMBER([struct mnttab.mnt_special],[AC_DEFINE_UNQUOTED([USE_STRUCT_MNTTAB],[1],[define to use 'struct mnttab'])],,[
#include <stdio.h>
#include <sys/mnttab.h>
])
AC_CHECK_MEMBER([struct mntent.mnt_fsname],[AC_DEFINE_UNQUOTED([USE_STRUCT_MNTENT],[1],[define to use 'struct mntent'])],,[
#include <stdio.h>
#include <mntent.h>
])
AC_CHECK_MEMBER([struct statvfs.f_frsize],[AC_DEFINE_UNQUOTED([HAVE_STATVFS_FRSIZE],[1],[define if statvfs has the f_frsize member])],,[
#include <sys/types.h>
#include <sys/statvfs.h>
])

AC_MSG_CHECKING([for default location of configuration file])
test x"$prefix" = x"NONE" && prefix="$ac_default_prefix"
configpath=`eval echo "$sysconfdir"/istat.conf`
AC_MSG_RESULT([$configpath])
AC_DEFINE_UNQUOTED([CONFIG_FILE_PATH],["${configpath}"],[default location of the configuration file])

AC_MSG_CHECKING([CPU usage probe type])
case $use_cpu in
	procfs)
		AC_DEFINE_UNQUOTED([USE_CPU_PROCFS],1,[Use procfs to probe CPU usage])
		;;
	kstat)
		AC_DEFINE_UNQUOTED([USE_CPU_KSTAT],1,[Use kstat(3) to probe CPU usage])
		;;
	sysctl)
		AC_DEFINE_UNQUOTED([USE_CPU_SYSCTL],1,[Use sysctl to probe CPU usage])
		;;
	*)
		AC_MSG_ERROR([cannot determine CPU usage probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_cpu)

AC_MSG_CHECKING([load average probe type])
case $use_load in
    procfs)
        AC_DEFINE_UNQUOTED([USE_LOAD_PROCFS],1,[Use procfs to probe load average])
        ;;
	getloadavg)
		AC_DEFINE_UNQUOTED([USE_LOAD_GETLOADAVG],1,[Use getloadavg(3) to probe load average])
		;;
	*)
		AC_MSG_ERROR([cannot determine load average probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_load)

AC_MSG_CHECKING([memory usage probe type])
case $use_mem in
	procfs)
		AC_DEFINE_UNQUOTED([USE_MEM_PROCFS],1,[Use procfs to probe memory usage])
		;;
	kstat)
		AC_DEFINE_UNQUOTED([USE_MEM_KSTAT],1,[Use kstat(3) to probe memory usage])
		;;
	kvm)
		AC_DEFINE_UNQUOTED([USE_MEM_KVM],1,[Use kvm(3) to probe memory usage])
		;;
	*)
		AC_MSG_ERROR([cannot determine memory usage probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_mem)

AC_MSG_CHECKING([swap probe type])
case $use_swap in
	procfs)
		AC_DEFINE_UNQUOTED([USE_SWAP_PROCFS],1,[Use procfs to probe swap])
		;;
	swapctl)
		AC_DEFINE_UNQUOTED([USE_SWAP_SWAPCTL],1,[Use swapctl(3) to probe swap])
		;;
	kvm)
		AC_DEFINE_UNQUOTED([USE_SWAP_KVM],1,[Use kvm(3) to probe swap])
		;;
	*)
		AC_MSG_ERROR([cannot determine swap probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_swap)

AC_MSG_CHECKING([uptime probe type])
case $use_uptime in
	procfs)
		AC_DEFINE_UNQUOTED([USE_UPTIME_PROCFS],1,[Use procfs to probe uptime])
		;;
	kstat)
		AC_DEFINE_UNQUOTED([USE_UPTIME_KSTAT],1,[Use kstat(3) to probe uptime])
		;;
	gettime)
		AC_DEFINE_UNQUOTED([USE_UPTIME_GETTIME],1,[Use clock_gettime(2) to probe load uptime])
		;;
	sysctl)
		AC_DEFINE_UNQUOTED([USE_UPTIME_SYSCTL],1,[Use sysctl(2) to probe uptime])
		;;
	*)
		AC_MSG_ERROR([cannot determine uptime probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_uptime)

AC_MSG_CHECKING([network activity probe type])
case $use_net in
	procfs)
		AC_DEFINE_UNQUOTED([USE_NET_PROCFS],1,[Use procfs to probe network activity])
		;;
	kstat)
		AC_DEFINE_UNQUOTED([USE_NET_KSTAT],1,[Use kstat(3) to probe network activity])
		;;
	sysctl)
		AC_DEFINE_UNQUOTED([USE_NET_SYSCTL],1,[Use sysctl to probe network activity])
		;;
	*)
		AC_MSG_ERROR([cannot determine network activity probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_net)

AC_MSG_CHECKING([disk space probe type])
case $use_disk in
	statfs|statvfs)
		AC_DEFINE_UNQUOTED([USE_DISK_STATFS],1,[Use statfs/statvfs(2) to probe disk space])
		;;
	*)
		AC_MSG_ERROR([cannot determine disk usage probe type for ${host_os}])
		;;
esac
AC_MSG_RESULT($use_disk)

AC_CONFIG_FILES([
Makefile
resource/Makefile
probes/Makefile
libfslabel/Makefile
])
AC_OUTPUT
