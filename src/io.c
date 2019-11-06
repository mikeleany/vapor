/******************************************************************************
 * $Id: io.c 12 2010-10-10 16:17:26Z mike $
 * Project: Vapor Chess
 * Purpose: Handles input and output.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "io.h"

#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>

static char *InBuf = NULL;
static int InBufSize = 0;
static int InBufFill = 0;
#define MIN_BUF_SIZE  1024  // minimum size of the input buffer

/******************************************************************************
 * int resizeInBuf(void);
 * DESCRIPTION
 *    Resizes the input buffer, or creates it if it doesn't exist.
 * RETURN VALUE
 *    Does not return a value.
 */
static inline void resizeInBuf(void)
{
  if (!InBuf)
    InBufSize = MIN_BUF_SIZE;
  else
    InBufSize = InBufSize*2;

  InBuf = realloc(InBuf, InBufSize);
  assert(InBuf);
}

/******************************************************************************
 * int readToBuf(void);
 * DESCRIPTION
 *    Reads data into the input buffer.
 * RETURN VALUE
 *    Returns the number of bytes read.
 */
static int readToBuf(void)
{
  int Count;

  // resize buffer if necessary
  if (InBufFill == InBufSize)
    resizeInBuf();
  
  // read some data
  Count = read(0, InBuf+InBufFill, InBufSize-InBufFill);
  assert(Count != -1);
  InBufFill += Count;

  return Count;
}

/******************************************************************************
 * int initIO(void);
 * DESCRIPTION
 *    Initializes input and output.
 * RETURN VALUE
 *    Returns 0 for success, or -1 for failure.
 */
int initIO(void)
{
  // make output unbuffered
  setbuf(stdout, NULL);

  // initialize the read buffer
  if (!InBuf)
    resizeInBuf();

  return 0;
}

/******************************************************************************
 * int inputAvailable(void);
 * DESCRIPTION
 *    Determines if there is input available on standard input.
 * RETURN VALUE
 *    Returns 1 if input is available or 0 if not.
 */
#ifndef __MINGW32__ 
#include <sys/select.h>
static inline int inputAvailable(void)
{
  fd_set FDSet;
  struct timeval Time = { 0, 0 };

  FD_ZERO(&FDSet);
  FD_SET(STDIN, &FDSet);
  return (select(1, &FDSet, NULL, NULL, &Time) > 0);
}
#else
#include <windows.h>
static inline int inputAvailable(void)
{
  DWORD Bytes;
  HANDLE StdInHandle = GetStdHandle(STD_INPUT_HANDLE);

  if (isatty(STDIN))
      return (WaitForSingleObject(StdInHandle, 0) == WAIT_OBJECT_0);
  else if (PeekNamedPipe(StdInHandle, NULL, 0, NULL, &Bytes, NULL))
    return (Bytes > 0);
  else
    return 0;
}
#endif

/******************************************************************************
 * int inputLineReady(void);
 * DESCRIPTION
 *    Determines if there is a line available to be read using readLine.
 * RETURN VALUE
 *    Returns 1 if a line is available or 0 if no line is available.
 */
int inputLineReady(void)
{
  if (memchr(InBuf, '\n', InBufFill))
    return 1;
  while (inputAvailable())
  {
    readToBuf();
    if (memchr(InBuf, '\n', InBufFill))
      return 1;
  }

  return 0;
}

/******************************************************************************
 * const char *readLine(void);
 * DESCRIPTION
 *    Reads a line from standard input. If there is not a complete line
 *    immediately available, the function blocks until there is.
 * RETURN VALUE
 *    Returns a pointer to the line read from standard input. The pointer
 *    returned is valid until the next call to readLine.
 */
const char *readLine(void)
{
  static char *CmdStr = NULL;
  static int StrCap = 0;
  int Count;
  char *Ptr;

  if (!InBuf)
    initIO();

  // only read if we haven't already read a full line
  Ptr = memchr(InBuf, '\n', InBufFill); // search for the newline character
  while (!Ptr)
  {
    readToBuf();

    // search for the newline character
    Ptr = memchr(InBuf, '\n', InBufFill);
  }

  Count = (Ptr-InBuf) + 1; // include the newline character

  // allocate more memory for the string if needed
  if (!CmdStr || StrCap < Count+1)
  {
    StrCap = Count+1;
    CmdStr = realloc(CmdStr, Count+1);
    assert(CmdStr);
  }

  // copy the string
  memcpy(CmdStr, InBuf, Count);
  CmdStr[Count] = 0;

  // move data in the buffer to the front
  InBufFill = InBufFill - Count;
  memmove(InBuf, InBuf+Count, InBufFill);

  return CmdStr;
}

/* end of file */
