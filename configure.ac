dnl ****************************************************************************
dnl   global configure.ac
dnl   (c) Benoît Minisini 2000-2009
dnl ****************************************************************************

dnl ---- Initialization

m4_include([version.m4])
AC_INIT(gambas3, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SUBDIRS(main)

GB_CONFIG_SUBDIRS(bzlib2, gb.compress.bzlib2)
GB_CONFIG_SUBDIRS(zlib, gb.compress.zlib)
GB_CONFIG_SUBDIRS(mysql, gb.db.mysql)
GB_CONFIG_SUBDIRS(odbc, gb.db.odbc)
GB_CONFIG_SUBDIRS(postgresql, gb.db.postgresql)
GB_CONFIG_SUBDIRS(sqlite2, gb.db.sqlite2)
GB_CONFIG_SUBDIRS(sqlite3, gb.db.sqlite3)
GB_CONFIG_SUBDIRS(net, gb.net)
GB_CONFIG_SUBDIRS(curl, gb.net.curl)
GB_CONFIG_SUBDIRS(mime, gb.mime)
GB_CONFIG_SUBDIRS(pcre, gb.pcre)
GB_CONFIG_SUBDIRS(sdl, gb.sdl)
GB_CONFIG_SUBDIRS(sdlsound, gb.sdl.sound)
GB_CONFIG_SUBDIRS(sdl2, gb.sdl2)
GB_CONFIG_SUBDIRS(libxml, gb.libxml)
GB_CONFIG_SUBDIRS(xml, gb.xml)
GB_CONFIG_SUBDIRS(v4l, gb.v4l)
GB_CONFIG_SUBDIRS(crypt, gb.crypt)
GB_CONFIG_SUBDIRS(qt4, gb.qt4)
GB_CONFIG_SUBDIRS(gtk, gb.gtk)
GB_CONFIG_SUBDIRS(gtk3, gb.gtk3)
GB_CONFIG_SUBDIRS(opengl, gb.opengl)
GB_CONFIG_SUBDIRS(x11, gb.desktop.x11)
GB_CONFIG_SUBDIRS(keyring, gb.desktop.gnome.keyring)
GB_CONFIG_SUBDIRS(pdf, gb.pdf)
GB_CONFIG_SUBDIRS(cairo, gb.cairo)
GB_CONFIG_SUBDIRS(imageio, gb.image.io)
GB_CONFIG_SUBDIRS(imageimlib, gb.image.imlib)
GB_CONFIG_SUBDIRS(dbus, gb.dbus)
GB_CONFIG_SUBDIRS(gsl, gb.gsl)
GB_CONFIG_SUBDIRS(gmp, gb.gmp)
GB_CONFIG_SUBDIRS(ncurses, gb.ncurses)
GB_CONFIG_SUBDIRS(media, gb.media)
GB_CONFIG_SUBDIRS(jit, gb.jit)
GB_CONFIG_SUBDIRS(httpd, gb.httpd)
GB_CONFIG_SUBDIRS(openssl, gb.openssl)
GB_CONFIG_SUBDIRS(openal, gb.openal)

AC_CONFIG_SUBDIRS(comp)
AC_CONFIG_SUBDIRS(app)

AM_MAINTAINER_MODE

GB_INIT_AUTOMAKE(gambas3)

AC_CANONICAL_HOST
  
GAMBAS_VERSION=3
AC_SUBST(GAMBAS_VERSION)
  
gbbindir=$bindir/gambas$GAMBAS_VERSION/
AC_SUBST(gbbindir)
gblibdir=$libdir/gambas$GAMBAS_VERSION/
AC_SUBST(gblibdir)
gbdatadir=$datadir/gambas$GAMBAS_VERSION/
AC_SUBST(gbdatadir)

AC_OUTPUT(Makefile)

GB_CLEAR_MESSAGES

for gb_dir in gb.*; do
  if test -e $gb_dir/DISABLED; then
    for gb_comp in $gb_dir/DISABLED.*; do
      if ! test -s $srcdir/warnings.log; then
        GB_MESSAGE([THESE COMPONENTS ARE DISABLED:])
      fi
      gb_comp=`basename "$gb_comp"`
      GB_MESSAGE([- ${gb_comp#DISABLED.}])
    done
  fi
done

if ! test -s $srcdir/warnings.log; then
  GB_MESSAGE([ALL COMPONENTS WILL BE COMPILED])
fi

GB_PRINT_MESSAGES
echo
GB_CLEAR_MESSAGES
