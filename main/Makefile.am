ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = gbc gbx lib share
EXTRA_DIST = TODO reconf spec README mime

install-exec-local:
	@if test x"$(DESTDIR)" != x; then \
		echo "Installing with DESTDIR='$(DESTDIR)'"; \
	fi
	@echo "Making runtime symbolic link"
	@$(LN_S) -f gbx$(GAMBAS_VERSION) $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION) || true
	
	@if test x"$(XDG_UTILS)" != x; then \
		echo "Registering Gambas executable mimetype"; \
		$(INSTALL) -d $(DESTDIR)$(gbdatadir)/icons; \
		cp -f $(srcdir)/mime/application-x-gambas.png $(DESTDIR)$(gbdatadir)/icons; \
		xdg-icon-resource install --context mimetypes --size 64 $(DESTDIR)$(gbdatadir)/icons/application-x-gambas.png application-x-gambas; \
		xdg-mime install $(srcdir)/mime/application-x-gambas.xml; \
	fi

uninstall-local:
	@rm -f $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION)
	@rm -rf $(DESTDIR)$(gblibdir)/info
