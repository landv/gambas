COMPONENT = gb.jit.llvm
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.jit.llvm.la
noinst_LTLIBRARIES = libjit.llvm.la

libjit_llvm_la_LIBADD =
libjit_llvm_la_LDFLAGS = -module @LD_FLAGS@
libjit_llvm_la_CPPFLAGS = @JITLLVM_INC@
libjit_llvm_la_CXXFLAGS = $(AM_CFLAGS)  -std=gnu++0x -fno-exceptions -fno-rtti

gb_jit_llvm_la_LIBADD = libjit.la @JITLLVM_LIB@
gb_jit_llvm_la_LDFLAGS = -module @LD_FLAGS@ @JITLLVM_LDFLAGS@
gb_jit_llvm_la_CPPFLAGS = @JITLLVM_INC@
gb_jit_llvm_la_CXXFLAGS = $(AM_CXXFLAGS) -std=gnu++0x -fno-exceptions

libjit_llvm_la_SOURCES = \
 jit_gambas_pass.cpp \
 jit_gambas_pass.h

gb_jit_llvm_la_SOURCES = \
 gb.jit.h \
 jit_api.cpp \
 jit_codegen_conv.h \
 jit_codegen.cpp \
 jit_compile.cpp \
 jit_conv.cpp \
 jit_expressions.cpp \
 jit.h \
 jit_read.cpp \
 jit_runtime.c \
 jit_runtime.h \
 main.cpp \
 main.h
