# makefile - build system for doq
#
#
# @author: Cade Brown <cade@kscript.org>


# -*- Config -*-

# kscript module name
NAME           := doq

# kscript directory
KS             ?= /usr/local

# install path
PREFIX         := /usr/local
DESTDIR        ?= 
TODIR          := $(DESTDIR)$(PREFIX)

# DEBUG
CXXFLAGS += -g


# -*- Files -*-

src_CC         := $(wildcard src/*.cc)
src_HH         := $(wildcard include/*.hh)

src_O          := $(patsubst %.cc,%.o,$(src_CC))


# -*- Output -*-

# output shared object file
prog_BIN       := $(NAME)


# -*- Rules -*-

.PHONY: default all clean install uninstall FORCE


default: $(prog_BIN)

all: $(prog_BIN)

clean: FORCE
	rm -f $(wildcard $(src_O) $(prog_BIN))

install: FORCE
	install -d $(TODIR)/bin/$(NAME)
	strip $(TODIR)/bin/$(NAME)

uninstall: FORCE
	rm -rf $(TODIR)/bin/$(NAME)

FORCE:

$(prog_BIN): $(src_O)
	$(CXX) \
		$^ \
		$(LDFLAGS) -o $@

%.o: %.cc $(src_HH)
	$(CXX) $(CXXFLAGS) -Iinclude -fPIC -c -o $@ $<


