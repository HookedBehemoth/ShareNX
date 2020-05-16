export APP_VERSION := 3.0.0

ifeq ($(RELEASE),)
	APP_VERSION	:=	$(APP_VERSION)-$(shell git describe --dirty --always)
endif

all: nro ovl

ovl:
	$(MAKE) -f Makefile.overlay

nro:
	$(MAKE) -f Makefile.applet

clean:
	$(MAKE) -f Makefile.overlay clean
	$(MAKE) -f Makefile.applet clean

.PHONY: all