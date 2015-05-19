include config.mk

.POSIX:
.SUFFIXES: .c .o

all: options $(PROGNAME)

options:
	@echo $(PROGNAME) build options:
	@echo "CFLAGS  	= $(CFLAGS)"
	@echo "LDFLAGS 	= $(LDFLAGS)"
	@echo "CC      	= $(CC)"

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

dist: clean
	@echo creating dist tarball
	@mkdir -p $(PROGNAME)-$(VERSION)
	@cp COPYING README Makefile config.mk config.def.h syn.c \
		$(PROGNAME)-$(VERSION)
	@tar -cf $(PROGNAME)-$(VERSION).tar $(PROGNAME)-$(VERSION)
	@xz $(PROGNAME)-$(VERSION).tar
	@rm -rf $(PROGNAME)-$(VERSION)

install: all
	@echo installing executable to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f $(PROGNAME) $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/$(PROGNAME)

$(PROGNAME): syn.c config.h
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

uninstall:
	@echo removing executable from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin

clean:
	@echo cleaning
	@rm -f $(PROGNAME) $(BIN) $(OBJ) $(PROGNAME)-$(VERSION).tar.xz

