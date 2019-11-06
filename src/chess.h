/******************************************************************************
 * $Id: chess.h 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Chess types and data.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__CHESS_H
#define VAPOR__CHESS_H

#include "vapor.h"

/******************************************************************************
 * the chess board
 */
#include "chessboard.h"

/******************************************************************************
 * the pieces
 */

/* color indicates the color of a piece/player */
typedef enum color
{
  BLACK,
  WHITE,
} color;
#define NUM_COLORS (WHITE + 1)

/* piece indicates the type of piece */
typedef enum piece
{
  NO_PIECE,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
} piece;
#define NUM_PIECES (KING)

/******************************************************************************
 * the position
 */

/* zobrist stores a hash key of a position */
typedef uint64 zobrist;

/* position stores the information to uniquely identify a chess position */
typedef struct position
{
  zobrist ZKey;
  bitboard Occ;
  bitboard OccBy[NUM_COLORS][NUM_PIECES + 1];

  square EPSquare;  // set iff the last move was a 2-square pawn move even if
                    // en passant is not a legal move

  uint32 Flags;      // position flags defined below

  int DrawPlies;    // number of plies that count toward a 50 move draw
  int MoveNum;      // full-move number of the next move starting with 1

} position;

/* position flags */
#define PF_WHITEMOVE    0x0001  // it's white's turn
#define PF_EPLEGAL      0x0002  // en passant is a legal move
#define PF_CHECK        0x0008  // the color on move is in check

#define PF_WKSCASTLE    0x0100  // white has king-side castling right
#define PF_WQSCASTLE    0x0200  // white has queen-side castling right
#define PF_BKSCASTLE    0x0400  // black has king-side castling right
#define PF_BQSCASTLE    0x0800  // black has queen-side castling right

#define PF_WCASTLE      0x0300  // all white castling rights flags
#define PF_BCASTLE      0x0c00  // all black castling rights flags
#define PF_CASTLEFLAGS  0x0f00  // all castling rights flags

#define PF_INVALID  0x80000000  // invalid position

/******************************************************************************
 * the move
 */

typedef uint16 hashmove;

/* movetype enumeration */
typedef enum movetype
{
  MT_STANDARD = 0,
  MT_CASTLE,
  MT_ADVANCE2,

  MT_INVALID = -1
} movetype;

/* move stores basic information about a move */ 
typedef struct move
{
  piece Piece;    // moved piece
  piece CaptPc;   // the captured piece (NO_PIECE if not applicable)
  piece PromPc;   // the piece promoted to (NO_PIECE if not applicable)

  square Orig;    // origin square
  square Dest;    // destination square

  movetype Type;  // type of move
} move;

#endif // #ifndef VAPOR__CHESS_H

/* end of file */
