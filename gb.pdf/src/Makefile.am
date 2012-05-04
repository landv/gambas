COMPONENT = gb.pdf
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.pdf.la

gb_pdf_la_LIBADD = @POPPLER_LIB@
gb_pdf_la_LDFLAGS = -module @LD_FLAGS@ @POPPLER_LDFLAGS@
gb_pdf_la_CPPFLAGS = @POPPLER_INC@

gb_pdf_la_SOURCES = main.h main.cpp \
                        CPdfDocument.h CPdfDocument.cpp 



 
