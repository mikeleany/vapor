# $Id: Makefile 31 2012-08-02 13:17:08Z mikeleany $
# Project: Vapor Chess
# Purpose: Rules for building an optimized debug version of Vapor.
#
# Copyright 2010 by Michael Leany
# All rights reserved

# Source Directory
export srcdir := ../src

# C Compiler Options
export CFLAGS := -O3 -g3

# Build Type
export buildtype := -DEBUG

all: FORCE
	@$(MAKE) --no-print-directory -s -f ../vapor.mk $@

clean: FORCE
	@$(MAKE) --no-print-directory -s -f ../vapor.mk $@

FORCE: ;

# end of file
