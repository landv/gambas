EXTRA_DIST = reconf examples spec

install-exec-local:
	@if test "x$(ROOT)" != "x"; then \
	 echo "[Installing with ROOT=$(ROOT)]"; \
	fi
	@if test "x$(DESTDIR)" != "x"; then \
	 echo "[Installing with DESTDIR=$(DESTDIR)]"; \
	 ROOT=$DESTDIR; \
	fi
	
	@echo
	@echo "Installing the gambas examples..."
	@rm -rf $(DESTDIR)$(gbdatadir)/examples
	@$(INSTALL) -d $(DESTDIR)$(gbdatadir)/examples
	@echo "Copying..."
	@cp -R $(srcdir)/examples $(DESTDIR)$(gbdatadir)
	@(cd $(DESTDIR)$(gbdatadir)/examples; d=`pwd`; for p in */ */*/; do cd $$d/$$p; \
	  if test -e .project; then \
	    ## Remove the possible environment variables defined in the project property dialog
	    cat .project | grep -v "^Environment=" > .project.new; \
	    rm .project; \
	    mv .project.new .project; \
	    echo "Compiling $$p..."; cd $$d/$$p; $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag -r $(DESTDIR)$(prefix); $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	  fi \
	done)

uninstall-local:
	@rm -rf $(DESTDIR)$(gbdatadir)/examples

dist-hook:
	@(cd $(distdir)/examples; \
	rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core*" -o -name ".kdbg*" -o -name ".svn"`;)

