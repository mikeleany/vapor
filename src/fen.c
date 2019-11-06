/******************************************************************************
 * $Id: fen.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Convert to and from FEN strings.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#include "fen.h"
#include "notation.h"
#include "zobrist.h"
#include "moves.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FIELDS  6
#define MIN_FIELDS  4

const char *const STARTPOS =
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/******************************************************************************
 * int parseBoard(position *Pos, const char *BoardStr);
 * PARAMETERS
 *    Pos - pointer to the position structure in which to store the resulting
 *        bitboards data.
 *    BoardStr - the piece placement data field from a FEN string.
 * DESCRIPTION
 *    Reads and parses the piece placement data at BoardStr to fill the
 *    bitboards in *Pos.
 * RETURN VALUE
 *    Returns 0 if the string represents valid piece placement data, or -1 for
 *    invalid data or if the function fails, in which case the data in *Pos is
 *    undefined.
 */
static int parseBoard(position *Pos, const char *BoardStr);

/******************************************************************************
 * int importFEN(position *Pos, const char *FENStr);
 * PARAMETERS
 *    Pos - pointer to the position structure into which FENStr is imported.
 *    FENStr - the position in Forsyth–Edwards Notation.
 * DESCRIPTION
 *    Import the FEN position pointed to by FENStr into *Pos.
 * RETURN VALUE
 *    Returns 0 if the string represents a valid position, or -1 for an invalid
 *    FEN position or if the function fails, in which case the data in *Pos is
 *    undefined.
 */
int importFEN(position *Pos, const char *FENStr)
{
  char Buffer[128];
  const char *Field[MAX_FIELDS] = { NULL };
  int i;
  color Active;

  if (!Pos || !FENStr)
    return -1;

  // clear the position and set the invalid flag until we verify validity
  memset(Pos, 0, sizeof(position));
  Pos->Flags = PF_INVALID;

  // copy FENStr to Buffer for use by strtok()
  strncpy(Buffer, FENStr, 127);
  Buffer[127] = 0;

  // use strtok() to separate the fields
  for (i = 0; i < MAX_FIELDS; i++)
  {
    Field[i] = strtok((i==0)?Buffer:NULL, " ");
    if (!Field[i])
    {
      if (i < MIN_FIELDS)
      {
        // not enough fields present to parse string
        return -1;
      }
      break;
    }
  }

  /* piece placement data (Field[0]) */
  if (parseBoard(Pos, Field[0]) == -1)
    return -1;

  /* active color (Field[1]) */
  if (strcmp(Field[1], "w") == 0)
  {
    Pos->Flags |= PF_WHITEMOVE;
    Active = WHITE;
  }
  else if (strcmp(Field[1], "b") == 0)
  {
    Active = BLACK;
  }
  else // invalid active color
    return -1;

  /* castling availability (Field[2])*/
  if (strcmp(Field[2], "-") != 0)
  {
    i = 0;
    if (Field[2][i] == 'K')
    {
      Pos->Flags |= PF_WKSCASTLE;
      i++;
    }
    if (Field[2][i] == 'Q')
    {
      Pos->Flags |= PF_WQSCASTLE;
      i++;
    }
    if (Field[2][i] == 'k')
    {
      Pos->Flags |= PF_BKSCASTLE;
      i++;
    }
    if (Field[2][i] == 'q')
    {
      Pos->Flags |= PF_BQSCASTLE;
      i++;
    }

    if (Field[2][i] != 0)
    {
      // castling field is messed up
      return -1;
    }
  }

  /* en passant target square (Field[3]) */
  if (strcmp(Field[3], "-") == 0)
    Pos->EPSquare = NO_SQUARE;
  else
  {
    Pos->EPSquare = strToSquare(Field[3]);
    if (strlen(Field[3]) > 2 || Pos->EPSquare == NO_SQUARE)
      return -1;
    // TODO: determine if en passant is legal
    Pos->Flags |= PF_EPLEGAL;
  }

  /* halfmove clock (Field[4]) and fullmove number (Field[5]) */
  if (Field[4])
    Pos->DrawPlies = atoi(Field[4]);
  if (Pos->DrawPlies < 0)
    Pos->DrawPlies = 0;

  if (Field[5])
    Pos->MoveNum = atoi(Field[5]);
  if (Pos->MoveNum < 1)
    Pos->MoveNum = 1;

  /* calculate the zobrist key */
  Pos->ZKey = calcZobrist(Pos);

  /* determine if active color is in check */
  if (attacked(Pos, firstSq(Pos->OccBy[Active][KING]), !Active))
    Pos->Flags |= PF_CHECK;

  /* determine if the position is legal */
  if (!isPosLegal(Pos))
    return -1;

  /* mark the position as valid */
  Pos->Flags &= ~PF_INVALID;

  return 0;
}

/******************************************************************************
 * int parseBoard(position *Pos, const char *BoardStr);
 * PARAMETERS
 *    Pos - pointer to the position structure in which to store the resulting
 *        bitboards data.
 *    BoardStr - the piece placement data field from a FEN string.
 * DESCRIPTION
 *    Reads and parses the piece placement data at BoardStr to fill the
 *    bitboards in *Pos.
 * RETURN VALUE
 *    Returns 0 if the string represents valid piece placement data, or -1 for
 *    invalid data or if the function fails, in which case the data in *Pos is
 *    undefined.
 */
static int parseBoard(position *Pos, const char *BoardStr)
{
  int i;
  file f = F_a;
  rank r = R_8;
  color c;
  piece p;

  for (i = 0; BoardStr[i]; i++)
  {
    if (isalpha(BoardStr[i]))
    {
      if (f >= NUM_FILES)
        return -1; // more than 8 squares in rank
      
      if (isupper(BoardStr[i]))
        c = WHITE;
      else
        c = BLACK;
      p = charToPiece(BoardStr[i]);
      if (!p)
        return -1;

      Pos->Occ |= SQMASKFR(f, r);
      Pos->OccBy[c][0] |= SQMASKFR(f, r);
      Pos->OccBy[c][p] |= SQMASKFR(f, r);
      f++;
    }
    else if (BoardStr[i] > '0' && BoardStr[i] <= '9')
    {
      f += BoardStr[i] - '0';
      if (f > NUM_FILES)
        return -1; // more than 8 squares in rank
    }
    else if (BoardStr[i] == '/')
    {
      if (f == NUM_FILES && r > R_1)
      {
        f = F_a;
        r--;
      }
      else
        return -1; // rank too short or too many ranks
    }
    else
      return -1; // invalid character
  }
  if (f != NUM_FILES || r != R_1)
    return -1; // missing part of the board

  return 0;
}

/******************************************************************************
 * char *exportFEN(char *FENStr, const position *Pos);
 * PARAMETERS
 *    FENStr - memory location into which the FEN string is written. This must
 *        be capable of holding the entire FEN string, including the
 *        terminating null character. 100 characters should be sufficient for
 *        the largest possible FEN string.
 *    Pos - pointer to the position structure from which the position is
 *        exported.
 * DESCRIPTION
 *    Export the position in *Pos into the buffer pointed to by FENStr.
 * RETURN VALUE
 *    Returns FENStr, or NULL if *Pos has the PF_INVALID flag set, in which
 *    case the buffer pointed to by FENStr is unmodified.
 */
char *exportFEN(char *FENStr, const position *Pos)
{
  int i = 0;
  int n;
  file f;
  rank r;
  color c;
  piece p;

  if (!FENStr || !Pos)
    return NULL;
  if (Pos->Flags & PF_INVALID)
    return NULL;

  /* piece placement data */
  for (r = R_8; r >= R_1; r--)
  {
    n = 0;
    
    for (f = F_a; f < NUM_FILES; f++)
    {
      if (!(Pos->Occ & SQMASKFR(f, r)))
      {
        n++;
        continue;
      }

      if (n > 0)
      {
        FENStr[i++] = n + '0';
        n = 0;
      }

      if (Pos->OccBy[WHITE][0] & SQMASKFR(f, r))
        c = WHITE;
      else
        c = BLACK;

      for (p = PAWN; p <= KING; p++)
      {
        if (Pos->OccBy[c][p] & SQMASKFR(f, r))
        {
          if (c == WHITE)
            FENStr[i++] = PIECE_CHAR[p];
          else
            FENStr[i++] = tolower(PIECE_CHAR[p]);
          break;
        }
      }
      
      // assert that we have found the piece
      assert(p <= KING);
    }
    // reached end of rank
    
    if (n > 0)
    {
      FENStr[i++] = n + '0';
      n = 0;
    }

    if (r != R_1)
      FENStr[i++] = '/';
  }

  FENStr[i++] = ' ';

  /* active color */
  if (Pos->Flags & PF_WHITEMOVE)
    FENStr[i++] = 'w';
  else
    FENStr[i++] = 'b';

  FENStr[i++] = ' ';

  /* castling availability */
  if (Pos->Flags & PF_CASTLEFLAGS)
  {
    if (Pos->Flags & PF_WKSCASTLE)
      FENStr[i++] = 'K';
    if (Pos->Flags & PF_WQSCASTLE)
      FENStr[i++] = 'Q';
    if (Pos->Flags & PF_BKSCASTLE)
      FENStr[i++] = 'k';
    if (Pos->Flags & PF_BQSCASTLE)
      FENStr[i++] = 'q';
  }
  else
    FENStr[i++] = '-';

  FENStr[i++] = ' ';

  /* en passant */
  if (Pos->EPSquare != NO_SQUARE)
  {
    FENStr[i++] = SQUARE_NAME[Pos->EPSquare][0];
    FENStr[i++] = SQUARE_NAME[Pos->EPSquare][1];
  }
  else
    FENStr[i++] = '-';

  FENStr[i++] = ' ';

  /* halfmove clock and fullmove number */
  sprintf(&FENStr[i], "%u %u", Pos->DrawPlies, Pos->MoveNum);

  return FENStr;
}

/******************************************************************************
 * int isPosLegal(const position *Pos);
 * PARAMETERS
 *    Pos - pointer to the position to test.
 * DESCRIPTION
 *    Determine if the given position is legal. Does not check or modify the
 *    PF_INVALID flag.
 * RETURN VALUE
 *    Returns 1 if the position is legal, or 0 otherwise.
 */
int isPosLegal(const position *Pos)
{
  const color Active = (Pos->Flags & PF_WHITEMOVE)?(WHITE):(BLACK);

  /* verify that there is exactly one king per side */
  if (Pos->OccBy[WHITE][KING] != LSB(Pos->OccBy[WHITE][KING]))
    return 0;
  if (Pos->OccBy[BLACK][KING] != LSB(Pos->OccBy[BLACK][KING]))
    return 0;

  /* verify that there are no pawns on the 1st or 8th ranks */
  if ((Pos->OccBy[WHITE][PAWN] | Pos->OccBy[BLACK][PAWN]) & PROM_RANKS)
    return 0;

  /* verify that there is an en-passant square only if there is a pawn which
   * could have crossed over that square on the previous move; this implies
   * that the en-passant square is on the 3rd rank if black is to move or on
   * the 6th rank if white is to move
   */
  if (Pos->EPSquare == NO_SQUARE) {/* empty */}
  else if (Active == BLACK)
  {
    if (RANK(Pos->EPSquare) != R_3)
      return 0;
    if (!(SQMASK(Pos->EPSquare + 1) & Pos->OccBy[WHITE][PAWN]))
      return 0;
  }
  else // Active == WHITE
  {
    if (RANK(Pos->EPSquare) != R_6)
      return 0;
    if (!(SQMASK(Pos->EPSquare - 1) & Pos->OccBy[BLACK][PAWN]))
      return 0;
  }

  /* verify that a castling right is available only if there is a king and a
   * rook on the applicable home squares
   */
  if ((Pos->Flags & PF_WCASTLE))
  {
    if (!(SQMASK(e1) & Pos->OccBy[WHITE][KING]))
      return 0;
    if (Pos->Flags & PF_WKSCASTLE && !(SQMASK(h1) & Pos->OccBy[WHITE][ROOK]))
      return 0;
    if (Pos->Flags & PF_WQSCASTLE && !(SQMASK(a1) & Pos->OccBy[WHITE][ROOK]))
      return 0;
  }
  if ((Pos->Flags & PF_BCASTLE))
  {
    if (!(SQMASK(e8) & Pos->OccBy[BLACK][KING]))
      return 0;
    if (Pos->Flags & PF_BKSCASTLE && !(SQMASK(h8) & Pos->OccBy[BLACK][ROOK]))
      return 0;
    if (Pos->Flags & PF_BQSCASTLE && !(SQMASK(a8) & Pos->OccBy[BLACK][ROOK]))
      return 0;
  }

  /* verify that the king of the side not to move is not under direct attack */
  if (attacked(Pos, firstSq(Pos->OccBy[!Active][KING]), Active))
    return 0;

  return 1;
}

/* end of file */
