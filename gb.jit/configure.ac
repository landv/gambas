dnl ---- configure.ac for gb.jit

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.jit)
AC_PROG_LIBTOOL

min_llvm_version=3.1

dnl llvm-config file can be forced with LLVM_CONFIG env var
AC_PATH_PROG([LLVM_CONFIG], [llvm-config], [no])

if test "x$LLVM_CONFIG" = xno; then
  touch DISABLED DISABLED.gb.jit
else
  AC_MSG_CHECKING(for LLVM - version >= $min_llvm_version)

  LLVM_VERSION=`$LLVM_CONFIG --version`
  llvm_major_version=`echo $LLVM_VERSION | sed 's/svn.*//g' | sed 's/\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  llvm_minor_version=`echo $LLVM_VERSION | sed 's/svn.*//g' | sed 's/\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  min_llvm_major_version=`echo $min_llvm_version | sed 's/\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  min_llvm_minor_version=`echo $min_llvm_version | sed 's/\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  
  if test $llvm_major_version -ge $min_llvm_major_version &&
    test $llvm_minor_version -ge $min_llvm_minor_version; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    touch DISABLED DISABLED.gb.jit
  fi
fi

GB_COMPONENT(
  jit, JIT, gb.jit, [src],
  [GB_FIND(llvm/LLVMContext.h, /usr/lib/llvm* /usr/local/lib /usr/local /usr/lib /usr `$LLVM_CONFIG --prefix`, include)],
  [GB_FIND(libLLVM-$LLVM_VERSION.so, /usr/lib/llvm* /usr/local /usr `$LLVM_CONFIG --prefix`, lib)],
  [$C_LIB `$LLVM_CONFIG --ldflags` -lLLVM-$LLVM_VERSION],
  [-I../../main/gbx -I../../main/share -D_DEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS])

AC_OUTPUT(Makefile src/Makefile)

GB_PRINT_MESSAGES
