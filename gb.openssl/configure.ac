dnl ---- configure.ac for gb.openssl

m4_include([../version.m4])
AC_INIT(gambas3-gb-openssl, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.openssl)
AC_PROG_LIBTOOL

# We really only want to make sure that EVP_MD_do_all() exists.
# Don't do a version check because some systems may have the
# function from somewhere but don't have a high enough version.
#GB_COMPONENT_PKG_CONFIG(openssl, OPENSSL, gb.openssl, [src], 'openssl >= 1.0.0' libcrypto)

AC_CHECK_LIB(crypto,EVP_MD_do_all,,touch DISABLED)

GB_COMPONENT_PKG_CONFIG(openssl, OPENSSL, gb.openssl, [src], libcrypto)

AC_OUTPUT(Makefile src/Makefile)
GB_PRINT_MESSAGES
