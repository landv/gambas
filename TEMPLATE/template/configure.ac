dnl ---- configure.ac for __COMPONENT

m4_include([../version.m4])
AC_INIT(gambas3-__COMPONENT_DASH, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(__COMPONENT)
AC_PROG_LIBTOOL

#if __USE_XWINDOW || __USE_QT
GB_CHECK_XWINDOW
#endif

#if __USE_QT
GB_FIND_QT_MOC
#endif

#if __USE_PKGCONFIG

GB_COMPONENT_PKG_CONFIG(
  __NAME,
  __UNAME,
  __COMPONENT,
  [src],
  __PKGCONFIG_NAME)
  
#else

#if __USE_C
#define __LIBRARY_C $C_LIB
#else
#define __LIBRARY_C
#endif
#if __USE_CPP
#define __LIBRARY_CPP $CXX_LIB
#else
#define __LIBRARY_CPP
#endif
#if __USE_THREAD
#define __LIBRARY_THREAD $THREAD_LIB
#define __INCLUDE_THREAD $THREAD_INC
#else
#define __LIBRARY_THREAD
#define __INCLUDE_THREAD
#endif

GB_COMPONENT(
  __NAME,
  __UNAME,
  __COMPONENT,
  [src],
#ifdef __SEARCH_INCLUDE
  [GB_FIND(__SEARCH_INCLUDE, __SEARCH_INCLUDE_PATH, __SEARCH_INCLUDE_DIR)],
#else
  [],
#endif
#ifdef __SEARCH_LIBRARY
  [GB_FIND(__SEARCH_LIBRARY, __SEARCH_LIBRARY_PATH, __SEARCH_LIBRARY_DIR)],
#else
  [],
#endif
  [__LIBRARY_C __LIBRARY_CPP __LIBRARY_THREAD __LIBRARY],
  [__INCLUDE_THREAD  __INCLUDE])
  
#endif
  
AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
