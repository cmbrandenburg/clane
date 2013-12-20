SUBDIRS = check net uri http

EXTRA_DIST = \
	leak_check

leak-check:
	for i in $(SUBDIRS); do $(MAKE) -C $$i leak-check || exec false; done

