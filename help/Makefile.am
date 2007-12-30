## Process this file with automake to produce Makefile.in

EXTRA_DIST = reconf spec help

install-exec-local:
	@echo
	@echo "Installing the gambas help files..."
	@$(INSTALL) -d $(DESTDIR)$(gbdatadir)/help
	@rm -rf $(DESTDIR)$(gbdatadir)/help
	@cp -R $(srcdir)/help $(DESTDIR)$(gbdatadir)
	@(cd $(DESTDIR)$(gbdatadir)/help; tar xfj $(DESTDIR)$(gbdatadir)/help/help.tar.bz2)

uninstall-local:
	@rm -rf $(DESTDIR)$(gbdatadir)/help
