ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = gbc gbx lib share
EXTRA_DIST = TODO reconf spec README mime gb.*.h
BUILT_SOURCES = trunk_version.h
CLEANFILES = trunk_version.h

.PHONY: trunk_version.h
trunk_version.h:
	@if test -d ../.svn ; then \
		if test ../.svn -nt trunk_version.h ; then \
			echo '#define TRUNK_VERSION "'`LC_ALL=C svn info 2>/dev/null | grep Revision | egrep -wo [0-9]+`'"' > trunk_version.h; \
		fi \
	else \
		touch trunk_version.h; \
	fi

install-exec-local:
	@rm -f $(srcdir)/../warnings.log
	
	@if test "x$(ROOT)" != "x"; then \
	 echo "[Installing with ROOT=$(ROOT)]"; \
	fi
	@if test "x$(DESTDIR)" != "x"; then \
	 echo "[Installing with DESTDIR=$(DESTDIR)]"; \
	 ROOT=$DESTDIR; \
	fi
	
	@echo "Making runtime symbolic link"
	@$(LN_S) -f gbx$(GAMBAS_VERSION) $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION) || true
	
	@if test x"$(XDG_UTILS)" != x; then \
		echo "Registering Gambas executable mimetype"; \
		$(INSTALL) -d $(DESTDIR)$(gbdatadir)/icons; \
		cp -f $(srcdir)/mime/application-x-gambas3.png $(DESTDIR)$(gbdatadir)/icons; \
		xdg-icon-resource install --context mimetypes --size 64 $(DESTDIR)$(gbdatadir)/icons/application-x-gambas3.png application-x-gambas3; \
		xdg-mime install $(srcdir)/mime/application-x-gambas3.xml; \
	fi

uninstall-local:
	@rm -f $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION)
	@rm -rf $(DESTDIR)$(gblibdir)/info
	@if test x"$(XDG_UTILS)" != x; then \
		xdg-mime uninstall $(srcdir)/mime/application-x-gambas3.xml; \
		xdg-icon-resource uninstall --context mimetypes --size 64 application-x-gambas3; \
	fi

dist-hook:
	@rm -f $(distdir)/trunk_version.h
