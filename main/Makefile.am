## Process this file with automake to produce Makefile.in

SUBDIRS = libltdl gbx gbc lib share
EXTRA_DIST = TODO reconf spec README

install-exec-local:
	@if test x"$(DESTDIR)" != x; then \
		echo "Installing with DESTDIR='$(DESTDIR)'"; \
	fi
	@echo "Making runtime symbolic link"
##	@if test "$(DESTDIR)$(bindir)" != "$(DESTDIR)/usr/bin" && test "$(DESTDIR)$(bindir)" != "$(DESTDIR)/usr/bin/"; then \
##	 $(LN_S) -f $(bindir)/gbx$(GAMBAS_VERSION) $(DESTDIR)/usr/bin/gbx$(GAMBAS_VERSION) || true; \
##	 $(LN_S) -f $(bindir)/gbc$(GAMBAS_VERSION) $(DESTDIR)/usr/bin/gbc$(GAMBAS_VERSION) || true; \
##	 $(LN_S) -f $(bindir)/gba$(GAMBAS_VERSION) $(DESTDIR)/usr/bin/gba$(GAMBAS_VERSION) || true; \
##	 $(LN_S) -f $(bindir)/gbi$(GAMBAS_VERSION) $(DESTDIR)/usr/bin/gbi$(GAMBAS_VERSION) || true; \
##	fi
	@$(LN_S) -f gbx$(GAMBAS_VERSION) $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION) || true

uninstall-local:
##	@rm -f $(DESTDIR)/usr/bin/gbx$(GAMBAS_VERSION)
	@rm -f $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION)
##	@rm -f $(DESTDIR)/usr/bin/gbc$(GAMBAS_VERSION)
##	@rm -f $(DESTDIR)/usr/bin/gba$(GAMBAS_VERSION)
##	@rm -f $(DESTDIR)/usr/bin/gbi$(GAMBAS_VERSION)
	@rm -rf $(DESTDIR)$(gblibdir)/info
