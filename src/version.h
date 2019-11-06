/******************************************************************************
 * $Id: version.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Maintain Vapor Chess version information.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__VERSION_H
#define VAPOR__VERSION_H

#include "vapor.h"

typedef struct version
{
  const char *AuthorName;
  const char *AppName;
  const char *AppShortName;
  const char *ExecName;
  const char *VerName;
  const char *VerNum;
  const char *Revision;
  const char *BuildType;
  const char *BuildDate;
} version;

extern const version VER;

void printVersion(void);

#endif // #ifndef VAPOR__VERSION_H

/* end of file */
