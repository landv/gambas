dnl ---- configure.ac for gb.pdf

m4_include([../version.m4])
AC_INIT(gambas3-gb-pdf, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.pdf)
AC_PROG_LIBTOOL

GB_COMPONENT_SEARCH(
  poppler, POPPLER, gb.pdf, [src],
  'poppler >= 0.5',
  [GB_FIND(PDFDoc.h, `pkg-config --variable=includedir poppler`, poppler)],
  [],
  [],
  []
)

if test "$have_poppler" = "yes"; then
  pkg-config --atleast-version=0.5 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_5, $((1-$?)), Poppler version >= 0.5)
  pkg-config --atleast-version=0.5.91 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_6, $((1-$?)), Poppler version >= 0.6)
  pkg-config --atleast-version=0.8.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_8, $((1-$?)), Poppler version >= 0.8)
  pkg-config --atleast-version=0.11.3 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_11_3, $((1-$?)), Poppler version >= 0.11.3)
  pkg-config --atleast-version=0.17.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_17, $((1-$?)), Poppler version >= 0.17)
  pkg-config --atleast-version=0.20.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_20, $((1-$?)), Poppler version >= 0.20)
fi

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
