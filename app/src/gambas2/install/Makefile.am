EXTRA_DIST = $(PACKAGE)

all:
	@(if test "$(COMPONENT_build)" = "1"; then \
	 echo "Building $(PACKAGE) component..."; \
	else \
	 echo "Compiling $(PACKAGE) project..."; \
	fi)
	@(cd $(srcdir)/$(PACKAGE); \
	$(GAMBAS_path)/gbc2 -ag; \
	$(GAMBAS_path)/gba2)

install-exec-local:
	@(cd $(srcdir)/$(PACKAGE); \
	if test "$(COMPONENT_build)" = "1"; then \
	 $(INSTALL) $(PACKAGE).gambas $(prefix)/lib/gambas2; \
	 echo "Installing $(PACKAGE).gambas in $(GBLIBRARY_path)"; \
	 $(INSTALL) .component $(GBLIBRARY_path)/$(PACKAGE).component; \
	 echo "Installing $(PACKAGE).component in $(GBLIBRARY_path)"; \
	 chmod a-x $(GBLIBRARY_path)/$(PACKAGE).component; \
	 $(INSTALL) .info $(GBINFO_path)/$(PACKAGE).info; \
	 echo "Installing $(PACKAGE).info in $(GBINFO_path)"; \
	 chmod a-x $(GBINFO_path)/$(PACKAGE).info; \
	 $(INSTALL) .list $(GBINFO_path)/$(PACKAGE).list; \
	 echo "Installing $(PACKAGE).list in $(GBINFO_path)"; \
	 chmod a-x $(GBINFO_path)/$(PACKAGE).list; \
	else \
	 echo "Installing $(PACKAGE).gambas file in $(bindir)..."; \
	 cp $(srcdir)/$(PACKAGE)/$(PACKAGE).gambas $(bindir); \
	fi)

uninstall-local:
	@(if test "$(COMPONENT_build)" = "1"; then \
	 echo "Removing $(PACKAGE) component..."; \
	 rm -f $(GBLIBRARY_path)/$(PACKAGE).gambas; \
	 rm -f $(GBLIBRARY_path)/$(PACKAGE).component; \
	 rm -f $(GBINFO_path)/$(PACKAGE).info; \
	 rm -f $(GBINFO_path)/$(PACKAGE).list; \
	else \
	 echo "Removing $(PACKAGE).gambas file..." \
	 rm $(bindir)/$(PACKAGE).gambas; \
	fi)

dist-hook:
	@(cd $(distdir)/$(PACKAGES); \
	rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)
