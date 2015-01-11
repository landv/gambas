EXTRA_DIST = reconf src spec mime desktop

install-exec-local:
	@if test "x$(ROOT)" != "x"; then \
	 echo "[Installing with ROOT=$(ROOT)]"; \
	fi
	@if test "x$(DESTDIR)" != "x"; then \
	 echo "[Installing with DESTDIR=$(DESTDIR)]"; \
	 ROOT=$DESTDIR; \
	fi
	
	@echo "Installing the development environment..."
	@(cd $(srcdir)/src; d=`pwd`; \
	  for p in `cat INSTALL`; do \
	    echo "Compiling $$p..."; cd $$d/$$p; \
	    $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag -r $(DESTDIR)$(prefix); \
	    if test $$? -eq 0; then \
	      $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	      rm -rf .gambas; \
	      echo "Installing $$p..."; \
	      $(INSTALL) $$p.gambas $(DESTDIR)$(bindir); \
	    else \
	      echo "|| Unable to compile $$p" >> ../../../warnings.log; \
	    fi \
	  done)
	##@if test "$(bindir)" != "$(ROOT)/usr/bin" && test "$(bindir)" != "$(ROOT)/usr/bin/"; then
	@ln -s gambas$(GAMBAS_VERSION).gambas $(DESTDIR)$(bindir)/gambas$(GAMBAS_VERSION) || true
	
	@echo "Installing the scripter..."
	@ln -s gbs$(GAMBAS_VERSION).gambas $(DESTDIR)$(bindir)/gbs$(GAMBAS_VERSION) || true
	@ln -s gbs$(GAMBAS_VERSION).gambas $(DESTDIR)$(bindir)/gbw$(GAMBAS_VERSION) || true
	@if test x"$(XDG_UTILS)" != x; then \
		$(INSTALL) -d $(DESTDIR)$(gbdatadir)/icons; \
		cp -f $(srcdir)/mime/application-x-gambas*.png $(DESTDIR)$(gbdatadir)/icons; \
		echo "Registering Gambas script mimetype"; \
		xdg-icon-resource install --context mimetypes --size 64 $(DESTDIR)$(gbdatadir)/icons/application-x-gambasscript.png application-x-gambasscript; \
		xdg-mime install $(srcdir)/mime/application-x-gambasscript.xml; \
		echo "Registering Gambas server page mimetype"; \
		xdg-icon-resource install --context mimetypes --size 64 $(DESTDIR)$(gbdatadir)/icons/application-x-gambasserverpage.png application-x-gambasserverpage; \
		xdg-mime install $(srcdir)/mime/application-x-gambasserverpage.xml; \
	fi

	@echo "Installing the Gambas appdata file"
	@$(INSTALL) -d $(DESTDIR)$(datarootdir)/appdata
	@$(INSTALL) $(srcdir)/desktop/gambas3.appdata.xml $(DESTDIR)$(datarootdir)/appdata

uninstall-local:
	@rm -f $(DESTDIR)$(bindir)/gambas$(GAMBAS_VERSION)
	@rm -f $(DESTDIR)$(bindir)/gbs$(GAMBAS_VERSION)
	@rm -f $(DESTDIR)$(bindir)/gbw$(GAMBAS_VERSION)
	@(cd $(srcdir)/src; for p in `cat INSTALL`; do rm -f $(DESTDIR)$(bindir)/$$p.gambas; done)

dist-hook:
	@(cd $(distdir)/src; \
	rm -rf `find . -name ".gambas" -o -name ".action" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core*" -o -name ".kdbg*" -o -name ".svn"`;)

