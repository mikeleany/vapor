/******************************************************************************
 * $Id: parse.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Provides functions to parse a command line.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__PARSE_H
#define VAPOR__PARSE_H

#include "vapor.h"

extern const char **Args; // array of arguments
extern int nArgs;         // number of arguments in Args

/******************************************************************************
 * int splitCmdLine(const char *CmdLine);
 * PARAMETERS
 *    CmdLine - pointer to the command line to be split into tokens.
 * DESCRIPTION
 *    Splits the command line into white-space delimited tokens, and fills Args
 *    with pointers to the tokens. Sets nArgs to the number of tokens in Args.
 * RETURN VALUE
 *    nArgs.
 */
int splitCmdLine(const char *CmdLine);

/******************************************************************************
 * void joinArgs(int First, int Last);
 * PARAMETERS
 *    First - the index of the first argument to join.
 *    Last - the index of the last argument to join.
 * DESCRIPTION
 *    Combines the arguments Args[First] through Args[Last] into one argument
 *    along with the whitespace contained in Orig. Then shifts all arguments
 *    in Args appropriately, and adjusts nArgs to the new number of Args.
 * RETURN VALUE
 *    nArgs.
 */
int joinArgs(int First, int Last);

#endif // #ifndef VAPOR__PARSE_H

/* end of file */
