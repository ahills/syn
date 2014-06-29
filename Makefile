include config.mk

.POSIX:
.SUFFIXES: .c .o

SRC = syn.c

OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=)

all: options bin

options:
	@echo $(PROGNAME) build options:
	@echo "CFLAGS  	= $(CFLAGS)"
	@echo "LDFLAGS 	= $(LDFLAGS)"
	@echo "CC      	= $(CC)"

bin: $(BIN)

$(OBJ): config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

.o:
	@echo LD $@
	@$(LD) -o $@ $< $(LDFLAGS)

.c.o:
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS)

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

