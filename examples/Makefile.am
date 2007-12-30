## Process this file with automake to produce Makefile.in

EXTRA_DIST = reconf examples spec

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

	@echo
	@echo "Installing the gambas examples..."
	@rm -rf $(DESTDIR)$(gbdatadir)/examples
	@$(INSTALL) -d $(DESTDIR)$(gbdatadir)/examples
	@cp -R $(srcdir)/examples $(DESTDIR)$(gbdatadir)
	@(cd $(DESTDIR)$(gbdatadir)/examples; d=`pwd`; for p in */ */*/; do cd $$d/$$p; \
	  if test -e .project; then \
	    echo "Compiling $$p..."; cd $$d/$$p; $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag -r $(DESTDIR)$(prefix); $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	  fi \
	done)

uninstall-local:
	@rm -rf $(DESTDIR)$(gbdatadir)/examples

dist-hook:
	@(cd $(distdir)/examples; \
	rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)
	
##	@(cd $(distdir)/examples/examples; for p in */ */*/; do \
##	  if test -e $$p/.project; then \
##	    rm -rf $$p/core.* $$p/.gambas $$p/.xvpics  $$p/*/.xvpics $$p/*/*/.xvpics $$p/*~ $$p/*/*~ $$p/.*.out $$p/*.out $$p/.lang/*.pot $$p/$$p.gambas .kdbg* .lock; \
##	  fi \
##	done)

