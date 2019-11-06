/******************************************************************************
 * $Id: moves.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Handles move generation and move making.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#include "moves.h"
#include "zobrist.h"

#include <stdlib.h>
#include <string.h>

const bitboard KNIGHT_ATT[NUM_SQUARES];
const bitboard KING_ATT[NUM_SQUARES];
const bitboard FILE_ATT[NUM_RANKS][64];

const bitboard PROM_RANKS = RANKMASK(R_1) | RANKMASK(R_8);


#define MIN_MVSTACK_SIZE  512

static move *MvStack = NULL;
const move *MoveStack = NULL;
int StackSize = 0;
int StackTop  = 0;

void resetMoveStack(void)
{
  if (!MvStack)
  {
    StackSize = MIN_MVSTACK_SIZE;
    MvStack = malloc(sizeof(move[StackSize]));
    MoveStack = MvStack;
  }
  assert(MvStack);

  StackTop = 0;
}

int getMoveStackTop(void)
{
  if (!MvStack)
    resetMoveStack();
  return StackTop;
}

int popMoveStack(int NewTop)
{
  if (NewTop < 0)
    StackTop = 0;
  else if (NewTop < StackTop)
    StackTop = NewTop;

  return StackTop;
}

static inline int pushMove(const move *Move)
{
  if (!MvStack)
    resetMoveStack();
  if (StackTop == StackSize)
  {
    StackSize *= 2;
    MvStack = realloc(MvStack, sizeof(move[StackSize]));
    MoveStack = MvStack;
    assert(MvStack);
  }

  MvStack[StackTop] = *Move;

  return StackTop++;
}

int expandMove(const position *Pos, hashmove HashMove, move *Move)
{
  static const int PAWN_SHIFT[2] = {7, 9};
  const color Mover = Pos->Flags & PF_WHITEMOVE;
  const int KingSide = Mover; // king-side element of PAWN_SHIFT array
  piece p;
  square ROrig = NO_SQUARE;
  square RDest = NO_SQUARE;
  bitboard Occ = Pos->Occ;
  bitboard OrigBd;
  bitboard DestBd;

  memset(Move, 0, sizeof(move));
  Move->Type = MT_INVALID;

  if (!HashMove)
    return -1;

  Move->PromPc = HashMove & 7;
  Move->Dest = (HashMove >> 3) & 077;
  DestBd = SQMASK(Move->Dest);
  Move->Orig = HashMove >> 9;
  OrigBd = SQMASK(Move->Orig);

  // verify that mover has piece at Orig
  if (!(Pos->OccBy[Mover][0] & OrigBd))
    return -1;
  // verify that mover does not have piece at Dest
  if (Pos->OccBy[Mover][0] & DestBd)
    return -1;

  // find the moved piece
  for (p = PAWN; p <= KING; p++)
  {
    if (Pos->OccBy[Mover][p] & OrigBd)
    {
      Move->Piece = p;
      break;
    }
  }
  if (Move->Piece == NO_PIECE)
    return -1;

  // find the captured piece if any
  if (Pos->OccBy[!Mover][0] & DestBd)
  {
    for (p = PAWN; p <= KING; p++)
    {
      if (Pos->OccBy[!Mover][p] & DestBd)
      {
        Move->CaptPc = p;
        break;
      }
    }
  }
  else if (Move->Dest == Pos->EPSquare && Move->Piece == PAWN)
    Move->CaptPc = PAWN;

  // determine the move type
  if (Move->CaptPc == NO_PIECE && Move->PromPc == NO_PIECE)
  {
    if (Move->Piece == PAWN)
    {
      // verify pawn doesn't jump over a piece
      // NOTE: (harmless for single-space pawn moves or captures)
      if (!(fileAtt(Occ, Move->Orig) & DestBd))
        return -1;

      if (Mover == BLACK && Move->Orig - Move->Dest == 2)
        Move->Type = MT_ADVANCE2;
      else if (Mover == WHITE && Move->Dest - Move->Orig == 2)
        Move->Type = MT_ADVANCE2;
    }
    else if (Move->Piece == KING)
    {
      // determine if it can be a castling move
      switch (Move->Dest)
      {
        case g1:
          if (Move->Orig == e1 && Mover == WHITE && 
              (Pos->Flags & PF_WKSCASTLE))
          {
            ROrig = h1;
            RDest = f1;
          }
          break;
        case c1:
          if (Move->Orig == e1 && Mover == WHITE && 
              (Pos->Flags & PF_WQSCASTLE))
          {
            ROrig = a1;
            RDest = d1;
          }
          break;
        case g8:
          if (Move->Orig == e8 && Mover == BLACK && 
              (Pos->Flags & PF_BKSCASTLE))
          {
            ROrig = h8;
            RDest = f8;
          }
          break;
        case c8:
          if (Move->Orig == e8 && Mover == BLACK && 
              (Pos->Flags & PF_BQSCASTLE))
          {
            ROrig = a8;
            RDest = d8;
          }
          break;
        default:
          break;
      }
      if (RDest != NO_SQUARE)
      {
        // verify that mover has rook at ROrig
        if (!(Pos->OccBy[Mover][ROOK] & SQMASK(ROrig)))
          return -1;
        // verify that there is no piece between king and rook
        if (!(rankAtt(Occ, ROrig) & OrigBd))
          return -1;

        Move->Type = MT_CASTLE;
      }
    }
  }
  if (Move->Type == MT_INVALID) {
    switch (Move->Piece) {
      case PAWN:
        // make sure it's a promotion if dest is last rank
        if (Move->PromPc == NO_PIECE && (RANK(Move->Dest) == R_8 || RANK(Move->Dest) == R_1))
          return -1;

        if (Move->CaptPc) {
          if (Move->Orig == Move->Dest + PAWN_SHIFT[!KingSide]
                || Move->Orig == Move->Dest - PAWN_SHIFT[KingSide])
          {
            Move->Type = MT_STANDARD;
          }
        } else {
          if (Mover == BLACK && Move->Orig - Move->Dest == 1)
            Move->Type = MT_STANDARD;
          else if (Mover == WHITE && Move->Dest - Move->Orig == 1)
            Move->Type = MT_STANDARD;
        }
        break;
      case KNIGHT:
        if (KNIGHT_ATT[Move->Orig] & DestBd)
          Move->Type = MT_STANDARD;
        break;
      case BISHOP:
        if ((diagonalAtt(Occ, Move->Orig) | antidiagAtt(Occ, Move->Orig)) & DestBd)
          Move->Type = MT_STANDARD;
        break;
      case ROOK:
        if ((rankAtt(Occ, Move->Orig) | fileAtt(Occ, Move->Orig)) & DestBd)
          Move->Type = MT_STANDARD;
        break;
      case QUEEN:
        if ((diagonalAtt(Occ, Move->Orig) | antidiagAtt(Occ, Move->Orig)
              | rankAtt(Occ, Move->Orig) | fileAtt(Occ, Move->Orig)) & DestBd)
          Move->Type = MT_STANDARD;
        break;
      case KING:
        if (KING_ATT[Move->Orig] & DestBd)
          Move->Type = MT_STANDARD;
        break;
      default:
        assert(0); // should never happen
    }
  }
  if (Move->Type == MT_INVALID)
    return -1;

  return 0;
}

/******************************************************************************
 * int quickMakeMove(position *Pos, move Move);
 * PARAMETERS
 *    Pos - pointer to the position structure on which the move is made.
 *    Move - the move and any additional data.
 * DESCRIPTION
 *    Makes Move on the board pointed to by Pos. Move must be a valid pseudo-
 *    legal move. A move is considered pseudo-legal if it is legal or would be
 *    legal if it did not leave the mover in check. If a move is not fully
 *    legal it is still made and the resulting position has the PF_INVALID flag
 *    set.
 * RETURN VALUE
 *    Returns zero on success, -1 on failure.
 * !!!CAUTION!!!
 *    The only legality check is to ensure the move does not leave the mover in
 *    check. Results with non-pseudo-legal moves are undefined and could crash
 *    the program. Therefore, only moves returned by the move generation and
 *    verification functions should be passed to this function.
 */
int quickMakeMove(position *Pos, move Move)
{
  const color Mover = (Pos->Flags & PF_WHITEMOVE)? WHITE : BLACK;
  register bitboard Mask;
  square Sq;

  if (Move.Type == MT_INVALID)
    return -1;

  /* switch mover */
  Pos->Flags ^= PF_WHITEMOVE;
  Pos->ZKey ^= Z_WHITEMOVE;

  /* move counts */
  if (Mover == BLACK)
    Pos->MoveNum++;
  if (Move.Piece != PAWN && Move.CaptPc == NO_PIECE)
    Pos->DrawPlies++;
  else
    Pos->DrawPlies = 0;

  /* clear captured piece if any */
  if (Move.CaptPc != NO_PIECE)
  {
    if (Move.Dest != Pos->EPSquare)
    {
      Mask = ~SQMASK(Move.Dest);
      Pos->ZKey ^= Z_PLACEMENT[!Mover][Move.CaptPc][Move.Dest];
    }
    else // en passant
    {
      Sq = SQUARE(FILE(Move.Dest), RANK(Move.Orig));
      Mask = ~SQMASK(Sq);
      Pos->ZKey ^= Z_PLACEMENT[!Mover][Move.CaptPc][Sq];
    }
    Pos->Occ &= Mask;
    Pos->OccBy[!Mover][0] &= Mask;
    Pos->OccBy[!Mover][Move.CaptPc] &= Mask;

    // castling flags
    if (Pos->Flags & PF_CASTLEFLAGS)
    {
      Pos->ZKey ^= Z_CASTLE[(Pos->Flags & PF_CASTLEFLAGS) >> 8];
      switch (Move.Dest)
      {
        case h1:
          Pos->Flags &= ~PF_WKSCASTLE;
          break;
        case a1:
          Pos->Flags &= ~PF_WQSCASTLE;
          break;
        case h8:
          Pos->Flags &= ~PF_BKSCASTLE;
          break;
        case a8:
          Pos->Flags &= ~PF_BQSCASTLE;
          break;
        default:
          break;
      }
      Pos->ZKey ^= Z_CASTLE[(Pos->Flags & PF_CASTLEFLAGS) >> 8];
    }
  }

  /* move piece to new location */
  Mask = SQMASK(Move.Orig) | SQMASK(Move.Dest);
  Pos->Occ ^= Mask;
  Pos->OccBy[Mover][0] ^= Mask;
  Pos->OccBy[Mover][Move.Piece] ^= Mask;
  Pos->ZKey ^= Z_PLACEMENT[Mover][Move.Piece][Move.Orig];
  Pos->ZKey ^= Z_PLACEMENT[Mover][Move.Piece][Move.Dest];

  /* castling */
  if (Move.Type == MT_CASTLE)
  {
    if (Move.Dest > Move.Orig)
    {
      Sq = (Mover == WHITE)?h1:h8;
      Mask = SQMASK(Move.Orig + 8) | SQMASK(Sq);
      Pos->ZKey ^= Z_PLACEMENT[Mover][ROOK][Move.Orig+8];
      Pos->ZKey ^= Z_PLACEMENT[Mover][ROOK][Sq];
    }
    else
    {
      Sq = (Mover == WHITE)?a1:a8;
      Mask = SQMASK(Move.Orig - 8) | SQMASK(Sq);
      Pos->ZKey ^= Z_PLACEMENT[Mover][ROOK][Move.Orig-8];
      Pos->ZKey ^= Z_PLACEMENT[Mover][ROOK][Sq];
    }
    Pos->Occ ^= Mask;
    Pos->OccBy[Mover][0] ^= Mask;
    Pos->OccBy[Mover][ROOK] ^= Mask;
  }

  /* promotion */
  else if (Move.PromPc != NO_PIECE)
  {
    Pos->OccBy[Mover][Move.Piece] ^= SQMASK(Move.Dest);
    Pos->ZKey ^= Z_PLACEMENT[Mover][Move.Piece][Move.Dest];
    Pos->OccBy[Mover][Move.PromPc] ^= SQMASK(Move.Dest);
    Pos->ZKey ^= Z_PLACEMENT[Mover][Move.PromPc][Move.Dest];
  }

  /* en passant square */
  if (Pos->EPSquare != NO_SQUARE) {
    // clear old data
    Pos->ZKey ^= Z_EPSQ[FILE(Pos->EPSquare)];
    Pos->EPSquare = NO_SQUARE;
    Pos->Flags &= ~PF_EPLEGAL;
  }
  if (Move.Type == MT_ADVANCE2)
  {
    Pos->EPSquare = (Move.Orig + Move.Dest)/2;
    // TODO: determine if there is a capture pawn?
    Pos->Flags |= PF_EPLEGAL;
    Pos->ZKey ^= Z_EPSQ[FILE(Pos->EPSquare)];
  }

  /* castling flags */
  if (Pos->Flags & PF_CASTLEFLAGS)
  {
    Pos->ZKey ^= Z_CASTLE[(Pos->Flags & PF_CASTLEFLAGS) >> 8];
    switch (Move.Orig)
    {
      case e1:
        Pos->Flags &= ~PF_WCASTLE;
        break;
      case h1:
        Pos->Flags &= ~PF_WKSCASTLE;
        break;
      case a1:
        Pos->Flags &= ~PF_WQSCASTLE;
        break;
      case e8:
        Pos->Flags &= ~PF_BCASTLE;
        break;
      case h8:
        Pos->Flags &= ~PF_BKSCASTLE;
        break;
      case a8:
        Pos->Flags &= ~PF_BQSCASTLE;
        break;
      default:
        break;
    }
    Pos->ZKey ^= Z_CASTLE[(Pos->Flags & PF_CASTLEFLAGS) >> 8];
  }

  assert(Pos->ZKey == calcZobrist(Pos));

  /* determine if opponent is now in check */
  // TODO: only consider discovered attacks and attacks by a moved piece
  //       (including the moved rook in a castling move)
  if (attacked(Pos, firstSq(Pos->OccBy[!Mover][KING]), Mover))
    Pos->Flags |= PF_CHECK;
  else
    Pos->Flags &= ~PF_CHECK;

  /* determine if move puts mover in check */
  // TODO: only consider discovered attacks non-king moves
  if (attacked(Pos, firstSq(Pos->OccBy[Mover][KING]), !Mover))
  {
    Pos->Flags |= PF_INVALID;
    return -1;
  }

  return 0;
}

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
int genHashMove(const position *Pos, hashmove HashMove)
{
  move Move;

  if (expandMove(Pos, HashMove, &Move) != 0) {
    return 0;
  } else {
    pushMove(&Move);
    return 1;
  }
}

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
int genCaptures(const position *Pos)
{
  static const int PAWN_SHIFT[2] = {7, 9};
  const color Mover = (Pos->Flags & PF_WHITEMOVE)? WHITE : BLACK;
  const int KingSide = Mover; // king-side element of PAWN_SHIFT array
  const bitboard Occ = Pos->Occ;
  const int StackBase = StackTop;

  bitboard Targets;
  move Move;
  bitboard Pieces;
  bitboard MvBd;
  bitboard PAttQS, PAttKS;  // for pawn captures toward queen and king sides
  int nMoves = 0;
  int i;

  if (Pos->Flags & PF_INVALID)
    return 0;

  memset(&Move, 0, sizeof(move));

  /* pawn attacks */
  Move.Piece = PAWN;
  Pieces = Pos->OccBy[Mover][PAWN];
  PAttQS = (Pieces >> PAWN_SHIFT[!KingSide]);
  PAttKS = (Pieces << PAWN_SHIFT[KingSide]);

  /* capture promotions in MVV/LVA order */
  Move.PromPc = QUEEN;
  for (Move.CaptPc = QUEEN; Move.CaptPc > PAWN; Move.CaptPc--)
  {
    Targets = PROM_RANKS & Pos->OccBy[!Mover][Move.CaptPc];
    if (!Targets)
      continue;

    // toward queen side
    MvBd = PAttQS & Targets;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Dest = firstSq(MvBd);
      Move.Orig = Move.Dest + PAWN_SHIFT[!KingSide];
      pushMove(&Move);
      nMoves++;
    }

    // toward king side
    MvBd = PAttKS & Targets;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Dest = firstSq(MvBd);
      Move.Orig = Move.Dest - PAWN_SHIFT[KingSide];
      pushMove(&Move);
      nMoves++;
    }
  }

  /* non-capture promotions */
  Move.CaptPc = NO_PIECE;
  if (Mover == WHITE)
  {
    MvBd = (Pieces << 1) & PROM_RANKS & ~Occ;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Dest = firstSq(MvBd);
      Move.Orig = Move.Dest - 1;
      pushMove(&Move);
      nMoves++;
    }
  }
  else // Mover is BLACK
  {
    MvBd = (Pieces >> 1) & PROM_RANKS & ~Occ;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Dest = firstSq(MvBd);
      Move.Orig = Move.Dest + 1;
      pushMove(&Move);
      nMoves++;
    }
  }

  /* non-promotion captures in MVV/LVA order */
  Move.PromPc = NO_PIECE;
  for (Move.CaptPc = QUEEN; Move.CaptPc > NO_PIECE; Move.CaptPc--)
  {
    Targets = Pos->OccBy[!Mover][Move.CaptPc];
    if (!Targets)
      continue;

    /* pawn moves */
    // toward queen side
    MvBd = PAttQS & ~PROM_RANKS & Targets;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Piece = PAWN;
      Move.Dest = firstSq(MvBd);
      Move.Orig = Move.Dest + PAWN_SHIFT[!KingSide];
      pushMove(&Move);
      nMoves++;
    }
  
    // toward king side
    MvBd = PAttKS & ~PROM_RANKS & Targets;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Piece = PAWN;
      Move.Dest = firstSq(MvBd);
      Move.Orig = Move.Dest - PAWN_SHIFT[KingSide];
      pushMove(&Move);
      nMoves++;
    }
  
    /* knight moves */
    Pieces = Pos->OccBy[Mover][KNIGHT];
    if (Pieces)
    {
      Move.Piece = KNIGHT;
      do {
        Move.Orig = firstSq(Pieces);
        MvBd = KNIGHT_ATT[Move.Orig] & Targets;
        for (; MvBd; CLEARLSB(MvBd))
        {
          Move.Dest = firstSq(MvBd);
          pushMove(&Move);
          nMoves++;
        }
        CLEARLSB(Pieces);
      } while (Pieces);
    }
  
    /* bishop moves */
    Pieces = Pos->OccBy[Mover][BISHOP];
    if (Pieces)
    {
      Move.Piece = BISHOP;
      do {
        Move.Orig = firstSq(Pieces);
        MvBd = (diagonalAtt(Occ, Move.Orig)
            | antidiagAtt(Occ, Move.Orig))
            & Targets;
        for (; MvBd; CLEARLSB(MvBd))
        {
          Move.Dest = firstSq(MvBd);
          pushMove(&Move);
          nMoves++;
        }
        CLEARLSB(Pieces);
      } while (Pieces);
    }
  
    /* rook moves */
    Pieces = Pos->OccBy[Mover][ROOK];
    if (Pieces)
    {
      Move.Piece = ROOK;
      do {
        Move.Orig = firstSq(Pieces);
        MvBd = (rankAtt(Occ, Move.Orig)
            | fileAtt(Occ, Move.Orig))
            & Targets;
        for (; MvBd; CLEARLSB(MvBd))
        {
          Move.Dest = firstSq(MvBd);
          pushMove(&Move);
          nMoves++;
        }
        CLEARLSB(Pieces);
      } while (Pieces);
    }
  
    /* queen moves */
    Pieces = Pos->OccBy[Mover][QUEEN];
    if (Pieces)
    {
      Move.Piece = QUEEN;
      do {
        Move.Orig = firstSq(Pieces);
        MvBd = (diagonalAtt(Occ, Move.Orig)
            | antidiagAtt(Occ, Move.Orig)
            | rankAtt(Occ, Move.Orig)
            | fileAtt(Occ, Move.Orig))
            & Targets;
        for (; MvBd; CLEARLSB(MvBd))
        {
          Move.Dest = firstSq(MvBd);
          pushMove(&Move);
          nMoves++;
        }
        CLEARLSB(Pieces);
      } while (Pieces);
    }
  
    /* king moves */
    Move.Piece = KING;
    // since there is exactly one king per side, we don't need a loop
    Move.Orig = firstSq(Pos->OccBy[Mover][KING]);
    MvBd = KING_ATT[Move.Orig] & Targets;
    for (; MvBd; CLEARLSB(MvBd))
    {
      Move.Dest = firstSq(MvBd);
      pushMove(&Move);
      nMoves++;
    }
  }

  /* en passant */
  if ((Pos->Flags & PF_EPLEGAL) && Pos->EPSquare != NO_SQUARE)
  {
    // toward queen side
    if (PAttQS & SQMASK(Pos->EPSquare))
    {
      Move.Piece = PAWN;
      Move.CaptPc = PAWN;
      Move.Dest = Pos->EPSquare;
      Move.Orig = Move.Dest + PAWN_SHIFT[!KingSide];
      pushMove(&Move);
      nMoves++;
    }
  
    // toward king side
    if (PAttKS & SQMASK(Pos->EPSquare))
    {
      Move.Piece = PAWN;
      Move.CaptPc = PAWN;
      Move.Dest = Pos->EPSquare;
      Move.Orig = Move.Dest - PAWN_SHIFT[KingSide];
      pushMove(&Move);
      nMoves++;
    }
  }

  /* under promotions */
  for (i = StackBase; i < StackBase+nMoves && MvStack[i].PromPc == QUEEN; i++)
  {
    Move = MvStack[i];
    Move.PromPc = KNIGHT;
    pushMove(&Move);
    Move.PromPc = ROOK;
    pushMove(&Move);
    Move.PromPc = BISHOP;
    pushMove(&Move);
    nMoves += 3;
  }

  return nMoves;
}

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
int genQuietMoves(const position *Pos)
{
  const color Mover = (Pos->Flags & PF_WHITEMOVE)? WHITE : BLACK;
  const bitboard Occ = Pos->Occ;
  const bitboard Targets = ~Occ;
  move Move;
  int nMoves = 0;
  bitboard Pieces;
  bitboard MvBd, MvBd2;

  if (Pos->Flags & PF_INVALID)
    return 0;

  memset(&Move, 0, sizeof(move));

  /* castling */
  if (Mover == WHITE && (Pos->Flags & PF_WCASTLE)
      && !(Pos->Flags & PF_CHECK))
  {
    Pieces = rankAtt(Occ, e1) & Pos->OccBy[Mover][ROOK];
    if ((Pieces & SQMASK(h1)) && (Pos->Flags & PF_WKSCASTLE)
        && !attacked(Pos, f1, !Mover))
    {
      Move.Type = MT_CASTLE;
      Move.Piece = KING;
      Move.Orig = e1;
      Move.Dest = g1;
      pushMove(&Move);
      nMoves++;
      Move.Type = MT_STANDARD;
    }
    if ((Pieces & SQMASK(a1)) && (Pos->Flags & PF_WQSCASTLE)
        && !attacked(Pos, d1, !Mover))
    {
      Move.Type = MT_CASTLE;
      Move.Piece = KING;
      Move.Orig = e1;
      Move.Dest = c1;
      pushMove(&Move);
      nMoves++;
      Move.Type = MT_STANDARD;
    }
  }
  else if (Mover == BLACK && (Pos->Flags & PF_BCASTLE)
      && !(Pos->Flags & PF_CHECK))
  {
    Pieces = rankAtt(Occ, e8) & Pos->OccBy[Mover][ROOK];
    if ((Pieces & SQMASK(h8)) && (Pos->Flags & PF_BKSCASTLE)
        && !attacked(Pos, f8, !Mover))
    {
      Move.Type = MT_CASTLE;
      Move.Piece = KING;
      Move.Orig = e8;
      Move.Dest = g8;
      pushMove(&Move);
      nMoves++;
      Move.Type = MT_STANDARD;
    }
    if ((Pieces & SQMASK(a8)) && (Pos->Flags & PF_BQSCASTLE)
        && !attacked(Pos, d8, !Mover))
    {
      Move.Type = MT_CASTLE;
      Move.Piece = KING;
      Move.Orig = e8;
      Move.Dest = c8;
      pushMove(&Move);
      nMoves++;
      Move.Type = MT_STANDARD;
    }
  }

  /* pawn advancement */
  Pieces = Pos->OccBy[Mover][PAWN];
  if (Pieces)
  {
    Move.Piece = PAWN;
    Move.Type = MT_ADVANCE2;

    if (Mover == WHITE)
    {
      MvBd = (Pieces << 1) & ~PROM_RANKS & Targets;   // one-square
      MvBd2 = (MvBd << 1) & RANKMASK(R_4) & Targets;  // two-square

      // two-square advances
      for (; MvBd2; CLEARLSB(MvBd2))
      {
        Move.Dest = firstSq(MvBd2);
        Move.Orig = Move.Dest - 2;
        pushMove(&Move);
        nMoves++;
      }

      // one-square advances
      Move.Type = MT_STANDARD;
      for (; MvBd; CLEARLSB(MvBd))
      {
        Move.Dest = firstSq(MvBd);
        Move.Orig = Move.Dest - 1;
        pushMove(&Move);
        nMoves++;
      }
    }
    else // Mover is BLACK
    {
      MvBd = (Pieces >> 1) & ~PROM_RANKS & Targets;   // one-square
      MvBd2 = (MvBd >> 1) & RANKMASK(R_5) & Targets;  // two-square

      // one-square advances
      for (; MvBd2; CLEARLSB(MvBd2))
      {
        Move.Dest = firstSq(MvBd2);
        Move.Orig = Move.Dest + 2;
        pushMove(&Move);
        nMoves++;
      }

      // two-square advances
      Move.Type = MT_STANDARD;
      for (; MvBd; CLEARLSB(MvBd))
      {
        Move.Dest = firstSq(MvBd);
        Move.Orig = Move.Dest + 1;
        pushMove(&Move);
        nMoves++;
      }
    }
  }

  /* knight moves */
  Pieces = Pos->OccBy[Mover][KNIGHT];
  if (Pieces)
  {
    Move.Piece = KNIGHT;
    do {
      Move.Orig = firstSq(Pieces);
      MvBd = KNIGHT_ATT[Move.Orig] & Targets;
      for (; MvBd; CLEARLSB(MvBd))
      {
        Move.Dest = firstSq(MvBd);
        pushMove(&Move);
        nMoves++;
      }
      CLEARLSB(Pieces);
    } while (Pieces);
  }

  /* bishop moves */
  Pieces = Pos->OccBy[Mover][BISHOP];
  if (Pieces)
  {
    Move.Piece = BISHOP;
    do {
      Move.Orig = firstSq(Pieces);
      MvBd = (diagonalAtt(Occ, Move.Orig)
          | antidiagAtt(Occ, Move.Orig))
          & Targets;
      for (; MvBd; CLEARLSB(MvBd))
      {
        Move.Dest = firstSq(MvBd);
        pushMove(&Move);
        nMoves++;
      }
      CLEARLSB(Pieces);
    } while (Pieces);
  }

  /* rook moves */
  Pieces = Pos->OccBy[Mover][ROOK];
  if (Pieces)
  {
    Move.Piece = ROOK;
    do {
      Move.Orig = firstSq(Pieces);
      MvBd = (rankAtt(Occ, Move.Orig)
          | fileAtt(Occ, Move.Orig))
          & Targets;
      for (; MvBd; CLEARLSB(MvBd))
      {
        Move.Dest = firstSq(MvBd);
        pushMove(&Move);
        nMoves++;
      }
      CLEARLSB(Pieces);
    } while (Pieces);
  }

  /* queen moves */
  Pieces = Pos->OccBy[Mover][QUEEN];
  if (Pieces)
  {
    Move.Piece = QUEEN;
    do {
      Move.Orig = firstSq(Pieces);
      MvBd = (diagonalAtt(Occ, Move.Orig)
          | antidiagAtt(Occ, Move.Orig)
          | rankAtt(Occ, Move.Orig)
          | fileAtt(Occ, Move.Orig))
          & Targets;
      for (; MvBd; CLEARLSB(MvBd))
      {
        Move.Dest = firstSq(MvBd);
        pushMove(&Move);
        nMoves++;
      }
      CLEARLSB(Pieces);
    } while (Pieces);
  }

  /* king moves */
  Move.Piece = KING;
  // since there is exactly one king per side, we don't need a loop
  Move.Orig = firstSq(Pos->OccBy[Mover][KING]);
  MvBd = KING_ATT[Move.Orig] & Targets;
  for (; MvBd; CLEARLSB(MvBd))
  {
    Move.Dest = firstSq(MvBd);
    pushMove(&Move);
    nMoves++;
  }

  return nMoves;
}

/* end of file */
