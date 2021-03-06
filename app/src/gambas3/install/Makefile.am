EXTRA_DIST = $(PACKAGE)

all:
	@(if test "$(COMPONENT_build)" = "1"; then \
	 echo "Building $(COMPONENT_name) component..."; \
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
	 $(INSTALL) -d $(DESTDIR)/$(GBLIBRARY_path); \
	 $(INSTALL) $(PACKAGE).gambas $(DESTDIR)/$(GBLIBRARY_path)/$(COMPONENT_name).gambas; \
	 echo "Installing $(PACKAGE).component in $(GBLIBRARY_path)"; \
	 $(INSTALL) -d $(DESTDIR)/$(GBLIBRARY_path); \
	 $(INSTALL) .component $(DESTDIR)/$(GBLIBRARY_path)/$(COMPONENT_name).component; \
	 chmod a-x $(DESTDIR)/$(GBLIBRARY_path)/$(COMPONENT_name).component; \
	 echo "Installing $(PACKAGE).info in $(GBINFO_path)"; \
	 $(INSTALL) -d $(DESTDIR)/$(GBINFO_path); \
	 $(INSTALL) .info $(DESTDIR)/$(GBINFO_path)/$(COMPONENT_name).info; \
	 chmod a-x $(DESTDIR)/$(GBINFO_path)/$(COMPONENT_name).info; \
	 echo "Installing $(PACKAGE).list in $(GBINFO_path)"; \
	 $(INSTALL) .list $(DESTDIR)/$(GBINFO_path)/$(COMPONENT_name).list; \
	 chmod a-x $(DESTDIR)/$(GBINFO_path)/$(COMPONENT_name).list; \
	 if test -d .hidden/control; then \
	  echo "Installing controls in $(GBCONTROL_path)"; \
	  $(INSTALL) -d $(DESTDIR)/$(GBCONTROL_path)/$(COMPONENT_name); \
	  $(INSTALL) .hidden/control/*.png $(DESTDIR)/$(GBCONTROL_path)/$(COMPONENT_name); \
	  chmod a-x $(DESTDIR)/$(GBCONTROL_path)/$(COMPONENT_name)/*.png; \
	 fi \
	else \
	 echo "Installing '$(PACKAGE)' executable in $(bindir)..."; \
	 $(INSTALL) -d $(DESTDIR)$(bindir); \
	 $(INSTALL) $(PACKAGE).gambas $(DESTDIR)$(bindir); \
	 ln -sf $(PACKAGE).gambas $(DESTDIR)$(bindir)/$(PACKAGE); \
	 if test "$(PACKAGE)" != $(PACKAGE_LCASE); then \
	   echo "Creating lower case symbolic link..."; \
	   ln -sf $(PACKAGE).gambas $(DESTDIR)$(bindir)/$(PACKAGE_LCASE); \
	 fi \
	fi)

uninstall-local:
	@(if test "$(COMPONENT_build)" = "1"; then \
	 echo "Removing $(COMPONENT_name) component..."; \
	 rm -f $(DESTDIR)/$(GBLIBRARY_path)/$(COMPONENT_name).gambas; \
	 rm -f $(DESTDIR)/$(GBLIBRARY_path)/$(COMPONENT_name).component; \
	 rm -f $(DESTDIR)/$(GBINFO_path)/$(COMPONENT_name).info; \
	 rm -f $(DESTDIR)/$(GBINFO_path)/$(COMPONENT_name).list; \
	 rm -rf $(DESTDIR)/$(GBCONTROL_path)/$(COMPONENT_name); \
	else \
	 echo "Removing executable..."; \
	 rm -f $(DESTDIR)$(bindir)/$(PACKAGE).gambas; \
	 rm -f $(DESTDIR)$(bindir)/$(PACKAGE); \
	 rm -f $(DESTDIR)$(bindir)/$(PACKAGE_LCASE); \
	fi)

dist-hook:
	@(cd $(distdir)/$(PACKAGES); \
	rm -rf `find . -name ".gambas" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core.*" -o -name ".kdbg*" -o -name ".svn"`;)
