dnl ---- configure.ac for gb.jit

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.jit)
AC_PROG_LIBTOOL
AM_CXXFLAGS="$AM_CXXFLAGS -std=gnu++0x"

LLVM_CONFIG="llvm-config"

if [[ -f /usr/lib/llvm-3.0/bin/llvm-config ]]; then
  LLVM_CONFIG="/usr/lib/llvm-3.0/bin/llvm-config"
fi

GB_COMPONENT(
  jit,
  JIT,
  [Gambas-JIT-Compiler],
  [src],
  [GB_FIND(llvm/LLVMContext.h, /usr/lib/llvm-3.0 /usr/local/lib /usr/local /usr/lib /usr `$LLVM_CONFIG --prefix`, include)],
  [GB_FIND(libLLVM-3.2svn.so, /usr/lib/llvm-3.0 /usr/local /usr `$LLVM_CONFIG --prefix`, lib)],
  [$C_LIB `$LLVM_CONFIG --ldflags` -lLLVM-3.2svn],
  [ -I../../main/gbx -I../../main/share -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
