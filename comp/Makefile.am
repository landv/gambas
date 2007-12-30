## Process this file with automake to produce Makefile.in

EXTRA_DIST = reconf src spec
##AUTOMAKE_OPTIONS = dist-bzip2

install-exec-local:
	@if test "x$(ROOT)" != "x"; then \
	 echo "Installing with ROOT=$(ROOT)"; \
         echo; \
        fi

	@if test "x$(DESTDIR)" != "x"; then \
	 echo "Installing with DESTDIR=$(DESTDIR)"; \
         echo; \
	 ROOT=$DESTDIR; \
        fi
	
	@echo "Installing the components..."
	@(cd $(srcdir)/src; d=`pwd`; for p in `cat order`; do \
	  echo "Compiling $$p..."; \
	  cd $$d/$$p; \
	  $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag; \
	  $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	  rm -rf .gambas; \
	  echo "Installing $$p..."; \
	  $(INSTALL) $$p.gambas $(DESTDIR)$(gblibdir); \
	  $(INSTALL) .component $(DESTDIR)$(gblibdir)/$$p.component; \
	  chmod a-x $(DESTDIR)$(gblibdir)/$$p.component; \
	  $(INSTALL) .info $(DESTDIR)$(gbdatadir)/info/$$p.info; \
	  chmod a-x $(DESTDIR)$(gbdatadir)/info/$$p.info; \
	  $(INSTALL) .list $(DESTDIR)$(gbdatadir)/info/$$p.list; \
	  chmod a-x $(DESTDIR)$(gbdatadir)/info/$$p.list; \
	  done)

uninstall-local:
	@(cd $(srcdir)/src; for p in *; do \
	  rm -f $(DESTDIR)$(gblibdir)/$$p.gambas; \
	  rm -f $(DESTDIR)$(gblibdir)/$$p.component; \
	  rm -f $(DESTDIR)$(gbdatadir)/info/$$p.info; \
	  rm -f $(DESTDIR)$(gbdatadir)/info/$$p.list; \
	  done)

dist-hook:
	@(cd $(distdir)/src; rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)

