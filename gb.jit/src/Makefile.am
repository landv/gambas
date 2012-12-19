COMPONENT = gb.jit
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.jit.la

gb_jit_la_LIBADD = @JIT_LIB@
gb_jit_la_LDFLAGS = -module @LD_FLAGS@ @JIT_LDFLAGS@
gb_jit_la_CPPFLAGS = @JIT_INC@
gb_jit_la_CXXFLAGS = $(AM_CXXFLAGS) -std=gnu++0x -fno-exceptions

gb_jit_la_SOURCES = gb.jit.h jit_api.cpp jit_codegen_conv.h jit_codegen.cpp jit_compile.cpp jit_conv.cpp jit_expressions.cpp jit.h jit_read.cpp jit_runtime.c jit_runtime.h jit_gambas_pass.cpp jit_gambas_pass.h jit_gambas_pass_dummy.cpp main.cpp main.h
