/******************************************************************************
 * $Id: uci.c 36 2012-08-09 03:50:35Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Implements the UCI protocol.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#include "uci.h"
#include "version.h"
#include "init.h"
#include "io.h"
#include "parse.h"
#include "game.h"
#include "notation.h"
#include "search.h"
#include "hash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int BlockInput = 1;

typedef enum command
{
  C_QUIT = 0,
  C_GO,
  C_PONDERHIT,
  C_POSITION,
  C_STOP,
  C_ISREADY,
  C_UCI,
  C_UCINEWGAME,
  C_SETOPTION,
  C_DEBUG,
  C_REGISTER,

  NO_CMD,
  INVALID_CMD = -1
} command;

static const char *const CmdList[] = {
    "quit",
    "go",
    "ponderhit",
    "position",
    "stop",
    "isready",
    "uci",
    "ucinewgame",
    "setoption",
    "debug",
    "register",

    NULL
};

typedef enum go_var
{
  GV_SEARCHMOVES,
  GV_PONDER,
  GV_WTIME,
  GV_BTIME,
  GV_WINC,
  GV_BINC,
  GV_MOVESTOGO,
  GV_DEPTH,
  GV_NODES,
  GV_MATE,
  GV_MOVETIME,
  GV_INFINITE,

  NO_VAR = -1
} go_var;

static const char *const GoVarList[] = {
    "searchmoves",
    "ponder",
    "wtime",
    "btime",
    "winc",
    "binc",
    "movestogo",
    "depth",
    "nodes",
    "mate",
    "movetime",
    "infinite",

    NULL
};

static inline int lcmatch(const char *Str1, const char *Str2)
{
  int i = 0;

  while (Str1[i] || Str2[i]) {
    if (tolower(Str1[i]) != tolower(Str2[i]))
      return 0;
    i++;
  }

  return 1;
}

/******************************************************************************
 * command findCmd(const char *Str);
 * PARAMETERS
 *    Str -- The string containing the command to find.
 * DESCRIPTION
 *    Finds Str in CmdList and returns the command index.
 * RETURN VALUE
 *    Returns the command index of Str or INVALID_CMD if Str is not in CmdList.
 */
static inline command findCmd(const char *Str)
{
  int i;

  if (!Str)
    return NO_CMD;

  for (i = 0; CmdList[i]; i++)
  {
    if (strcmp(Str, CmdList[i]) == 0)
      return i;
  }
  // TODO: if command not found, move on to next token

  return INVALID_CMD;
}

/******************************************************************************
 * void parseGoCmd(void);
 * DESCRIPTION
 *    Parses and executes the "go" command.
 * RETURN VALUE
 *    Does not return a value.
 */
static void parseGoCmd(void);

/******************************************************************************
 * void uciPrintPV(void);
 * DESCRIPTION
 *    Prints the PV data in a way UCI understands.
 * RETURN VALUE
 *    Does not return a value.
 */
static void uciPrintPV(void)
{
  char MoveStr[8];

  printf("info score ");
  if (PVData.Val > LONG_MATE)
    printf("mate %i", (INFINITY-PVData.Val+1)/2);
  else if (PVData.Val <= -LONG_MATE)
    printf("mate %i", (-INFINITY-PVData.Val)/2);
  else
    printf("cp %i", PVData.Val);

  printf(" depth %i nodes %"_i64" time %"_i64" nps %"_i64" pv",
         PVData.Depth, PVData.Nodes, toMillisec(PVData.Time),
         PVData.NodesPerSec);

  for (int i = 0; i < PVData.Length; i++)
  {
    getCoordStr(PVData.Move[i], MoveStr);
    printf(" %s", MoveStr);
  }
  printf("\n");
}

static void uciCheckInput(void)
{
  const char *CmdStr;
  command Cmd;

  if (!BlockInput && !inputLineReady())
    return;

  CmdStr = readLine();
  splitCmdLine(CmdStr);
  Cmd = findCmd(Args[0]);
  
  switch (Cmd)
  {
    case C_QUIT:
      exit(0);
    case C_PONDERHIT:
      Search.Flags &= ~SF_PONDER;
      return;
    case C_STOP:
      Search.Flags |= SF_STOPPED;
      return;
    case C_ISREADY:
      printf("readyok\n");
      return;
    default:
      return;
  }
}

/******************************************************************************
 * int ucimain(void);
 * DESCRIPTION
 *    Use the UCI protocol for the remainder of the execution time.
 * RETURN VALUE
 *    Returns 0 upon success or 1 upon failure.
 */
int ucimain(void)
{
  static const uint64 MEGABYTE = 0x100000;
  const char *CmdStr;
  command Cmd = NO_CMD;
  int i;

  uint64 HashMB = 256;


  while (Cmd != C_UCI)
  {
    CmdStr = readLine();
    splitCmdLine(CmdStr);
    Cmd = findCmd(Args[0]);
    if (Cmd == C_QUIT)
      return 0;
  }
  
  printf("id name %s v%s%s\n",
    VER.AppShortName, VER.VerNum, VER.BuildType);
  printf("id author %s\n", VER.AuthorName);
  printf("option name Ponder type check\n");
  printf("option name Hash type spin default %"_u64" min 0\n", HashMB);
  printf("uciok\n");

  while (Cmd != C_ISREADY)
  {
    CmdStr = readLine();
    splitCmdLine(CmdStr);
    Cmd = findCmd(Args[0]);
    if (Cmd == C_SETOPTION)
    {
      if (Args[1] && strcmp(Args[1], "name") == 0)
      {
        // find "value" keyword
        for (i = 2; i < nArgs; i++)
        {
          if (strcmp(Args[i], "value") == 0)
            break;
        }
        joinArgs(i+1, nArgs-1); // join value
        joinArgs(2, i-1);       // join name
        if (Args[2] && lcmatch(Args[2], "Hash") && Args[4]) {
          HashMB = atoi(Args[4]);
        }
      }
    } else if (Cmd == C_QUIT) {
      return 0;
    }
  }

  init();
  if (HashMB && !initHash(HashMB * MEGABYTE)) {
    printf("info string cannot allocate hash of %iMB\n", (int)HashMB);
  } else {
    printf("info string allocated hash of %iMB\n",
        (int)(nHashEntries*sizeof(hash_buckets)/MEGABYTE));
  }
  printPV = uciPrintPV;
  checkInput = uciCheckInput;

  while (Cmd != C_QUIT)
  {
    switch (Cmd)
    {
      case C_ISREADY:
        printf("readyok\n");
        break;

      case C_POSITION:
      {
        if (Args[1] && strcmp(Args[1], "startpos") == 0)
        {
          resetGame();
          i = 2;
        }
        else if (Args[2] && strcmp(Args[1], "fen") == 0)
        {
          // find "moves" keyword
          for (i = 2; i < nArgs; i++)
          {
            if (strcmp(Args[i], "moves") == 0)
              break;
          }
          joinArgs(2, i-1);
          if (setGamePos(Args[2]) != 0)
            printf("info string invalid FEN string: %s\n", Args[2]);
          i = 3;
        }
        else
        {
          printf("info string malformed command\n");
          break;
        }

        if (Args[i] && strcmp(Args[i], "moves") == 0)
        {
          for (i++; i < nArgs; i++)
          {
            if (makeGameMove(Args[i]) != 0)
            {
              printf("info string illegal move: %s\n", Args[i]);
              break;
            }
          }
        }
      } break;

      case C_GO:
        parseGoCmd();
        break;

      default:
        break;
    }

    CmdStr = readLine();
    splitCmdLine(CmdStr);
    Cmd = findCmd(Args[0]);
  }

  freeHash();
  return 0;
}

/******************************************************************************
 * void parseGoCmd(void);
 * DESCRIPTION
 *    Parses and executes the "go" command.
 * RETURN VALUE
 *    Does not return a value.
 */
static void parseGoCmd(void)
{
  int i;
  go_var Var;
  char MoveStr[8];

  memset(&Search, 0, sizeof(Search));

  for (i = 1; i < nArgs; i++)
  {
    for (Var = 0; GoVarList[Var]; Var++)
    {
      if (strcmp(Args[i], GoVarList[Var]) == 0)
        break;
    }
    if (!GoVarList[Var])
      continue;

    switch (Var)
    {
      case GV_INFINITE:
        Search.Flags |= SF_INFINITE;
        break;
      case GV_PONDER:
        Search.Flags |= SF_PONDER;
        break;
      case GV_DEPTH:
        Search.MaxDepth = atoi(Args[++i]);
        break;
      case GV_MATE:
        Search.MaxDepth = 2*atoi(Args[++i]);
        break;
      case GV_NODES:
        Search.MaxNodes = atoi(Args[++i]);
        break;
      case GV_MOVETIME:
        Search.MoveTime = atoi(Args[++i])*ONE_MILLISEC;
        break;
      case GV_WTIME:
        Search.Time[WHITE] = atoi(Args[++i])*ONE_MILLISEC;
        break;
      case GV_BTIME:
        Search.Time[BLACK] = atoi(Args[++i])*ONE_MILLISEC;
        break;
      case GV_WINC:
        Search.Inc[WHITE] = atoi(Args[++i])*ONE_MILLISEC;
        break;
      case GV_BINC:
        Search.Inc[BLACK] = atoi(Args[++i])*ONE_MILLISEC;
        break;
      case GV_MOVESTOGO:
        Search.MovesToGo = atoi(Args[++i]);
        break;
      default:
        break;
    }
  }

  BlockInput = 0;
  searchRoot();
  BlockInput = 1;


  while ((Search.Flags & (SF_PONDER | SF_INFINITE))
      && !(Search.Flags & SF_STOPPED))
  {
    uciCheckInput();
  }

  if (PVData.Length)
  {
    getCoordStr(PVData.Move[0], MoveStr);
    printf("bestmove %s", MoveStr);
    if (PVData.Length > 1)
    {
      getCoordStr(PVData.Move[1], MoveStr);
      printf(" ponder %s", MoveStr);
    }
    printf("\n");
  }
  else
    printf("bestmove 0000\n");
}

/* end of file */
