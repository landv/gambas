COMPONENT = gb.jit
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.jit.la
noinst_LTLIBRARIES = libjit.la

libjit_la_LIBADD =
libjit_la_LDFLAGS = -module @LD_FLAGS@
libjit_la_CPPFLAGS = @JIT_INC@
libjit_la_CXXFLAGS = $(AM_CFLAGS)  -std=gnu++0x -fno-exceptions -fno-rtti

gb_jit_la_LIBADD = libjit.la @JIT_LIB@
gb_jit_la_LDFLAGS = -module @LD_FLAGS@ @JIT_LDFLAGS@
gb_jit_la_CPPFLAGS = @JIT_INC@
gb_jit_la_CXXFLAGS = $(AM_CXXFLAGS) -std=gnu++0x -fno-exceptions

libjit_la_SOURCES = \
 jit_gambas_pass.cpp \
 jit_gambas_pass.h

gb_jit_la_SOURCES = \
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
