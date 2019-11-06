/******************************************************************************
 * $Id: game.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Maintain the game history.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "game.h"
#include "fen.h"
#include "notation.h"
#include "moves.h"

#include <string.h>

/* the game state */
char StartFEN[128] = "";
hashmove MoveList[MAX_GAME_PLIES] = { 0 };
int nMoves = 0;
position Pos;

/* game zobrist history */
zobrist ZobHistory[MAX_ZHIST_LEN] = { 0 };
int ZHistLength = 0;

/* pointer to the current position */
const position *CurPos = &Pos;

/******************************************************************************
 * void resetGame(void)
 * DESCRIPTION
 *    Resets the game to the standard starting position. Clears any previous
 *    game history.
 * RETURN VALUE
 *    Does not return a value.
 */
void resetGame(void)
{
  int Result;

  Result = setGamePos(STARTPOS);
  assert(Result == 0);
}

/******************************************************************************
 * int setGamePos(const char *FENStr)
 * PARAMETERS
 *    FENStr -- FEN of the position to set.
 * DESCRIPTION
 *    Sets the current position to that given by FENStr. Clears any previous
 *    game history.
 * RETURN VALUE
 *    Returns 0 for success or -1 for failure.
 */
int setGamePos(const char *FENStr)
{
  int Result;

  nMoves = 0;
  ZHistLength = 1;
  strncpy(StartFEN, FENStr, 127);
  StartFEN[127] = 0;

  Result = importFEN(&Pos, StartFEN);
  ZobHistory[0] = Pos.ZKey;

  return Result;
}

/******************************************************************************
 * int makeGameMove(const char *MoveStr);
 * PARAMETERS
 *    MoveStr -- The move to make in coordinate notation.
 * DESCRIPTION
 *    Makes the move on the game board, updating the current position and game
 *    history as appropriate.
 * RETURN VALUE
 *    Returns 0 for success or -1 for failure.
 */
int makeGameMove(const char *MoveStr)
{
  move Move;
  int Result;

  assert(nMoves < MAX_GAME_PLIES);
  if (nMoves >= MAX_GAME_PLIES)
  {
    Pos.Flags |= PF_INVALID;
    return -1;
  }

  MoveList[nMoves] = coordToHashMove(MoveStr);
  if (expandMove(&Pos, MoveList[nMoves++], &Move) != 0)
  {
    Pos.Flags |= PF_INVALID;
    return -1;
  }

  Result = quickMakeMove(&Pos, Move);
  if (Pos.DrawPlies < MAX_ZHIST_LEN)
  {
    ZobHistory[Pos.DrawPlies] = Pos.ZKey;
    ZHistLength = Pos.DrawPlies + 1;
  }

  return Result;
}

/* end of file */
