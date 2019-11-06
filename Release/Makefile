# $Id: Makefile 10 2010-04-24 17:09:01Z mike $
# Project: Vapor Chess
# Purpose: Rules for building a release version of Vapor.
#
# Copyright 2010 by Michael Leany
# All rights reserved

# Source Directory
export srcdir := ../src

# C Preprocessor Flags
export CPPFLAGS := -D NDEBUG

# C Compiler Options
export CFLAGS := -O3

all: FORCE
	@$(MAKE) --no-print-directory -s -f ../vapor.mk $@

clean: FORCE
	@$(MAKE) --no-print-directory -s -f ../vapor.mk $@

FORCE: ;

# end of file
