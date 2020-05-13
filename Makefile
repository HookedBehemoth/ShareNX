all: nro ovl

ovl:
	$(MAKE) -f Makefile.overlay

nro:
	$(MAKE) -f Makefile.applet
