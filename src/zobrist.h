/******************************************************************************
 * $Id$
 * Project: Vapor Chess
 * Purpose: Handle Zobrist Keys
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__ZOBRIST_H
#define VAPOR__ZOBRIST_H

#include "vapor.h"
#include "chess.h"

extern const zobrist Z_PLACEMENT[NUM_COLORS][NUM_PIECES+1][NUM_SQUARES];
extern const zobrist Z_EPSQ[NUM_FILES];
extern const zobrist Z_CASTLE[16];
extern const zobrist Z_WHITEMOVE;

zobrist calcZobrist(const position *Pos);

#endif // #ifndef VAPOR__ZOBRIST_H

/* end of file */
