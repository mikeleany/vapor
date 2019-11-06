/******************************************************************************
 * $Id: mgtest.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Tests the move generator.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#include "mgtest.h"
#include "chess.h"
#include "moves.h"
#include "notation.h"
#include "fen.h"
#include "microtime.h"

#include <stdio.h>
#include <string.h>

static uint64 Nodes;
static char FENStr[128];

static int isConsistent(const position *Pos)
{
  piece p;
  bitboard Bd1 = 0, Bd2 = 0;

  // check bitboard consistency
  for (p = PAWN; p <= KING; p++)
  {
    Bd1 ^= Pos->OccBy[WHITE][p];
    Bd2 ^= Pos->OccBy[BLACK][p];
  }
  if (Bd1 != Pos->OccBy[WHITE][0] || Bd2 != Pos->OccBy[BLACK][0])
    return 0;
  if ((Bd1 ^ Bd2) != Pos->Occ)
    return 0;

  // validate DrawPlies and MoveNum
  if (Pos->DrawPlies < 0 || Pos->MoveNum < 1)
    return 0;

  // check epsquare
  if (Pos->EPSquare != NO_SQUARE && (Pos->EPSquare < a1 || Pos->EPSquare > h8))
    return 0;
  if ((Pos->Flags & PF_EPLEGAL) && Pos->EPSquare == NO_SQUARE)
    return 0;

  // check legality
  return isPosLegal(Pos);
}

static uint64 countVariations(const position *Pos, int Depth)
{
  position NewPos;
  int MvBase;
  int nMoves;
  uint64 Total = 0;
  int i;

  Nodes++;

  if (Depth == 0)
    return 1;

  MvBase = getMoveStackTop();
  nMoves = genCaptures(Pos) + genQuietMoves(Pos);

  for (i = 0; i < nMoves; i++)
  {
    NewPos = *Pos;
    if (quickMakeMove(&NewPos, MoveStack[MvBase + i]) == 0)
      Total += countVariations(&NewPos, Depth-1);
  }

  popMoveStack(MvBase);

  return Total;
}

static int64 mgtestCount(const position *Pos, int Depth)
{
  static char MoveStr[8];
  position NewPos;
  int MvBase;
  int nMoves;
  int64 Count;
  int64 Total = 0;
  int i;
  move TmpMove;

  Nodes++;

  if (Depth == 0)
    return 1;

  MvBase = getMoveStackTop();
  nMoves = genCaptures(Pos) + genQuietMoves(Pos);

  for (i = 0; i < nMoves; i++)
  {
    // test the move verification functions
    getCoordStr(MoveStack[MvBase + i], MoveStr);
    if (expandMove(Pos, coordToHashMove(MoveStr), &TmpMove) != 0
        || memcmp(&TmpMove, &MoveStack[MvBase + i], sizeof(move)) != 0)
    {
      fprintf(stderr, "\nMove verification failed:\n");
      fprintf(stderr, "FEN: %s\n", exportFEN(FENStr, Pos));
      fprintf(stderr, "Move: %s\n", MoveStr);
      return -1;
    }
    
    NewPos = *Pos;
    if (quickMakeMove(&NewPos, MoveStack[MvBase + i]) == 0)
    {
      if (!isConsistent(&NewPos))
      {
        fprintf(stderr, "\nInconsistent position as result of move:\n");
        fprintf(stderr, "FEN: %s\n", exportFEN(FENStr, Pos));
        fprintf(stderr, "Move: %s\n", MoveStr);
        return -1;
      }
      Count = mgtestCount(&NewPos, Depth-1);
      if (Count < 0)
        return -1;
      Total += Count;
    }
  }

  popMoveStack(MvBase);

  return Total;
}

int mgtest(const char *FileName)
{
  position Pos;
  FILE *File;
  char Line[512];
  char *Str;
  int LineNum = 1;
  int Depth;
  int64 ExpCount, Count;
  microtime Time, TotalTime;

  File = fopen(FileName, "r");
  if (!File)
  {
    perror(FileName);
    return 1;
  }

  TotalTime = getMicroTime();
  while (fgets(Line, 511, File))
  {
    if (importFEN(&Pos, Line) != 0)
    {
      fprintf(stderr, "%s: line %i: invalid FEN\n\n", FileName, LineNum);
      fclose(File);
      return 1;
    }

    printf("Line %i: %s\n", LineNum, exportFEN(FENStr, &Pos));

    Str = Line;
    while ((Str = strchr(Str + 1, ';')))
    {
      if (sscanf(Str, " ;D%i %"_i64, &Depth, &ExpCount) != 2)
      {
        fprintf(stderr, "%s: line %i: invalid data\n\n", FileName, LineNum);
        fclose(File);
        return 1;
      }

      printf("    Depth: %i Exp. Variations: %12"_i64"    ", Depth, ExpCount);
      Nodes = 0;
      resetMoveStack();
      Time = getMicroTime();
      Count = mgtestCount(&Pos, Depth);
      Time = getMicroTime() - Time;
      if (Count < 0)
      {
        fclose(File);
        return 1;
      }
      printf("Time: %"_i64".%.3"_i64"s ", toSeconds(Time), mSecPart(Time));
      if (Time)
        printf("(%"_u64" n/s)\n", (Nodes*ONE_SEC)/Time);
      else
        printf("(%"_u64"+ n/s)\n", Nodes);
      if (Count != ExpCount)
      {
        printf("ERROR: line %i data does not match\n\n", LineNum);
        fclose(File);
        return 1;
      }
    };

    LineNum++;
  }
  TotalTime = getMicroTime() - TotalTime;

  printf("Move generation test completed.\n");
  printf("Total Time (m:ss): %"_i64":%.2"_i64"\n\n",
         toMinutes(TotalTime), secondsPart(TotalTime));
  fclose(File);

  return 0;
}

int printVariations(const char *Fen, int Depth)
{
  position Pos;
  position NewPos;
  int MvBase;
  int nMoves, nLegalMoves = 0;
  uint64 n, Total = 0;
  char MoveStr[8];
  int i;
  microtime Time, TotalTime;

  if (!Fen) {
    Fen = STARTPOS;
    printf("No position specified. Using standard staring position.\n");
  }
  if (importFEN(&Pos, Fen) != 0) {
    fprintf(stderr, "Invalid FEN:\n  %s\n", Fen);
    return 1;
  }
  printf("\nPosition: %s\nDepth: %i ply\n\n", Fen, Depth);
  resetMoveStack();

  TotalTime = getMicroTime();
  Nodes = 1;
  MvBase = getMoveStackTop();
  nMoves = genCaptures(&Pos) + genQuietMoves(&Pos);
  for (i = 0; i < nMoves; i++)
  {
    NewPos = Pos;
    if (quickMakeMove(&NewPos, MoveStack[MvBase + i]) == 0)
    {
      Time = getMicroTime();
      n = countVariations(&NewPos, Depth-1);
      Time = getMicroTime() - Time;
      Total += n;
      getLANStr(MoveStack[MvBase + i], MoveStr);
      printf("%s: %"_u64" (%"_i64".%.3"_i64"s)\n", MoveStr, n,
          toSeconds(Time), mSecPart(Time));
      nLegalMoves++;
    }
  }
  popMoveStack(MvBase);
  TotalTime = getMicroTime() - TotalTime;

  printf("Legal moves: %i\n", nLegalMoves);
  printf("\nNodes: %"_u64" \tTime: %"_i64".%.3"_i64"s \t",
         Nodes, toSeconds(TotalTime), mSecPart(TotalTime));
  if (TotalTime)
    printf("Rate: %"_u64" n/s\n", (Nodes*ONE_SEC)/TotalTime);
  else
    printf("Rate: %"_u64"+ n/s\n", Nodes);
  printf("Total variations: %"_u64"\n\n", Total);

  return 0;
}

int perftest(const char *Fen, int Depth)
{
  uint64 Count;
  microtime Time;
  position Pos;

  if (!Fen) {
    Fen = STARTPOS;
    printf("No position specified. Using standard staring position.\n");
  }
  if (importFEN(&Pos, Fen) != 0) {
    fprintf(stderr, "Invalid FEN:\n  %s\n", Fen);
    return 1;
  }
  printf("\nPosition: %s\nDepth: %i ply\n\n", Fen, Depth);

  Nodes = 0;
  resetMoveStack();
  Time = getMicroTime();
  Count = countVariations(&Pos, Depth);
  Time = getMicroTime() - Time;

  printf("Nodes: %"_u64" \tTime: %"_i64".%.3"_i64"s \t",
         Nodes, toSeconds(Time), mSecPart(Time));
  if (Time)
    printf("Rate: %"_u64" n/s\n", (Nodes*ONE_SEC)/Time);
  else
    printf("Rate: %"_u64"+ n/s\n", Nodes);
  printf("Total variations: %"_u64"\n", Count);

  return 0;
}

/* end of file */
