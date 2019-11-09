/******************************************************************************
 * $Id: version.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Maintain Vapor Chess version information.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "version.h"
#include <stdio.h>

#define AUTHOR_NAME     "Michael Leany"
#define APP_NAME        "Vapor Chess"
#define APP_SHORT_NAME  "Vapor"
#define VER_NAME        "deja vu"
#define VER_NUM         "0.2.1"

const version VER = {
    AUTHOR_NAME,
    APP_NAME,
    APP_SHORT_NAME,
    EXEC_NAME,
    VER_NAME,
    VER_NUM,
    REVISION,
    BUILD_TYPE,
    BUILD_DATE
};

void printVersion(void)
{
  printf("%s v%s%s \"%s\" (built %s)\n", VER.AppName, 
         VER.VerNum, VER.BuildType, VER.VerName, VER.BuildDate);
  printf("Copyright %.4s by %s. All rights reserved.\n",
         VER.BuildDate, VER.AuthorName);
}

/* end of file */
