/******************************************************************************
 * $Id: search.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Searches for the best moves.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__SEARCH_H
#define VAPOR__SEARCH_H

#include "vapor.h"
#include "chess.h"
#include "microtime.h"

#define MAX_SEARCH_DEPTH  32
#define MAX_PLY (MAX_SEARCH_DEPTH*2)

#define INFINITY  0x7fff
#define LONG_MATE 0x7f00   

struct searchdata
{
  uint32 Flags;
  microtime Time[NUM_COLORS];
  microtime Inc[NUM_COLORS];
  int MovesToGo;
  int MaxDepth;
  int64 MaxNodes;
  microtime MoveTime;
} Search;
#define SF_PONDER   0x01
#define SF_INFINITE 0x02
#define SF_STOPPED  0x80000000

struct pvdata
{
  int Val;
  int Depth;
  microtime Time;
  int64 Nodes;
  int64 NodesPerSec;

  int Length;
  move Move[MAX_PLY];
} PVData;

void searchRoot(void);

extern void (*printPV)(void);
extern void (*checkInput)(void);

#endif // #ifndef VAPOR__SEARCH_H

/* end of file */
