dnl ---- configure.ac for gb.pdf

AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.pdf)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  poppler,
  POPPLER,
  [Poppler PDF library],
  [src],
  poppler ">= 0.5"
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
fi

AC_OUTPUT( \
Makefile \
src/Makefile \
)
