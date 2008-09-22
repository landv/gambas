SUBDIRS = help
EXTRA_DIST = reconf spec

install-exec-local:
	@echo
	@echo "Installing the gambas help files..."
	@rm -rf $(DESTDIR)$(gbdatadir)/help
	@$(INSTALL) -d $(DESTDIR)$(gbdatadir)/help
	@cp -R $(srcdir)/help $(DESTDIR)$(gbdatadir)
	@(cd $(DESTDIR)$(gbdatadir)/help; tar xfz $(DESTDIR)$(gbdatadir)/help/help.tar.gz; $(LN_S) -f help/img img; $(LN_S) -f help/image image; rm -f  Makefile*)

uninstall-local:
	@rm -rf $(DESTDIR)$(gbdatadir)/help
