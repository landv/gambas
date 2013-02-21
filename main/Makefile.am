ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = gbc gbx lib share
EXTRA_DIST = TODO reconf spec README mime

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
		@xdg-mime uninstall $(srcdir)/mime/application-x-gambas3.xml;
		@xdg-icon-resource uninstall --context mimetypes --size 64 application-x-gambas3;
	fi

