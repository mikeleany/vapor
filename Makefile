# $Id: Makefile 10 2010-04-24 17:09:01Z mike $
# Project: Vapor Chess
# Purpose: Build debug, optimized debug, and release versions of Vapor.
#
# Copyright 2010 by Michael Leany
# All rights reserved

export vapordir := $(PWD)

all: dbg opt rel

opt:
	@echo Building optimized debug version . . .
	@$(MAKE) --no-print-directory -C Optimized
	@echo

rel:
	@echo Building release version . . .
	@$(MAKE) --no-print-directory -C Release
	@echo

dbg:
	@echo Building debug version . . .
	@$(MAKE) --no-print-directory -C Debug
	@echo

clean: clean-dbg clean-opt clean-rel

clean-dbg:
	@echo Cleaning debug version . . .
	@$(MAKE) --no-print-directory -C Debug clean
	@echo

clean-opt:
	@echo Cleaning optimized version . . .
	@$(MAKE) --no-print-directory -C Optimized clean
	@echo

clean-rel:
	@echo Cleaning release version . . .
	@$(MAKE) --no-print-directory -C Release clean
	@echo

# end of file
