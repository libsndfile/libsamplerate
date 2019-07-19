# Copyright (C) 2002-2017 Erik de Castro Lopo (erikd AT mega-nerd DOT com).

dnl Require autoconf version >= 2.69)
AC_PREREQ([2.69])

AC_INIT([libsamplerate],[0.1.9],[erikd@mega-nerd.com],
		[libsamplerate],[http://www.mega-nerd.com/libsamplerate/])

dnl Check whether we want to set defaults for CFLAGS, CPPFLAGS and LDFLAGS
AC_MSG_CHECKING([whether configure should try to set CFLAGS/CPPFLAGS/LDFLAGS])
AS_IF([test "x${CFLAGS+set}" = "xset" || test "x${CPPFLAGS+set}" = "xset" || test "x${LDFLAGS+set}" = "xset"], [
		enable_flags_setting=no
		: ${CFLAGS=""}
	], [
		enable_flags_setting=yes
		dnl Set to empty flags so AC_PROG_CC does not add -g -O2
		CFLAGS=""
	])
AC_MSG_RESULT([${enable_flags_setting}])

dnl Put config stuff in 'build-aux'.
AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_SRCDIR([src/samplerate.c])
AC_CANONICAL_HOST

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE([1.14 foreign dist-bzip2 no-dist-gzip subdir-objects])
AM_SILENT_RULES([yes])

dnl ====================================================================================

AC_PROG_CC
AC_PROG_CC_C99

AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [
		AC_MSG_ERROR([libsamplerate requires a C99 capable compiler!])
	])

AC_LANG([C])
AX_COMPILER_VENDOR
AX_COMPILER_VERSION

AC_PROG_SED
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

PKG_PROG_PKG_CONFIG
dnl Catch ancient versions of pkg-config below 0.27
AX_REQUIRE_DEFINED([PKG_INSTALLDIR])
PKG_INSTALLDIR

LT_INIT
LT_PROG_RC

dnl ====================================================================================
dnl  Rules for library version information:
dnl
dnl   1. Start with version information of `0:0:0' for each libtool library.
dnl   2. Update the version information only immediately before a public release of
dnl      your software. More frequent updates are unnecessary, and only guarantee
dnl      that the current interface number gets larger faster.
dnl   3. If the library source code has changed at all since the last update, then
dnl      increment revision (`c:r:a' becomes `c:r+1:a').
dnl   4. If any interfaces have been added, removed, or changed since the last update,
dnl      increment current, and set revision to 0.
dnl   5. If any interfaces have been added since the last public release, then increment
dnl      age.
dnl   6. If any interfaces have been removed since the last public release, then set age
dnl      to 0.

CLEAN_VERSION=`echo $PACKAGE_VERSION | $SED "s/p.*//"`
VERSION_MINOR=`echo $CLEAN_VERSION | $SED "s/.*\.//"`

SHARED_VERSION_INFO="1:$VERSION_MINOR:1"

dnl ====================================================================================

AS_IF([test "x${build_os}${host_os}" = "xmingw32mingw32"], [
		AC_CHECK_PROG(DLLTOOL, dlltool, dlltool)
		AC_CHECK_PROG(DLLWRAP, dllwrap, dllwrap)
		AC_CHECK_PROG(PEXPORTS, pexports, pexports)
	])

AC_HEADER_STDC

AC_CHECK_HEADERS([stdint.h sys/times.h])

dnl ====================================================================================
dnl  Couple of initializations here. Fill in real values later.

SHLIB_VERSION_ARG=""

dnl ====================================================================================
dnl  Finished checking, handle options.

AC_ARG_ENABLE([werror],
	[AS_HELP_STRING([--enable-werror], [enable -Werror in all Makefiles])])

AC_ARG_ENABLE([cpu-clip],
	[AS_HELP_STRING([--disable-cpu-clip], [disable tricky cpu specific clipper])])

AC_ARG_ENABLE([sndfile],
	[AS_HELP_STRING([--disable-sndfile], [disable support for sndfile (default=autodetect)])], [], [enable_sndfile=auto])

AC_ARG_ENABLE([alsa],
	[AS_HELP_STRING([--disable-alsa], [disable ALSA support in the varispeed-play example program (default=autodetect)])], [], [enable_alsa=auto])

AC_ARG_ENABLE([fftw],
	[AS_HELP_STRING([--disable-fftw], [disable usage of FFTW (default=autodetect)])], [], [enable_fftw=auto])

dnl ====================================================================================
dnl  Check types and their sizes.

AC_CHECK_SIZEOF(int,0)
AC_CHECK_SIZEOF(long,0)
AC_CHECK_SIZEOF(float,4)
AC_CHECK_SIZEOF(double,8)

dnl ====================================================================================
dnl  Determine endian-ness of host processor.

AC_C_BIGENDIAN([
		dnl big-endian
		ac_cv_c_big_endian=1
		ac_cv_c_little_endian=0
	], [
		dnl little-endian
		ac_cv_c_big_endian=0
		ac_cv_c_little_endian=1
	])

AC_DEFINE_UNQUOTED([CPU_IS_BIG_ENDIAN], [${ac_cv_c_big_endian}], [Host processor is big endian.])
AC_DEFINE_UNQUOTED([CPU_IS_LITTLE_ENDIAN], [${ac_cv_c_little_endian}], [Host processor is little endian.])

dnl ====================================================================================
dnl  Check for functions.

AC_CHECK_FUNCS([malloc calloc free memcpy memmove alarm signal])

AC_SEARCH_LIBS([floor], [m], [], [
		AC_MSG_ERROR([unable to find the floor() function!])
	])
AC_CHECK_FUNCS([floor ceil fmod lrint lrintf])

AC_CHECK_SIGNAL(SIGALRM)

dnl ====================================================================================
dnl  Determine if the processor can do clipping on float to int conversions.

AS_IF([test "x$enable_cpu_clip" != "xno"], [
		AC_C_CLIP_MODE
	], [
		AS_ECHO(["checking processor clipping capabilities... disabled"])
		ac_cv_c_clip_positive=0
		ac_cv_c_clip_negative=0
	])

AC_DEFINE_UNQUOTED([CPU_CLIPS_POSITIVE], [${ac_cv_c_clip_positive}], [Host processor clips on positive float to int conversion.])
AC_DEFINE_UNQUOTED([CPU_CLIPS_NEGATIVE], [${ac_cv_c_clip_negative}], [Host processor clips on negative float to int conversion.])

dnl ====================================================================================
dnl  Check for libsndfile which is required for the test and example programs.

AS_IF([test "x$enable_sndfile" != "xno"], [
		PKG_CHECK_MODULES([SNDFILE], [sndfile >= 1.0.6], [
				AC_DEFINE([HAVE_SNDFILE], [1], [Set to 1 if you have libsndfile])
				enable_sndfile="yes"
			], [
				AS_IF([test "x$enable_sndfile" = "xyes"], [
						dnl explicitly passed --enable-sndfile, hence error out loud and clearly
						AC_MSG_ERROR([You explicitly requested libsndfile support, but libsndfile could not be found!])
					], [
						dnl did not explicitly pass --enable-sndfile, relying on default automagic on
						enable_sndfile="no (auto)"
					])
			])
	])
AM_CONDITIONAL([HAVE_LIBSNDFILE], [test "x$enable_sndfile" = "xyes"])

dnl ====================================================================================
dnl  Check for ALSA.

AS_IF([test "x$enable_alsa" != "xno"], [
		PKG_CHECK_MODULES([ALSA], [alsa], [
				AC_DEFINE([HAVE_ALSA], [1], [Set to 1 if you have alsa])
				enable_alsa="yes"
			], [
				AS_IF([test "x$enable_alsa" = "xyes"], [
						dnl explicitly passed --enable-alsa, hence error out loud and clearly
						AC_MSG_ERROR([You explicitly requested alsa support, but alsa could not be found!])
					], [
						dnl did not explicitly pass --enable-alsa, relying on default automagic on
						enable_alsa="no (auto)"
					])
			])
	])
AM_CONDITIONAL([HAVE_LIBALSA], [test "x$enable_alsa" = "xyes"])

dnl ====================================================================================
dnl  Check for libfftw3 which is required for the test and example programs.

AS_IF([test "x$enable_fftw" != "xno"], [
		PKG_CHECK_MODULES([FFTW3], [fftw3 >= 3.0.0], [
				AC_DEFINE([HAVE_FFTW3], [1], [Set to 1 if you have fftw3])
				enable_fftw="yes"
			], [
				AS_IF([test "x$enable_fftw" = "xyes"], [
						dnl explicitly passed --enable-fftw, hence error out loud and clearly
						AC_MSG_ERROR([You explicitly requested FFTW3 support, but FFTW3 could not be found!])
					], [
						dnl did not explicitly pass --enable-fftw, relying on default automagic on
						enable_fftw="no (auto)"
					])
			])
	])

dnl ====================================================================================
dnl  Compiler stuff.

AS_IF([test "x$enable_flags_setting" = "xyes"], [
		AX_APPEND_COMPILE_FLAGS([-DHAVE_CONFIG_H -O2 -pipe], [CFLAGS])

		AS_CASE([${host_os}],
			[darwin*], [
				ldflags_test="-Wl,-dead_strip_dylibs"],
			[linux*], [
				ldflags_test="-Wl,-O1 -Wl,--as-needed -Wl,--no-undefined -Wl,--gc-sections"]
			)
		AX_APPEND_LINK_FLAGS([${ldflags_test}], [LDFLAGS])
	])

AS_IF([test "x$enable_werror" = "xyes"], [
		AX_APPEND_COMPILE_FLAGS([-Werror], [CFLAGS])
	])

AX_APPEND_COMPILE_FLAGS([-W -Wstrict-prototypes -Wmissing-prototypes -Wall -Waggregate-return -Wcast-align -Wcast-qual -Wnested-externs -Wshadow -Wpointer-arith -Wshorten-64-to-32], [CFLAGS])

dnl ====================================================================================
dnl  GCC stuff.

AS_IF([test "x$ax_cv_c_compiler_vendor" = "xgnu"], [
		# OS specific tweaks.
		# Disable -Wall for Apple Darwin/Rhapsody.
		# System headers on these systems are broken.
		AS_CASE([${host_os}],
			[darwin* | rhapsody*], [
				temp_CFLAGS=`echo $CFLAGS | sed "s/-Wall//"`
				CFLAGS=$temp_CFLAGS],
			[linux*|kfreebsd*-gnu*|gnu*], [
				SHLIB_VERSION_ARG="-Wl,--version-script=\$(top_builddir)/src/Version_script"
			])
	])

dnl ====================================================================================
dnl  Find known host OS.

OS_SPECIFIC_INCLUDES=""
os_is_win32=0

AS_CASE([${host_os}],
	[darwin* | rhapsody*], [
		OS_SPECIFIC_INCLUDES="-fpascal-strings -I/Developer/Headers/FlatCarbon"
		OS_SPECIFIC_LINKS="-framework CoreAudio"],
	[mingw32*], [
		OS_SPECIFIC_LINKS="-lwinmm"
		os_is_win32=1],
	[
		OS_SPECIFIC_INCLUDES=""
		OS_SPECIFIC_LINKS=""
	])

dnl ====================================================================================
dnl  Now use the information from the checking stage.

AC_DEFINE_UNQUOTED([OS_IS_WIN32], [${os_is_win32}], [Set to 1 if compiling for Win32])

AC_SUBST(SHLIB_VERSION_ARG)
AC_SUBST(SHARED_VERSION_INFO)
AC_SUBST(OS_SPECIFIC_INCLUDES)
AC_SUBST(OS_SPECIFIC_LINKS)

AC_CONFIG_FILES([
		Makefile
		src/Version_script
		Win32/Makefile.mingw
		libsamplerate.spec
		samplerate.pc
	])
AC_OUTPUT

dnl ====================================================================================

AX_RECURSIVE_EVAL([$libdir], [full_absolute_libdir])
AX_RECURSIVE_EVAL([$bindir], [full_absolute_bindir])
AX_RECURSIVE_EVAL([$pkgconfigdir], [full_absolute_pkgconfigdir])
AX_RECURSIVE_EVAL([$htmldir], [full_absolute_htmldir])
AC_MSG_RESULT([
-=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-=-

  Configuration summary :

    libsamplerate version : ............... ${VERSION}

    Host CPU : ............................ ${host_cpu}
    Host Vendor : ......................... ${host_vendor}
    Host OS : ............................. ${host_os}

    CFLAGS : .............................. ${CFLAGS}
    CPPFLAGS : ............................ ${CPPFLAGS}
    LDFLAGS : ............................. ${LDFLAGS}

  Tools :

    C Compiler Vendor is : ................ ${ax_cv_c_compiler_vendor} (${ax_cv_c_compiler_version})

  Extra tools required for testing and examples :

    Have libsndfile : ..................... ${enable_sndfile}
    Have ALSA : ........................... ${enable_alsa}
    Have FFTW : ........................... ${enable_fftw}

  Installation directories :

    Library directory : ................... ${full_absolute_libdir}
    Program directory : ................... ${full_absolute_bindir}
    Pkgconfig directory : ................. ${full_absolute_pkgconfigdir}
    HTML docs directory : ................. ${full_absolute_htmldir}

Compiling some other packages against libsamplerate may require
the addition of '$full_absolute_pkgconfigdir' to the
PKG_CONFIG_PATH environment variable.
])
