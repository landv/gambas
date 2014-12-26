SUBDIRS = \
 main \
 @bzlib2_dir@ \
 @zlib_dir@ \
 @mysql_dir@ \
 @odbc_dir@ \
 @postgresql_dir@ \
 @sqlite2_dir@ \
 @sqlite3_dir@ \
 @net_dir@ \
 @curl_dir@ \
 @mime_dir@ \
 @pcre_dir@ \
 @sdl_dir@ \
 @sdlsound_dir@ \
 @sdl2_dir@ \
 @libxml_dir@ \
 @xml_dir@ \
 @v4l_dir@ \
 @crypt_dir@ \
 @qt4_dir@ \
 @gtk_dir@ \
 @gtk3_dir@ \
 @opengl_dir@ \
 @x11_dir@ \
 @keyring_dir@ \
 @pdf_dir@ \
 @cairo_dir@ \
 @imageio_dir@ \
 @imageimlib_dir@ \
 @dbus_dir@ \
 @gsl_dir@ \
 @gmp_dir@ \
 @ncurses_dir@ \
 @media_dir@ \
 @jit_dir@ \
 @httpd_dir@ \
 @openssl_dir@ \
 @openal_dir@ \
 comp \
 app \
 .

EXTRA_DIST = component.am README README.*[^~] TODO TEMPLATE reconf reconf-all

am__tar = ${AMTAR} cof - "$$tardir"

install-exec-local:
	@if test -s $(srcdir)/warnings.log; then \
	  echo ; \
	  echo "||" ; \
	  cat $(srcdir)/warnings.log ; \
	  echo "||" ; \
	  echo ; \
	fi
	@rm -f $(srcdir)/warnings.log
	 
dist-hook:
	@(cd $(distdir); rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name "vgcore.*" -o -name ".kdbg*" -o -name ".svn"`;)
