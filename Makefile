include config.mk

.POSIX:
.SUFFIXES: .c .o

all: options syn

options:
	@echo $(PROGNAME) build options:
	@echo "CFLAGS  	= $(CFLAGS)"
	@echo "LDFLAGS 	= $(LDFLAGS)"
	@echo "CC      	= $(CC)"

syn: syn.c config.h

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

dist: clean
	@echo creating dist tarball
	@mkdir -p $(PROGNAME)-$(VERSION)
	@cp COPYING README Makefile config.mk config.def.h $(SRC) \
		$(PROGNAME)-$(VERSION)
	@tar -cf $(PROGNAME)-$(VERSION).tar $(PROGNAME)-$(VERSION)
	@xz $(PROGNAME)-$(VERSION).tar
	@rm -rf $(PROGNAME)-$(VERSION)

install: all
	@echo installing executable to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/$(BIN)
$(PROGNAME): $(SRC)
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

uninstall:
	@echo removing executable from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ) $(PROGNAME)-$(VERSION).tar.xz

test: $(BIN)
	@bash -c 'echo -e "\e[32minput:\e[0m"; \
		cat test.in; \
		echo -e "\e[32moutput:\e[0m"; \
		./$(BIN) < test.in' && false || echo 'Test passed'

