dnl ---- configure.ac for gb.jit

m4_include([../version.m4])
AC_INIT(gambas3-gb-jit-llvm, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.jit.llvm)
AC_PROG_LIBTOOL

min_llvm_version=3.1
max_llvm_version=3.5
next_max_llvm_version=3.6

dnl llvm-config file can be forced with LLVM_CONFIG env var
if test "x$LLVM_CONFIG" = x; then
  AC_PATH_PROG([LLVM_CONFIG], [llvm-config], [no])
fi

if test "x$LLVM_CONFIG" = xno; then
  touch DISABLED DISABLED.gb.jit.llvm
else
  AC_MSG_CHECKING([for LLVM, version between $min_llvm_version and $max_llvm_version])

  LLVM_VERSION=`$LLVM_CONFIG --version`

  AX_COMPARE_VERSION($LLVM_VERSION, [ge], $min_llvm_version, [min_llvm_version_ok=y], [min_llvm_version_ok=n])
  AX_COMPARE_VERSION($LLVM_VERSION, [lt], $next_max_llvm_version, [max_llvm_version_ok=y], [max_llvm_version_ok=n])

  if test "x$min_llvm_version_ok$max_llvm_version_ok" = xyy; then
    AC_MSG_RESULT([yes ($LLVM_VERSION)])
    
    if test "x$LLVM_LIBS" = x; then
      LLVM_LIBS=-lLLVM-$LLVM_VERSION
    fi
    
  else
    AC_MSG_RESULT(no)
    touch DISABLED DISABLED.gb.jit.llvm
  fi
fi

dnl  [GB_FIND(libLLVM-$LLVM_VERSION.$SHLIBEXT, `$LLVM_CONFIG --prefix` /usr/lib/llvm* /usr/local /usr, lib)],

GB_COMPONENT(
  jitllvm, JITLLVM, gb.jit.llvm, [src],
  [GB_FIND(llvm/ExecutionEngine/JIT.h llvm/Config/llvm-config.h llvm-c/Core.h, `$LLVM_CONFIG --prefix` /usr/lib/llvm* /usr/local /usr, include)],
  [],
  [$C_LIB `$LLVM_CONFIG --ldflags` $LLVM_LIBS],
  [-I../../main/gbx -I../../main/share -D_DEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS])

AC_OUTPUT(Makefile src/Makefile)

GB_PRINT_MESSAGES
