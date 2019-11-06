/******************************************************************************
 * $Id: moves.h 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Handles move generation and move making.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__MOVES_H
#define VAPOR__MOVES_H

#include "vapor.h"
#include "chess.h"

extern const move *MoveStack;
void resetMoveStack(void);
int getMoveStackTop(void);
int popMoveStack(int NewTop);

extern const bitboard PROM_RANKS;

static inline hashmove getHashMove(const move *Move)
{
  if (Move->Orig != NO_SQUARE && Move->Dest != NO_SQUARE)
    return (Move->Orig << 9) | (Move->Dest << 3) | Move->PromPc;
  else
    return 0;
}

int expandMove(const position *Pos, hashmove HashMove, move *Move);

/******************************************************************************
 * int quickMakeMove(position *Pos, move Move);
 * PARAMETERS
 *    Pos - pointer to the position structure on which the move is made.
 *    Move - the move and any additional data.
 * DESCRIPTION
 *    Makes Move on the board pointed to by Pos. Move must be a valid pseudo-
 *    legal move. A move is considered pseudo-legal if it is legal or would be
 *    legal if it did not leave the mover in check. If a move is not full legal
 *    it is still made and the resulting position has the PF_INVALID flag set.
 * RETURN VALUE
 *    Returns zero on success, -1 on failure.
 * !!!CAUTION!!!
 *    The only legality check is to ensure the move does not leave the mover in
 *    check. Results with non-pseudo-legal moves are undefined and could crash
 *    the program. Therefore, only moves returned by the move generation and
 *    verification functions should be passed to this function.
 */
int quickMakeMove(position *Pos, move Move);

/******************************************************************************
 * int genHashMove(const position *Pos, hashmove HashMove);
 * PARAMETERS
 *    Pos - pointer to the position to generate moves for.
 *    HashMove - a hash move.
 * DESCRIPTION
 *    Expand HashMove and push it to the stack.
 * RETURN VALUE
 *    Returns the number of moves generated -- zero for failure, one for
 *    success.
 */
int genHashMove(const position *Pos, hashmove HashMove);

/******************************************************************************
 * int genCaptures(const position *Pos);
 * PARAMETERS
 *    Pos - pointer to the position to generate moves for.
 * DESCRIPTION
 *    Generate promotion and capture moves for the position Pos and push them
 *    to the move stack.
 * RETURN VALUE
 *    Returns the number of moves generated.
 */
int genCaptures(const position *Pos);

/******************************************************************************
 * int genQuietMoves(const position *Pos);
 * PARAMETERS
 *    Pos - pointer to the position to generate moves for.
 * DESCRIPTION
 *    Generate quiet (non-promotion, non-capture) moves for the position Pos
 *    and push them to the move stack.
 * RETURN VALUE
 *    Returns the number of moves generated.
 */
int genQuietMoves(const position *Pos);

/******************************************************************************
 * int genCheckEvasions(const position *Pos);
 * PARAMETERS
 *    Pos - pointer to the position to generate moves for.
 * DESCRIPTION
 *    Generate moves for getting out of check for the position Pos and push
 *    them to the move stack.
 * RETURN VALUE
 *    Returns the number of moves generated.
 */
int genCheckEvasions(const position *Pos);

/******************************************************************************
 * attack tables
 */

extern const bitboard KNIGHT_ATT[NUM_SQUARES];
extern const bitboard KING_ATT[NUM_SQUARES];
extern const bitboard FILE_ATT[NUM_RANKS][64];

#define FILEATTINDEX(Bd, Sq) (((Bd) >> (((Sq)&070)+1)) & 0x3f)

/******************************************************************************
 * bitboard diagonalAtt(bitboard Occ, square Sq);
 * PARAMETERS
 *    Occ - bitboard of the occupied squares.
 *    Sq - the square from which to compute attacks.
 * DESCRIPTION
 *    Generate diagonal sliding attacks (a1 to h8 direction) from Sq when the
 *    squares in Occ are occupied.
 * RETURN VALUE
 *    Returns a bitboard with all squares that can be attacked along the
 *    diagonal of Sq.
 */
static inline bitboard diagonalAtt(bitboard Occ, square Sq)
{
  bitboard MaskedOcc = Occ & (DIAGMASK[Sq] - SQMASK(Sq));
  bitboard Results = MaskedOcc - SQMASK(Sq);
  Results ^= flipBd(flipBd(MaskedOcc) - SQMASK(Sq ^ 070));
  return Results & DIAGMASK[Sq];
}

/******************************************************************************
 * bitboard antidiagAtt(bitboard Occ, square Sq);
 * PARAMETERS
 *    Occ - bitboard of the occupied squares.
 *    Sq - the square from which to compute attacks.
 * DESCRIPTION
 *    Generate antidiagonal sliding attacks (a8 to h1 direction) from Sq when
 *    the squares in Occ are occupied.
 * RETURN VALUE
 *    Returns a bitboard with all squares that can be attacked along the
 *    antidiagonal of Sq.
 */
static inline bitboard antidiagAtt(bitboard Occ, square Sq)
{
  bitboard MaskedOcc = Occ & (ANTIDIAGMASK[Sq] - SQMASK(Sq));
  bitboard Results = MaskedOcc - SQMASK(Sq);
  Results ^= flipBd(flipBd(MaskedOcc) - SQMASK(Sq ^ 070));
  return Results & ANTIDIAGMASK[Sq];
}

/******************************************************************************
 * bitboard rankAtt(bitboard Occ, square Sq);
 * PARAMETERS
 *    Occ - bitboard of the occupied squares.
 *    Sq - the square from which to compute attacks.
 * DESCRIPTION
 *    Generate sliding attacks along the rank from Sq when the squares in Occ
 *    are occupied.
 * RETURN VALUE
 *    Returns a bitboard with all squares that can be attacked along the
 *    rank of Sq.
 */
static inline bitboard rankAtt(bitboard Occ, square Sq)
{
  bitboard MaskedOcc = Occ & (RANKMASK(RANK(Sq)) - SQMASK(Sq));
  bitboard Results = MaskedOcc - SQMASK(Sq);
  Results ^= flipBd(flipBd(MaskedOcc) - SQMASK(Sq ^ 070));
  return Results & RANKMASK(RANK(Sq));
}

/******************************************************************************
 * bitboard fileAtt(bitboard Occ, square Sq);
 * PARAMETERS
 *    Occ - bitboard of the occupied squares.
 *    Sq - the square from which to compute attacks.
 * DESCRIPTION
 *    Generate sliding attacks along the file from Sq when the squares in Occ
 *    are occupied.
 * RETURN VALUE
 *    Returns a bitboard with all squares that can be attacked along the
 *    file of Sq.
 */
static inline bitboard fileAtt(bitboard Occ, square Sq)
{
  return (bitboard)FILE_ATT[Sq & 007][FILEATTINDEX(Occ, Sq)] << (Sq & 070);
}

/******************************************************************************
 * attacked
 * Determines whether Sq is attacked by player Attacker.
 * Returns true if attacked, false otherwise.
 */
/******************************************************************************
 * int attacked(const position *Pos, square Sq, color Attacker);
 * PARAMETERS
 *    Pos - pointer to the position in which we're looking for attacks.
 *    Sq - the square that we're looking for attacks on.
 *    Attacker - the color we're looking for attacks by.
 * DESCRIPTION
 *    Determines whether Sq is attacked by Attacker in *Pos.
 * RETURN VALUE
 *    Returns non-zero if Sq is attacked, or zero if it is not attacked.
 */
static inline int attacked(const position *Pos, square Sq, color Attacker)
{
  static const int PAWN_SHIFT[2] = {7, 9};
  const int KingSide = !Attacker; // king-side element of PAWN_SHIFT array
  bitboard Bd;

  // rook-like attacks
  Bd = fileAtt(Pos->Occ, Sq) | rankAtt(Pos->Occ, Sq);
  if (Bd & (Pos->OccBy[Attacker][ROOK] | Pos->OccBy[Attacker][QUEEN]))
    return 1;
  // bishop-like attacks
  Bd = diagonalAtt(Pos->Occ, Sq) | antidiagAtt(Pos->Occ, Sq);
  if (Bd & (Pos->OccBy[Attacker][BISHOP] | Pos->OccBy[Attacker][QUEEN]))
    return 1;
  // knight attacks
  if (KNIGHT_ATT[Sq] & Pos->OccBy[Attacker][KNIGHT])
    return 1;
  // king attacks
  if (KING_ATT[Sq] & Pos->OccBy[Attacker][KING])
    return 1;
  // pawn attacks
  Bd = (SQMASK(Sq) >> PAWN_SHIFT[!KingSide])
       | (SQMASK(Sq) << PAWN_SHIFT[KingSide]);
  if (Bd & Pos->OccBy[Attacker][PAWN])
    return 1;

  // not attacked
  return 0;
}

#endif // #ifndef VAPOR__MOVES_H

/* end of file */
