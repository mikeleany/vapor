# $Id: vapor.mk 31 2012-08-02 13:17:08Z mikeleany $
# Project: Vapor Chess
# Purpose: Rules for building Vapor.
#
# Copyright 2010 by Michael Leany
# All rights reserved

# Source Directory
VPATH := $(srcdir)

# Files
executable := vapor
sources := $(notdir $(wildcard $(srcdir)/*.c))
objects := $(sources:.c=.o)

# Build Number and Date
revision := $(shell git rev-parse --short HEAD || echo unknown)
builddate := $(shell date +%F)

# C Preprocessor Options sent only to version.c
verflags := -D BUILD_TYPE=\"$(buildtype)\" -D REVISION=\"$(revision)\" \
	-D BUILD_DATE=\"$(builddate)\" -D EXEC_NAME=\"$(executable)\"

# C Preprocessor Options
#	-MMD	Creates a dependency file as a side-effect of compilation.
#	-MP		Include phony targets for headers files in dependency file.
CPPFLAGS := -MMD -MP $(CPPFLAGS)

# C Warning Options
CFLAGS := -Wall -Werror-implicit-function-declaration -Winit-self \
	-Wwrite-strings -Wstrict-prototypes -Wextra -Wno-unused-parameter \
	-pedantic-errors $(CFLAGS)

# C Dialect Options
CFLAGS := -std=c99 $(CFLAGS)

# Architecture Options
CFLAGS := $(ARCHFLAGS) $(CFLAGS)
LDFLAGS := $(ARCHFLAGS)

all: $(executable)
	@echo Build complete.

$(executable): $(objects)
	@echo Linking . . .
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
	@echo Built revision $(revision)$(buildtype) \($(builddate)\)

version.o: version.c $(filter-out version.o, $(objects))
	@echo Compiling $<
	$(CC) $(CFLAGS) $(CPPFLAGS) $(verflags) -c -o $@ $<

%.o: %.c
	@echo Compiling $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Include dependency files if they exist.
-include *.d

clean:
	$(RM) *.o *.d $(executable)
	@echo Build directory is now clean.

# end of file
