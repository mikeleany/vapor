/******************************************************************************
 * $Id: io.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Handles input and output.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__IO_H
#define VAPOR__IO_H

#include "vapor.h"

#include <stdio.h>

/******************************************************************************
 * int initIO(void);
 * DESCRIPTION
 *    Initializes input and output.
 * RETURN VALUE
 *    Returns 0 for success, or -1 for failure.
 */
int initIO(void);

/******************************************************************************
 * int inputLineReady(void);
 * DESCRIPTION
 *    Determines if there is a line available to be read using readLine.
 * RETURN VALUE
 *    Returns 1 if a line is available or 0 if no line is available.
 */
int inputLineReady(void);

/******************************************************************************
 * const char *readLine(void);
 * DESCRIPTION
 *    Reads a line from standard input. If there is not a complete line
 *    immediately available, the function blocks until there is.
 * RETURN VALUE
 *    Returns a pointer to the line read from standard input. The pointer
 *    returned is valid until the next call to readLine.
 */
const char *readLine(void);

#endif // #ifndef VAPOR__IO_H

/* end of file */
