SUBDIRS = main @bzlib2_dir@ @zlib_dir@ @mysql_dir@ @odbc_dir@ @postgresql_dir@ @sqlite3_dir@ @sqlite2_dir@ @firebird_dir@ @gtk_dir@ @net_dir@ @curl_dir@ @smtp_dir@ @pcre_dir@ @qt_dir@ @qte_dir@ @kde_dir@ @sdl_dir@ @sdlsound_dir@ @xml_dir@ @v4l_dir@ @crypt_dir@ @opengl_dir@ @corba_dir@ @pdf_dir@ @gtksvg_dir@ @desktop_dir@ @qt4_dir@ comp app help examples

EXTRA_DIST = component.am README README.*[^~] TODO TEMPLATE reconf reconf-all

am__tar = ${AMTAR} cof - "$$tardir"

dist-hook:
	@(cd $(distdir); rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)
