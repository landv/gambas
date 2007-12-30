EXTRA_DIST = reconf src spec

install-exec-local:
	@echo "Installing the development environment..."
	@(cd $(srcdir)/src; d=`pwd`; \
	  for p in `cat INSTALL`; do \
	    echo "Compiling $$p..."; cd $$d/$$p; \
	    $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag -r $(DESTDIR)$(prefix); \
	    $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	    rm -rf .gambas; \
	    $(INSTALL) $$p.gambas $(DESTDIR)$(bindir); \
	  done)
	##@if test "$(bindir)" != "$(ROOT)/usr/bin" && test "$(bindir)" != "$(ROOT)/usr/bin/"; then
	@ln -s gambas$(GAMBAS_VERSION).gambas $(DESTDIR)$(bindir)/gambas$(GAMBAS_VERSION) || true

uninstall-local:
	@rm -f $(DESTDIR)$(bindir)/gambas$(GAMBAS_VERSION)
	@(cd $(srcdir)/src; for p in `cat INSTALL`; do rm -f $(DESTDIR)$(bindir)/$$p.gambas; done)

dist-hook:
	@(cd $(distdir)/src; \
	rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)

