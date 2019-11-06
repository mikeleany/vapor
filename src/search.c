/******************************************************************************
 * $Id: search.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Searches for the best moves.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "search.h"
#include "game.h"
#include "moves.h"
#include "microtime.h"
#include "eval.h"
#include "zobrist.h"
#include "hash.h"

#include <stdlib.h>
#include <string.h>

#define CLOCK_NODES 1024 // check clock roughly every millisecond 
#define INPUT_NODES (32*CLOCK_NODES)  // roughly 30 times per second

uint16 Now = 0; // number of calls to searchRoot, used for hash aging

zobrist SearchHist[MAX_ZHIST_LEN*2];
int HistLength;

void (*printPV)(void) = NULL;
void (*checkInput)(void) = NULL;

typedef struct variation
{
  int Length;
  move Move[MAX_PLY];
  hash_entry Hash[MAX_PLY];
} variation;

/* globals */
int StopSearch;
int64 Nodes;
microtime StopTime;
microtime ExtStopTime;

microtime setupClock(void)
{
  color MyColor = (CurPos->Flags & PF_WHITEMOVE)?WHITE:BLACK;
  microtime StartTime = getMicroTime();
  microtime Time, Inc;

  StopSearch = 0;
  if (Search.Flags & SF_INFINITE)
    ExtStopTime = StopTime = 0;
  if (Search.MoveTime)
    ExtStopTime = StopTime = StartTime + Search.MoveTime;
  else if (Search.Time[MyColor])
  {
    Time = Search.Time[MyColor];
    Inc = Search.Inc[MyColor];

    if (!Search.MovesToGo || Search.MovesToGo > 7)
    {
      if (Time > 6*Inc)
      {
        StopTime = StartTime + Time/30 + Inc;
        ExtStopTime = StartTime + 2*Time/30 + Inc;
      }
      else
      {
        StopTime = StartTime + Time/5;
        ExtStopTime = StartTime + 2*Time/5;
      }
    }
    else
    {
      StopTime = StartTime + Time/(Search.MovesToGo*4);
      ExtStopTime = StartTime + 2*Time/(Search.MovesToGo*4);
    }
  }
  else
    ExtStopTime = StopTime = 0;

  return StartTime;
}

static inline int timeToStop(void)
{
  if (checkInput && Nodes%INPUT_NODES == 0)
  {
    checkInput();
    if (Search.Flags & SF_STOPPED)
    {
      StopSearch = 1;
      return 1;
    }
  }

  if (!(Search.Flags & SF_PONDER) && !(Search.Flags & SF_INFINITE))
  {
    if (Search.MaxNodes && Nodes >= Search.MaxNodes)
    {
      StopSearch = 1;
      return 1;
    }
    else if (StopTime && Nodes%CLOCK_NODES == 0 && getMicroTime() >= StopTime)
    {
      StopSearch = 1;
      return 1;
    }
  }

  return 0;
}

static inline int hashScore(int Score, int CurPly)
{
  if (Score >= LONG_MATE) {
    return Score + CurPly;
  } else if (Score <= -LONG_MATE) {
    return Score - CurPly;
  } else {
    return Score;
  }
}

static inline int unhashScore(int Score, int CurPly)
{
  if (Score >= LONG_MATE) {
    return Score - CurPly;
  } else if (Score <= -LONG_MATE) {
    return Score + CurPly;
  } else {
    return Score;
  }
}

int search(const position *Pos, int Ply, int Depth, int Alpha, int Beta,
           variation *LocalPV);
int quiesce(const position *Pos, int Alpha, int Beta);

void searchRoot(void)
{
  int i, j;
  int MvBase;
  int nMoves;
  move *MoveList;
  position *PosList;
  int Val;
  int BestVal = -INFINITY;
  int BestMove = 0;
  microtime StartTime;
  int Depth, MaxDepth = Search.MaxDepth?Search.MaxDepth:MAX_SEARCH_DEPTH;
  move TmpMove;
  position TmpPos;
  variation NewPV;
  const hash_entry *OldHash;
  hash_entry PVHash[MAX_PLY];

  Now++;

  // set up search history
  HistLength = ZHistLength;
  memcpy(SearchHist, ZobHistory, HistLength*sizeof(zobrist));


  StartTime = setupClock();
  Nodes = 1;
  resetMoveStack();
  MvBase = getMoveStackTop();
  nMoves = genCaptures(CurPos) + genQuietMoves(CurPos);

  PosList = calloc(nMoves, sizeof(position));
  MoveList = calloc(nMoves, sizeof(move));

  // make all moves, removing illegal moves
  for (i = 0, j = 0; i < nMoves; i++)
  {
    PosList[j] = *CurPos;
    if (quickMakeMove(&PosList[j], MoveStack[MvBase+i]) == 0)
      MoveList[j++] = MoveStack[MvBase+i];
  }
  resetMoveStack();
  nMoves = j;
  // note that the following frees the memory if nMoves is zero
  PosList = realloc(PosList, nMoves*sizeof(position));
  MoveList = realloc(MoveList, nMoves*sizeof(move));
  if (!nMoves) // no legal moves
  {
    PVData.Length = 0;
    return;
  }

  // lookup hash move
  OldHash = hashLookup(CurPos->ZKey);
  if (OldHash && OldHash->Move) {
    for (i = 0; i < nMoves; i++) {
      if (OldHash->Move == getHashMove(&MoveList[i])) {
        BestMove = i;
        break;
      }
    }
  }

  // iterative deepening
  for (Depth = 1; Depth <= MaxDepth; Depth++)
  {
    BestVal = -INFINITY;
    if (BestMove > 0)
    {
      // put previous best move at the front of the list (move others down)
      TmpMove = MoveList[BestMove];
      TmpPos = PosList[BestMove];
      for (i = BestMove; i > 0; i--)
      {
        MoveList[i] = MoveList[i-1];
        PosList[i] = PosList[i-1];
      }
      MoveList[0] = TmpMove;
      PosList[0] = TmpPos;
      BestMove = 0;
    }

    // begin search
    for (i = 0; i < nMoves; i++)
    {
      NewPV.Length = 0;
      SearchHist[HistLength++] = CurPos->ZKey;
      Val = -search(&PosList[i], 1, Depth-1, -INFINITY, -BestVal, &NewPV);
      HistLength--;
      resetMoveStack();
      if (StopSearch)
      {
        MaxDepth = Depth;
        break;
      }
      if (Val > BestVal)
      {
        BestVal = Val;
        BestMove = i;
        PVData.Val = Val;
        PVData.Depth = Depth;
        PVData.Length = NewPV.Length + 1;
        PVData.Move[0] = MoveList[BestMove];
        for (j = 0; j < NewPV.Length; j++) {
          PVData.Move[j+1] = NewPV.Move[j];
          PVHash[j+1] = NewPV.Hash[j];
        }
      }
    }

    // store node and time info in PVData
    PVData.Time = getMicroTime() - StartTime;
    PVData.Nodes = Nodes;
    if (PVData.Time > 0)
      PVData.NodesPerSec = (Nodes*ONE_SEC)/PVData.Time;
    else
      PVData.NodesPerSec = Nodes;

    // store PV positions in hash
    PVHash[0].ZKey = CurPos->ZKey;
    PVHash[0].Bound = exactscore;
    PVHash[0].Depth = PVData.Depth;
    PVHash[0].When = Now;
    PVHash[0].Score = hashScore(PVData.Val, 0);
    PVHash[0].Move = getHashMove(&PVData.Move[0]);
    for (i = 0; i < PVData.Length; i++) {
      saveToHash(&PVHash[i]);
    }

    // print the pv data
    if (printPV)
      printPV();
  }

  free(MoveList);
  free(PosList);
  resetMoveStack();
}

typedef enum searchstate
{
  INIT_SEARCH,
  CAPT_SEARCH,
  ALL_SEARCH,
  SEARCH_DONE,
} searchstate;

static inline int getNextMove(const position *Pos, int Index,
    searchstate *State, int *StackTop)
{
  while (Index >= *StackTop)
  {
    switch (*State)
    {
      case INIT_SEARCH:
        // TODO: search only check evasions if in check
        *StackTop += genCaptures(Pos);
        (*State)++;
        break;
      case CAPT_SEARCH:
        *StackTop += genQuietMoves(Pos);
        (*State)++;
        break;
      case ALL_SEARCH:
        // nothing left to search
        (*State)++;
        return -1;
      default:
        (*State) = SEARCH_DONE;
        return -1;
    }
  }

  return Index;
}

int search(const position *Pos, int Ply, int Depth, int Alpha, int Beta,
           variation *LocalPV)
{
  int StackTop = getMoveStackTop();
  searchstate State = INIT_SEARCH;
  int NextIndex = StackTop;
  int Move;
  int Val;
  int BestVal = -INFINITY;
  int BestMove = -1;
  int nLegalMoves = 0;
  position NewPos;
  variation NextPV;
  const hash_entry *OldHash = NULL;
  hash_entry NewHash = {
    Pos->ZKey,  // zobrist key
    upperbound, // bound on the value
    Depth,      // search depth
    Now,        // when the entry was created
    BestVal,    // score
    0,          // best move
  };
  hashmove HashMove = 0;

  Nodes++;
  NextPV.Length = 0;

  // 50 move draw detection
  if (Pos->DrawPlies >= 100)
    return 0;
  // repetition draw detection
  for (int i = 2; i < Pos->DrawPlies+2; i++)
  {
    if (SearchHist[HistLength-i] == Pos->ZKey)
      return 0;
  }

  // check extension
  if (Pos->Flags & PF_CHECK)
    Depth++;

  OldHash = hashLookup(Pos->ZKey);
  if (OldHash) {
    Val = unhashScore(OldHash->Score, Ply);
    if (OldHash->Depth >= Depth) {
      if (Val >= Beta) {
        // beta cut-off unless it's only an upper bound (could be lower)
        if (OldHash->Bound != upperbound)
          return Val;
      } else if (Val <= Alpha) {
        // alpha cut-off unless it's only a lower bound (could be higher)
        if (OldHash->Bound != lowerbound)
          return Val;
      } else if (OldHash->Bound == exactscore) {
        // Alpha < Exact Score < Beta ==> PV Node
        HashMove = OldHash->Move;
        // for PV nodes fully verify move legality before returning
        if (OldHash->Move && genHashMove(Pos, HashMove)) {
          NewPos = *Pos;
          if (quickMakeMove(&NewPos, MoveStack[NextIndex]) == 0) {
            LocalPV->Length = 1;
            LocalPV->Move[0] = MoveStack[NextIndex];
            LocalPV->Hash[0] = *OldHash;
            return Val;
          } else {
            NextIndex++; // no use retrying the move if it's not legal
          }
        }
        // since the move isn't legal, we've got a rare hash-key conflict
        // so lets just go on searching since we didn't find anything useful
      }
    }
    // if there's a hash move that we haven't already tried
    if (OldHash->Move && !HashMove) {
      HashMove = OldHash->Move;
      StackTop += genHashMove(Pos, HashMove);
    }
  }

  // if leaf node, enter qsearch
  if (Depth <= 0)
    return quiesce(Pos, Alpha, Beta);

  if (timeToStop())
    return INFINITY;

  while ((Move = getNextMove(Pos, NextIndex++, &State, &StackTop)) >= 0)
  {
    if (nLegalMoves && HashMove && OldHash->Move == getHashMove(&MoveStack[Move]))
      continue; // no need to search the hashmove twice

    NewPos = *Pos;
    if (quickMakeMove(&NewPos, MoveStack[Move]) == 0)
    {
      nLegalMoves++;
      SearchHist[HistLength++] = Pos->ZKey;
      Val = -search(&NewPos, Ply+1, Depth-1, -Beta, -Alpha, &NextPV);
      HistLength--;
      if (StopSearch)
        return INFINITY;
      if (Val >= Beta) {
        NewHash.Score = hashScore(Val, Ply);
        NewHash.Bound = lowerbound;
        NewHash.Move = getHashMove(&MoveStack[Move]);
        saveToHash(&NewHash);
        return Val;
      } else if (Val > Alpha) {
        Alpha = BestVal = Val;
        BestMove = Move;
      } else if (Val > BestVal) {
        BestVal = Val;
      }
    }
    popMoveStack(StackTop); // restore the stack for the next move
  }

  if (!nLegalMoves) {
    if (Pos->Flags & PF_CHECK)
      BestVal = -INFINITY + Ply; // checkmate
    else
      BestVal = 0; // stalemate
    if (BestVal > Alpha && BestVal < Beta)
      LocalPV->Length = 0;
  }

  NewHash.Score = hashScore(BestVal, Ply);

  if (BestMove >= 0) {
    NewHash.Bound = exactscore;
    NewHash.Move = getHashMove(&MoveStack[BestMove]);
    LocalPV->Length = NextPV.Length + 1;
    LocalPV->Move[0] = MoveStack[BestMove];
    LocalPV->Hash[0] = NewHash;
    for (int i = 0; i < NextPV.Length; i++) {
      LocalPV->Move[i+1] = NextPV.Move[i];
      LocalPV->Hash[i+1] = NextPV.Hash[i];
    }
  }
  saveToHash(&NewHash);
  return BestVal;
}

int quiesce(const position *Pos, int Alpha, int Beta)
{
  int MvBase = getMoveStackTop();
  int Val;
  int StandPat = evaluate(Pos);
  int BestVal = StandPat;
  int nMoves;
  position NewPos;

  if (timeToStop())
    return INFINITY;

  // check stand pat score against alpha and beta
  if (StandPat >= Beta)
    return StandPat;
  if (StandPat > Alpha)
    Alpha = StandPat;

  // best possible gain is from PxQ=Q with no recapture
  // if even that's not enough, don't bother generating moves
  if (StandPat + 2*PieceVal[QUEEN] <= Alpha)
    return StandPat + 2*PieceVal[QUEEN];

  // begin searching moves
  nMoves = genCaptures(Pos);
  for (int i = 0; i < nMoves; i++)
  {
    // determine if remaining moves can help raise alpha
    if (MoveStack[MvBase+i].PromPc == NO_PIECE
        && StandPat + PieceVal[MoveStack[MvBase+i].CaptPc] < Alpha)
    {
      return max(StandPat + PieceVal[MoveStack[MvBase+i].CaptPc], BestVal);
    }

    // search the next move
    NewPos = *Pos;
    if (quickMakeMove(&NewPos, MoveStack[MvBase+i]) == 0)
    {
      Nodes++;
      Val = -quiesce(&NewPos, -Beta, -Alpha);
      if (StopSearch)
        return INFINITY;
      if (Val >= Beta)
        return Val;
      else if (Val > Alpha)
        Alpha = BestVal = Val;
      else if (Val > BestVal)
        BestVal = Val;
    }
    popMoveStack(MvBase + nMoves); // restore the stack for the next move
  }

  return BestVal;
}

/* end of file */
