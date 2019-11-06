/******************************************************************************
 * $Id: eval.c 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Evaluate a position.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "eval.h"

const int PieceVal[NUM_PIECES] = { 0, 100, 320, 330, 500, 1000 };

int PcSqVal[NUM_PIECES][NUM_SQUARES] = {
    // NO_PIECE
    { 0 },
    // PAWN
    {
    //  1    2    3    4    5    6    7   8
        0,   5,   4,  -5,   5,  10,  70,  0, // a
        0,  10,  -5,  -2,   7,  15,  70,  0, // b
        0,  10,  -5,   2,  10,  20,  70,  0, // c
        0, -25,   5,  15,  20,  30,  70,  0, // d
        0, -25,   5,  15,  20,  30,  70,  0, // e
        0,  10, -10,   0,  10,  20,  70,  0, // f
        0,  10,  -5,  -2,   7,  15,  70,  0, // g
        0,   5,   4,  -5,   5,  10,  70,  0, // h
    },
    // KNIGHT
    {
    //    1    2    3    4    5    6    7    8
        -40, -30, -20, -20, -20, -20, -30, -40, // a
        -30, -10,   7,   5,   5,   7, -10, -30, // b
        -20,   0,  10,  15,  15,  12,   0, -20, // c
        -20,   5,  12,  20,  25,  15,   0, -20, // d
        -20,   5,  12,  20,  25,  15,   0, -20, // e
        -20,   0,  10,  15,  15,  12,   0, -20, // f
        -30, -10,   7,   5,   5,   7, -10, -30, // g
        -40, -30, -20, -20, -20, -20, -30, -40, // h
    },
};

int evaluate(const position *Pos)
{
  int Val[NUM_COLORS] = { 0, 0 };
  square Sq;
  bitboard Bd;

  for (piece p = PAWN; p < KING; p++)
  {
    Bd = Pos->OccBy[WHITE][p];
    while (Bd)
    {
      Sq = firstSq(Bd);
      CLEARLSB(Bd);
      Val[WHITE] += PieceVal[p];
      Val[WHITE] += PcSqVal[p][Sq];
    }
    Bd = Pos->OccBy[BLACK][p];
    while (Bd)
    {
      Sq = firstSq(Bd);
      CLEARLSB(Bd);
      Val[BLACK] += PieceVal[p];
      Val[BLACK] += PcSqVal[p][Sq^007];
    }
  }

  if (Pos->Flags & PF_WHITEMOVE)
    return Val[WHITE] - Val[BLACK];
  else
    return Val[BLACK] - Val[WHITE];
}

/* end of file */
