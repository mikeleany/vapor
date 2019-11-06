/******************************************************************************
 * $Id: parse.c 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Provides functions to parse a command line.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "parse.h"

#include <string.h>
#include <stdlib.h>

#define WHITE_CHARS " \f\n\r\t\v"

#define MIN_ARGS_CAP  128

const char **Args = NULL;   // array of arguments
int nArgs = 0;              // number of arguments in Args
static int ArgsCap = 0;     // capacity of Args
static const char *CmdStr = NULL; // the original command line
static char *ArgStr = NULL; // storage for split string
static int ArgStrCap = 0;   // capacity of ArgStr;

/******************************************************************************
 * void addArg(char *Arg);
 * PARAMETERS
 *    Arg - argument to add to the argument list.
 * DESCRIPTION
 *    Adds Arg to the argument list Args, resizing the array if necessary, and
 *    increments nArgs.
 * RETURN VALUE
 *    Does not return a value.
 */
static inline void addArg(char *Arg)
{
  while (!Args || nArgs >= ArgsCap)
  {
    if (!Args)
      ArgsCap = MIN_ARGS_CAP;
    else
      ArgsCap *= 2;

    Args = realloc(Args, ArgsCap*sizeof(char *));
    assert(Args);
  }

  Args[nArgs] = Arg;

  if (Arg)
    nArgs++;
}

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
int splitCmdLine(const char *CmdLine)
{
  char *Token;

  nArgs = 0;
  CmdStr = CmdLine;

  if (!CmdStr)
    return 0;

  // resize ArgStr if necessary
  if ((int)strlen(CmdStr) + 1 > ArgStrCap)
  {
    ArgStrCap = strlen(CmdStr) + 1;
    ArgStr = realloc(ArgStr, ArgStrCap);
    assert(ArgStr);
  }

  strcpy(ArgStr, CmdStr);

  /* divide string into tokens */
  Token = strtok(ArgStr, WHITE_CHARS);
  while (Token)
  {
    addArg(Token);
    Token = strtok(NULL, WHITE_CHARS);
  }
  addArg(NULL);

  return nArgs;
}

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
int joinArgs(int First, int Last)
{
  int Start, Len;
  int i, j;

  if (Last >= nArgs)
    Last = nArgs - 1;

  if (Last > First)
  {
    Start = Args[First] - ArgStr;
    Len = Args[Last] - Args[First] + strlen(Args[Last]);
    // restore from original string
    strncpy(ArgStr+Start, CmdStr+Start, Len);

    // move other arguments forward
    nArgs -= (Last - First);
    for (i = First + 1, j = Last + 1; i < nArgs; i++, j++)
      Args[i] = Args[j];
  }

  return nArgs;
}

/* end of file */
