/******************************************************************************
 * $Id: notation.c 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Functions and data related to chess notation.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "notation.h"

#include <string.h>

const char PIECE_CHAR[NUM_PIECES + 1] = { 
    0, 'P', 'N', 'B', 'R', 'Q', 'K',
};

const char SQUARE_NAME[NUM_SQUARES][3] = {
    "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8",
    "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8",
    "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8",
    "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8",
    "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8",
    "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8",
    "g1", "g2", "g3", "g4", "g5", "g6", "g7", "g8",
    "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8",
};

int getLANStr(move Move, char Str[])
{
  int i = 0;

  assert(Move.Piece >= PAWN && Move.Piece <= KING);
  assert(Move.Orig >= a1 && Move.Orig <= h8);
  assert(Move.Dest >= a1 && Move.Dest <= h8);

  if (Move.Type == MT_CASTLE)
  {
    if (Move.Dest > Move.Orig)
      strcpy(Str, "O-O");   // king-side
    else
      strcpy(Str, "O-O-O"); // queen-side
  }
  else
  {
    if (Move.Piece > PAWN)
      Str[i++] = PIECE_CHAR[Move.Piece];
    Str[i++] = SQUARE_NAME[Move.Orig][0];
    Str[i++] = SQUARE_NAME[Move.Orig][1];
    if (Move.CaptPc == NO_PIECE)
      Str[i++] = '-';
    else
      Str[i++] = 'x';
    Str[i++] = SQUARE_NAME[Move.Dest][0];
    Str[i++] = SQUARE_NAME[Move.Dest][1];
    if (Move.PromPc != NO_PIECE)
    {
      Str[i++] = '=';
      Str[i++] = PIECE_CHAR[Move.PromPc];
    }
    Str[i++] = 0;
  }

  return 0;
}

/* end of file */
