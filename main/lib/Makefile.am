SUBDIRS = . debug eval db compress vb option draw image gui gui.opengl image.effect signal complex

dist_gblib_DATA = gb.component
gblib_DATA = gb.component

install-data-hook:
	@echo
	@echo "Creating the information files for gb component..."
	@$(INSTALL) -d $(DESTDIR)$(gbdatadir)/info
	@$(DESTDIR)$(bindir)/gbi$(GAMBAS_VERSION) -r $(DESTDIR)$(prefix) gb
	@rm -f $(DESTDIR)$(gblibdir)/lib.gb.la
	@rm -f $(DESTDIR)$(gblibdir)/lib.gb.so*

uninstall-hook:
	@rm -rf $(DESTDIR)$(gblibdir)gb.component
	@rm -rf $(DESTDIR)$(gbdatadir)/info/gb.info
	@rm -rf $(DESTDIR)$(gbdatadir)/info/gb.list


