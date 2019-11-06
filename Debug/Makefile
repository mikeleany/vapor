# $Id: Makefile 31 2012-08-02 13:17:08Z mikeleany $
# Project: Vapor Chess
# Purpose: Rules for building a debug version of Vapor.
#
# Copyright 2010 by Michael Leany
# All rights reserved

# Source Directory
export srcdir := ../src

# C Compiler Options
export CFLAGS := -g3

# Build Type
export buildtype := -DEV

all: FORCE
	@$(MAKE) --no-print-directory -s -f ../vapor.mk $@

clean: FORCE
	@$(MAKE) --no-print-directory -s -f ../vapor.mk $@

FORCE: ;

# end of file
