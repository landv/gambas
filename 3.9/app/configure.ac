dnl ---- configure.ac for Gambas development environment

dnl ---- Initialization

m4_include([../version.m4])
AC_INIT(gambas3-ide, GB_VERSION, GB_MAIL, [], GB_URL)
GB_INIT_SHORT(ide)

dnl ---- Check for Portland scripts

AC_CHECK_PROGS(XDG_UTILS, [xdg-mime xdg-icon-resource], [])

dnl ---- Create makefiles

AC_OUTPUT( \
Makefile \
)

GB_PRINT_MESSAGES
