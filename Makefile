export APP_VERSION := 3.0.0

all: nro ovl

ovl:
	$(MAKE) -f Makefile.overlay

nro:
	$(MAKE) -f Makefile.applet
