-include ../Makefile.settings

all:
	# Only build the docs if this is a bzr checkout
	test ! -d ../.bzr || $(MAKE) -C user-guide

install:
	mkdir -p $(DESTDIR)$(MANDIR)/man8/ $(DESTDIR)$(MANDIR)/man5/
	install -m 0644 bitlbee.8 $(DESTDIR)$(MANDIR)/man8/
	install -m 0644 bitlbee.conf.5 $(DESTDIR)$(MANDIR)/man5/
	$(MAKE) -C user-guide $@

uninstall:
	rm -f $(DESTDIR)$(MANDIR)/man8/bitlbee.8*
	rm -f $(DESTDIR)$(MANDIR)/man5/bitlbee.conf.5*
	$(MAKE) -C user-guide $@

.PHONY: install uninstall
