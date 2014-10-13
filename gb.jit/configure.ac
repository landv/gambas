dnl ---- configure.ac for gb.jit

m4_include([../version.m4])
AC_INIT(gambas3-gb-jit, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.jit)
AC_PROG_LIBTOOL

min_llvm_version=3.1

dnl llvm-config file can be forced with LLVM_CONFIG env var
AC_PATH_PROG([LLVM_CONFIG], [llvm-config], [no])

if test "x$LLVM_CONFIG" = xno; then
  touch DISABLED DISABLED.gb.jit
else
  AC_MSG_CHECKING([for LLVM, version >= $min_llvm_version])

  LLVM_VERSION=`$LLVM_CONFIG --version`

  AX_COMPARE_VERSION($LLVM_VERSION, [ge], $min_llvm_version,
   [AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)
    touch DISABLED DISABLED.gb.jit])
fi


GB_COMPONENT(
  jit, JIT, gb.jit, [src],
  [GB_FIND(llvm/Config/llvm-config.h llvm-c/Core.h, `$LLVM_CONFIG --prefix` /usr/lib/llvm* /usr/local /usr, include)],
  [GB_FIND(libLLVM-$LLVM_VERSION.$SHLIBEXT, `$LLVM_CONFIG --prefix` /usr/lib/llvm* /usr/local /usr, lib)],
  [$C_LIB `$LLVM_CONFIG --ldflags` -lLLVM-$LLVM_VERSION],
  [-I../../main/gbx -I../../main/share -D_DEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS])

AC_OUTPUT(Makefile src/Makefile)

GB_PRINT_MESSAGES
