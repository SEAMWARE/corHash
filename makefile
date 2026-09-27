#
# FILE            makefile
#
# AUTHOR          Ken Zangelin
#
# Copyright 2026 Seamware
# SPDX-License-Identifier: Apache-2.0
#
#
# corHash - a chained hash table: the caller supplies the hash and the compare
# function, and the table allocates through the allocator it is given (NULL =
# malloc).
#
# Every library in this stack is a SIBLING repo - `-I..` and `../<name>/lib<name>.a`
# is the layout, and it is part of the build contract rather than a convenience.
#
LIB_SO        = libcorHash.so
LIB           = libcorHash.a
CC            = gcc
INCLUDE       = -I..
DFLAGS        =
#
# EXTRA_CFLAGS - the hook for a caller that needs to ADD flags to this build.
# Not DFLAGS: `make DFLAGS=...` REPLACES it, and a `DFLAGS +=` here would be
# ignored along with it, so a caller adding one flag would drop every default.
#
CFLAGS        = -O2 -Wall -Wextra -Werror -fPIC -fstack-protector-strong $(DFLAGS) $(INCLUDE) -MMD -MP $(EXTRA_CFLAGS)

LIB_SOURCES   = corHash.c        \
                corHashVersion.c

BUILD        ?= debug
OBJDIR        = obj/$(BUILD)
OBJECTS       = $(LIB_SOURCES:%.c=$(OBJDIR)/%.o)
DEPS          = $(OBJECTS:.o=.d) $(OBJDIR)/corHashTest.d

#
# corHashTest - a smoke test of the library, built with it so it can never rot.
# It stays in obj/: it is not a tool, and nothing installs it.
#
TEST          = $(OBJDIR)/corHashTest
TEST_LIBS     = ../kalloc/libkalloc.a ../kbase/libkbase.a -lpthread

all: $(LIB) $(LIB_SO) $(TEST)

#
# $(OBJDIR)/.flags - rebuild when the COMPILE LINE changes
#
# A flag change is invisible to every timestamp: the sources are older than the
# objects and make sees nothing to do, so the build silently keeps objects
# compiled with the previous flags. This records them and makes the objects
# depend on the record.
#
$(OBJDIR)/.flags: FORCE
	@mkdir -p $(OBJDIR)
	@echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(OBJDIR)/%.o: %.c $(OBJDIR)/.flags
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

#
# Removed first: `ar r` replaces and adds but never removes, so an object that
# is no longer built stays in the archive forever, and the next link quietly
# uses code that is not in the tree any more.
#
$(LIB): $(OBJECTS)
	@rm -f $@
	ar rcs $@ $(OBJECTS)

$(LIB_SO): $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

$(TEST): $(OBJDIR)/corHashTest.o $(LIB)
	$(CC) -o $@ $< $(LIB) $(TEST_LIBS)

#
# install - nothing to copy. Consumers compile with `-I..` and link
# `../corHash/libcorHash.a` straight out of the checkout, so `all` has already
# put the artefacts where every consumer looks for them.
#
install: all

di: all install

ci: clean install

clean:
	rm -rf obj $(LIB) $(LIB_SO) *.o *.d *.gcno *.gcda

FORCE:

.PHONY: all install di ci clean FORCE

-include $(DEPS)
