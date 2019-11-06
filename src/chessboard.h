/******************************************************************************
 * $Id: chessboard.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Board related types and bitboard operations.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__BOARD_H
#define VAPOR__BOARD_H

#include "vapor.h"

/******************************************************************************
 * board-related types
 */

/* file indicates a vertical column on the board */
typedef enum file
{
  F_a, F_b, F_c, F_d, F_e, F_f, F_g, F_h,
  NO_FILE = -1
} file;
#define NUM_FILES (F_h + 1)

/* rank indicates a horizontal row on the board */
typedef enum rank
{
  R_1, R_2, R_3, R_4, R_5, R_6, R_7, R_8,
  NO_RANK = -1
} rank;
#define NUM_RANKS (R_8 + 1)

/* square represents the intersection of a file and rank on the board */
typedef enum square
{
  a1, a2, a3, a4, a5, a6, a7, a8,
  b1, b2, b3, b4, b5, b6, b7, b8,
  c1, c2, c3, c4, c5, c6, c7, c8,
  d1, d2, d3, d4, d5, d6, d7, d8,
  e1, e2, e3, e4, e5, e6, e7, e8,
  f1, f2, f3, f4, f5, f6, f7, f8,
  g1, g2, g3, g4, g5, g6, g7, g8,
  h1, h2, h3, h4, h5, h6, h7, h8,

  NO_SQUARE = -1
} square;
#define NUM_SQUARES (h8+1)

/* bitboard indicates a boolean value for each square on the board */
typedef uint64 bitboard;

/******************************************************************************
 * board-related macros and data
 */

/* translations between square type and rank/file types */
#define SQUARE(F, R) ((square)(((F)*8) + (R)))
#define FILE(S) ((file)((S) / 8))
#define RANK(S) ((rank)((S) % 8))

/* square, file and rank masks */
#define SQMASK(S) ((bitboard)1 << (S))
#define SQMASKFR(F, R) (SQMASK(SQUARE((F), (R))))
#define FILEMASK(F) (0xffLL << ((F)*8))
#define RANKMASK(R) (0x0101010101010101 << (R))

/* diagonal and antidiagonal masks */
extern const bitboard DIAGMASK[NUM_SQUARES];      // a1 to h8 direction
extern const bitboard ANTIDIAGMASK[NUM_SQUARES];  // a8 to h1 direction

/* square testing */
#define TESTSQ(B, S) ((B) & SQMASK(S))
#define TESTSQFR(B, F, R) ((B) & SQUARE((F), (R)))

/* square manipulation */
#define SETSQ(B, S) ((B) |= SQMASK(S))
#define SETSQFR(B, F, R) (SETSQ((B), SQUARE((F), (R))))
#define CLEARSQ(B, S) ((B) &= ~SQMASK(S))
#define CLEARSQFR(B, F, R) (CLEARSQ((B), SQUARE((F), (R))))
#define TOGGLESQ(B, S) ((B) ^= SQMASK(S))
#define TOGGLESQFR(B, F, R) (TOGGLESQ((B), SQUARE((F), (R))))

/* macros for working with the least significant bit */
#define LSB(B)      ((B) & -(B))
#define CLEARLSB(B) ((B) &= ((B)-1))

/******************************************************************************
 * int popCnt(register bitboard Bd);
 * PARAMETERS
 *    Bd - The bitboard to search.
 * DESCRIPTION
 *    Determines the number of bits set in Bd.
 * RETURN VALUE
 *    Returns the number of bits.
 */
static inline int popCnt(register bitboard Bd)
{
  register int Cnt = 0;

  while (Bd)
  {
    Cnt++;
    CLEARLSB(Bd);
  }

  return Cnt;
}


/******************************************************************************
 * square firstSq(bitboard Bd);
 * PARAMETERS
 *    Bd - The bitboard to search.
 * DESCRIPTION
 *    Determines the square corresponding to the first (least significant) bit
 *    set in Bd. The least significant bit corresponds to the lowest numerical
 *    rank, within the lowest file alphabetically.
 * RETURN VALUE
 *    Returns the resulting square.
 * PORTABILITY ISSUES
 *    Contains gcc-specific code.
 */
#ifdef __x86_64__
static inline square firstSq(bitboard Bd)
{
  uint64 Sq;  // must be 64-bit for assembly instruction

  if (Bd)
  {
    asm("bsf %1, %0" : "=r" (Sq) : "rm" (Bd));
    return (square)Sq;
  }
  else
    return NO_SQUARE;    
}
#else // __x86_64__ not defined
static inline square firstSq(bitboard Bd)
{
  uint32 Hi = Bd >> 32;
  uint32 Lo = Bd & 0xffffffff;
  uint32 Sq;  // must be 32-bit for assembly instruction

  if (Lo)
  {
    asm("bsf %1, %0" : "=r" (Sq) : "rm" (Lo));
    return (square)Sq;
  }
  else if (Hi)
  {
    asm("bsf %1, %0" : "=r" (Sq) : "rm" (Hi));
    return (square)(Sq + 32);
  }
  else
    return NO_SQUARE;
}
#endif // #ifdef __x86_64__

/******************************************************************************
 * square lastSq(bitboard Bd);
 * PARAMETERS
 *    Bd - The bitboard to search.
 * DESCRIPTION
 *    Determines the square corresponding to the last (most significant) bit
 *    set in Bd. The most significant bit corresponds to the highest numerical
 *    rank, within the highest file alphabetically.
 * RETURN VALUE
 *    Returns the resulting square.
 * PORTABILITY ISSUES
 *    Contains gcc-specific code.
 */
#ifdef __x86_64__
static inline square lastSq(bitboard Bd)
{
  uint64 Sq;  // must be 64-bit for assembly instruction

  if (Bd)
  {
    asm("bsr %1, %0" : "=r" (Sq) : "rm" (Bd));
    return (square)Sq;
  }
  else
    return NO_SQUARE;
}
#else // __x86_64__ not defined
static inline square lastSq(bitboard Bd)
{
  uint32 Hi = Bd >> 32;
  uint32 Lo = Bd & 0xffffffff;
  uint32 Sq;  // must be 32-bit for assembly instruction

  if (Hi)
  {
    asm("bsr %1, %0" : "=r" (Sq) : "rm" (Hi));
    return (square)Sq;
  }
  else if (Lo)
  {
    // NOTE: will only work as written on AMD64 machines and is gcc specific
    asm("bsr %1, %0" : "=r" (Sq) : "rm" (Lo));
    return (square)(Sq + 32);
  }
  else
    return NO_SQUARE;
}
#endif // #ifdef __x86_64__

/******************************************************************************
 * bitboard flipBd(bitboard Bd);
 * PARAMETERS
 *    Bd - board to be flipped.
 * DESCRIPTION
 *    Flips the bitboard along the ranks, so the files are in reverse order.
 * RETURN VALUE
 *    Returns a flipped copy of Bd.
 * PORTABILITY ISSUES
 *    Contains gcc-specific code.
 */
#ifdef __x86_64__
static inline bitboard flipBd(bitboard Bd)
{
  // NOTE: will only work as written on AMD64 machines and is gcc specific
  asm("bswap %0" : "+r" (Bd));

  return Bd;
}
#else // __x86_64__ not defined
static inline bitboard flipBd(bitboard Bd)
{
  uint32 Hi = Bd >> 32;
  uint32 Lo = Bd & 0xffffffff;

  asm("bswap %0" : "+r" (Hi));
  asm("bswap %0" : "+r" (Lo));

  return ((bitboard)Lo << 32) + (bitboard)Hi;
}
#endif // #ifdef __x86_64__

#endif // #ifndef VAPOR__BOARD_H

/* end of file */
