/******************************************************************************
 * $Id: fen.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Import and export FEN strings.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__FEN_H
#define VAPOR__FEN_H

#include "vapor.h"
#include "chess.h"

extern const char *const STARTPOS;

/******************************************************************************
 * int importFEN(position *Pos, const char *FENStr);
 * PARAMETERS
 *    Pos - pointer to the position structure into which FENStr is imported.
 *    FENStr - the position in Forsyth–Edwards Notation.
 * DESCRIPTION
 *    Import the FEN position pointed to by FENStr into *Pos.
 * RETURN VALUE
 *    Returns 0 if the string represents a valid position, or -1 for an invalid
 *    FEN position.
 */
int importFEN(position *Pos, const char *FENStr);

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
char *exportFEN(char *FENStr, const position *Pos);

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
int isPosLegal(const position *Pos);

#endif // #ifndef VAPOR__FEN_H

/* end of file */
