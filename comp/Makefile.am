EXTRA_DIST = reconf src spec
##AUTOMAKE_OPTIONS = dist-bzip2

install-exec-local:
	@if test "x$(ROOT)" != "x"; then \
	 echo "[Installing with ROOT=$(ROOT)]"; \
	fi
	@if test "x$(DESTDIR)" != "x"; then \
	 echo "[Installing with DESTDIR=$(DESTDIR)]"; \
	 ROOT=$DESTDIR; \
	fi
	
	@echo "Installing the components..."
	@(cd $(srcdir)/src; d=`pwd`; for p in `cat order`; do \
	  echo "Compiling $$p..."; \
	  cd $$d/$$p; \
	  $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag -r $(DESTDIR)$(prefix); \
	  if test $$? -eq 0; then \
	    $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	    rm -rf .gambas; \
	    echo "Installing $$p..."; \
	    rm -f $(DESTDIR)$(gblibdir)/$$p.so $(DESTDIR)$(gblibdir)/$$p.so.* $(DESTDIR)$(gblibdir)/$$p.la; \
	    $(INSTALL) $$p.gambas $(DESTDIR)$(gblibdir); \
	    $(INSTALL) .component $(DESTDIR)$(gblibdir)/$$p.component; \
	    chmod a-x $(DESTDIR)$(gblibdir)/$$p.component; \
	    $(INSTALL) .info $(DESTDIR)$(gbdatadir)/info/$$p.info; \
	    chmod a-x $(DESTDIR)$(gbdatadir)/info/$$p.info; \
	    $(INSTALL) .list $(DESTDIR)$(gbdatadir)/info/$$p.list; \
	    chmod a-x $(DESTDIR)$(gbdatadir)/info/$$p.list; \
	    if test -d .hidden/control; then \
	      $(INSTALL) -d $(DESTDIR)$(gbdatadir)/control; \
	      $(INSTALL) -d $(DESTDIR)$(gbdatadir)/control/$$p; \
	      $(INSTALL) .hidden/control/*.png $(DESTDIR)$(gbdatadir)/control/$$p; \
	    fi; \
	    $(DESTDIR)$(bindir)/gbi$(GAMBAS_VERSION) -r $(DESTDIR)$(prefix) $$p > /dev/null; \
	    $(DESTDIR)$(bindir)/gbi$(GAMBAS_VERSION) -r $(DESTDIR)$(prefix) > /dev/null; \
	  else \
	    echo "|| Unable to compile $$p" >> ../../../warnings.log; \
	  fi \
	  done; true)

uninstall-local:
	@(cd $(srcdir)/src; for p in *; do \
	  rm -f $(DESTDIR)$(gblibdir)/$$p.gambas; \
	  rm -f $(DESTDIR)$(gblibdir)/$$p.component; \
	  rm -f $(DESTDIR)$(gbdatadir)/info/$$p.info; \
	  rm -f $(DESTDIR)$(gbdatadir)/info/$$p.list; \
	  done)

dist-hook:
	@(cd $(distdir)/src; rm -rf `find . -name ".gambas" -o -name ".action" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core*" -o -name ".kdbg*" -o -name ".svn"`;)

