EXTRA_DIST = $(PACKAGE)

all:
	@(if test "$(COMPONENT_build)" = "1"; then \
	 echo "Building $(PACKAGE) component..."; \
	else \
	 echo "Compiling $(PACKAGE) project..."; \
	fi)
	@(cd $(srcdir)/$(PACKAGE); \
	$(GAMBAS_path)/gbc$(VERSION) $(COMPILE_FLAGS); \
	$(GAMBAS_path)/gba$(VERSION))

install-exec-local:
	@(cd $(srcdir)/$(PACKAGE); \
	if test "$(COMPONENT_build)" = "1"; then \
	 echo "Installing $(PACKAGE).gambas in $(GBLIBRARY_path)"; \
	 $(INSTALL) -d $(prefix)/$(GBLIBRARY_path); \
	 $(INSTALL) $(PACKAGE).gambas $(prefix)/$(GBLIBRARY_path); \
	 echo "Installing $(PACKAGE).component in $(GBLIBRARY_path)"; \
	 $(INSTALL) -d $(prefix)/$(GBLIBRARY_path); \
	 $(INSTALL) .component $(prefix)/$(GBLIBRARY_path)/$(PACKAGE).component; \
	 chmod a-x $(prefix)/$(GBLIBRARY_path)/$(PACKAGE).component; \
	 echo "Installing $(PACKAGE).info in $(GBINFO_path)"; \
	 $(INSTALL) -d .info $(prefix)/$(GBINFO_path); \
	 $(INSTALL) .info $(prefix)/$(GBINFO_path)/$(PACKAGE).info; \
	 chmod a-x $(prefix)/$(GBINFO_path)/$(PACKAGE).info; \
	 echo "Installing $(PACKAGE).list in $(GBINFO_path)"; \
	 $(INSTALL) .list $(prefix)/$(GBINFO_path)/$(PACKAGE).list; \
	 chmod a-x $(prefix)/$(GBINFO_path)/$(PACKAGE).list; \
	 if test -d control; then \
	  echo "Installing controls in $(GBCONTROL_path)"; \
	  $(INSTALL) -d $(prefix)/$(GBCONTROL_path)/$(PACKAGE); \
	  $(INSTALL) control/*.png $(prefix)/$(GBCONTROL_path)/$(PACKAGE); \
	  chmod a-x $(prefix)/$(GBCONTROL_path)/$(PACKAGE)/*.png; \
	 fi \
	else \
	 echo "Installing $(PACKAGE).gambas file in $(bindir)..."; \
	 $(INSTALL) -d $(bindir); \
	 $(INSTALL) $(PACKAGE).gambas $(bindir); \
	fi)

uninstall-local:
	@(if test "$(COMPONENT_build)" = "1"; then \
	 echo "Removing $(PACKAGE) component..."; \
	 rm -f $(prefix)/$(GBLIBRARY_path)/$(PACKAGE).gambas; \
	 rm -f $(prefix)/$(GBLIBRARY_path)/$(PACKAGE).component; \
	 rm -f $(prefix)/$(GBINFO_path)/$(PACKAGE).info; \
	 rm -f $(prefix)/$(GBINFO_path)/$(PACKAGE).list; \
	 rm -rf $(prefix)/$(GBCONTROL_path)/$(PACKAGE); \
	else \
	 echo "Removing $(PACKAGE).gambas file..." \
	 rm $(bindir)/$(PACKAGE).gambas; \
	fi)

dist-hook:
	@(cd $(distdir)/$(PACKAGES); \
	rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)
