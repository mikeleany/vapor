/******************************************************************************
 * $Id: uci.h 12 2010-10-10 16:17:26Z mike $
 * Project: Vapor Chess
 * Purpose: Implements the UCI protocol.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__UCI_H
#define VAPOR__UCI_H

#include "vapor.h"

/******************************************************************************
 * int ucimain(const char *CmdStr);
 * PARAMETERS
 *    CmdStr -- The command line read prior to calling this function or NULL if
 *        no command line has been read.
 * DESCRIPTION
 *    Use the UCI protocol for the remainder of the execution time.
 * RETURN VALUE
 *    Returns 0 upon success or 1 upon failure.
 */
int ucimain(void);

#endif // #ifndef VAPOR__UCI_H

/* end of file */
