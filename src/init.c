/******************************************************************************
 * $Id: init.c 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Performs initialization.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "init.h"
#include "moves.h"
#include "game.h"

#include <stdlib.h>
#include <time.h>

/* diagonal and antidiagonal masks */
const bitboard DIAGMASK[NUM_SQUARES];     // a1 to h8 direction
const bitboard ANTIDIAGMASK[NUM_SQUARES]; // a8 to h1 direction

static void initMasks(void)
{
  bitboard *const Diagonal = (bitboard *)DIAGMASK;
  bitboard *const Antidiag = (bitboard *)ANTIDIAGMASK;
  file f;
  rank r;
  square sq;
  bitboard Bd1;
  bitboard Bd2;

  /* diagonal */
  Bd1 = 0x8040201008040201;
  for (f = F_a; f < NUM_FILES; f++)
  {
    Bd2 = Bd1 << f * 8;

    for (sq = SQUARE(f, R_1); sq < NUM_SQUARES; sq += 9)
      Diagonal[sq] = Bd2;
  }
  for (r = R_2; r < NUM_RANKS; r++)
  {
    Bd2 = Bd1 >> r * 8;

    for (sq = SQUARE(F_a, r); RANK(sq) >= r; sq += 9)
      Diagonal[sq] = Bd2;
  }

  /* antidiagonal */
  Bd1 = 0x0102040810204080;
  // NOTE: this initially puts an erroneous value in Antidiag[h8], but this is
  //       later overwritten by the correct value.
  for (f = F_a; f < NUM_FILES; f++)
  {
    Bd2 = Bd1 << f * 8;

    for (sq = SQUARE(f, R_8); sq < NUM_SQUARES; sq += 7)
      Antidiag[sq] = Bd2;
  }
  for (r = R_1; r < R_8; r++)
  {
    Bd2 = Bd1 >> (R_8 - r) * 8;

    for (sq = SQUARE(F_a, r); RANK(sq) <= r; sq += 7)
      Antidiag[sq] = Bd2;
  }
}

static void initKnightAttacks(void)
{
  bitboard *const NAtt = (bitboard *)KNIGHT_ATT;
  file f;
  rank r;
  square sq;

  for (sq = a1; sq < NUM_SQUARES; sq++)
  {
    NAtt[sq] = 0;

    f = FILE(sq);
    r = RANK(sq);
    if (f > F_b && r > R_1)
      NAtt[sq] |= SQMASKFR(f-2, r-1);
    if (f > F_b && r < R_8)
      NAtt[sq] |= SQMASKFR(f-2, r+1);
    if (f > F_a && r > R_2)
      NAtt[sq] |= SQMASKFR(f-1, r-2);
    if (f > F_a && r < R_7)
      NAtt[sq] |= SQMASKFR(f-1, r+2);
    if (f < F_h && r > R_2)
      NAtt[sq] |= SQMASKFR(f+1, r-2);
    if (f < F_h && r < R_7)
      NAtt[sq] |= SQMASKFR(f+1, r+2);
    if (f < F_g && r > R_1)
      NAtt[sq] |= SQMASKFR(f+2, r-1);
    if (f < F_g && r < R_8)
      NAtt[sq] |= SQMASKFR(f+2, r+1);
  }
}

static void initKingAttacks(void)
{
  bitboard *const KAtt = (bitboard *)KING_ATT;
  file f;
  rank r;
  square sq;

  for (sq = a1; sq < NUM_SQUARES; sq++)
  {
    KAtt[sq] = 0;

    f = FILE(sq);
    r = RANK(sq);
    if (f > F_a && r > R_1)
      KAtt[sq] |= SQMASKFR(f-1, r-1);
    if (f > F_a)
      KAtt[sq] |= SQMASKFR(f-1, r);
    if (f > F_a && r < R_8)
      KAtt[sq] |= SQMASKFR(f-1, r+1);
    if (r > R_1)
      KAtt[sq] |= SQMASKFR(f, r-1);
    if (r < R_8)
      KAtt[sq] |= SQMASKFR(f, r+1);
    if (f < F_h && r > R_1)
      KAtt[sq] |= SQMASKFR(f+1, r-1);
    if (f < F_h)
      KAtt[sq] |= SQMASKFR(f+1, r);
    if (f < F_h && r < R_8)
      KAtt[sq] |= SQMASKFR(f+1, r+1);
  }
}

static void initFileAttacks(void)
{
  bitboard (*const FileAtt)[64] = (bitboard (*)[64])FILE_ATT;
  unsigned char Occ;
  unsigned char TestBit;
  unsigned char Result;
  rank r;

  for (r = R_1; r < NUM_RANKS; r++)
  {
    for (Occ = 0; Occ < 128; Occ += 2)
    {
      Result = 0;

      for (TestBit = 1 << (r + 1); TestBit; TestBit <<= 1)
      {
        Result |= TestBit;
        if (Occ & TestBit)
          break;
      }
      for (TestBit = (1 << (r - 1)); TestBit; TestBit >>= 1)
      {
        Result |= TestBit;
        if (Occ & TestBit)
          break;
      }

      FileAtt[r][Occ >> 1] = Result;
    }
  }
}

static void initAttackTables(void)
{
  initKnightAttacks();
  initKingAttacks();
  initFileAttacks();
}

void init(void)
{
  static int Initialized = 0;

  if (Initialized)
    return;

  initMasks();
  initAttackTables();

  resetGame();

  srand(time(NULL));

  Initialized = 1;
}

/* end of file */
