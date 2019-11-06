/******************************************************************************
 * $Id: notation.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Functions and data related to chess notation.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef vapor__NOTATION_H
#define VAPOR__NOTATION_H

#include "vapor.h"
#include "chess.h"

#include <ctype.h>

extern const char PIECE_CHAR[NUM_PIECES + 1];
extern const char SQUARE_NAME[NUM_SQUARES][3];

static inline piece charToPiece(char ch)
{
  switch (toupper(ch))
  {
    case 'K':
      return KING;
    case 'Q':
      return QUEEN;
    case 'R':
      return ROOK;
    case 'B':
      return BISHOP;
    case 'N':
      return KNIGHT;
    case 'P':
      return PAWN;
    default:
      return 0; // no such piece
  }
}

static inline square strToSquare(const char *s)
{
  if (!s || (s[0] < 'a' || s[0] > 'h') || (s[1] < '1' || s[1] > '8'))
    return NO_SQUARE;

  return SQUARE(s[0]-'a', s[1]-'1');
}

int getLANStr(move Move, char Str[]);
static inline int getCoordStr(move Move, char MoveStr[])
{
  assert(Move.Orig >= a1 && Move.Orig <= h8);
  assert(Move.Dest >= a1 && Move.Dest <= h8);

  MoveStr[0] = SQUARE_NAME[Move.Orig][0];
  MoveStr[1] = SQUARE_NAME[Move.Orig][1];
  MoveStr[2] = SQUARE_NAME[Move.Dest][0];
  MoveStr[3] = SQUARE_NAME[Move.Dest][1];
  if (Move.PromPc == NO_PIECE)
    MoveStr[4] = 0;
  else
  {
    MoveStr[4] = PIECE_CHAR[Move.PromPc];
    MoveStr[5] = 0;
  }

  return 0;
}

static inline hashmove coordToHashMove(const char *Str)
{
  square Orig, Dest;

  Orig = strToSquare(Str);
  if (Orig == NO_SQUARE)
    return 0;
  Dest = strToSquare(Str + 2);
  if (Dest == NO_SQUARE)
    return 0;

  return (Orig << 9) | (Dest << 3) | charToPiece(Str[4]);
}

#endif // #ifndef VAPOR__NOTATION_H

/* end of file */
