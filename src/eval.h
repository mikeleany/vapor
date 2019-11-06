/******************************************************************************
 * $Id: eval.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Evaluate a position.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__EVAL_H
#define VAPOR__EVAL_H

#include "vapor.h"
#include "chess.h"

extern const int PieceVal[NUM_PIECES];

int evaluate(const position *Pos);

#endif // #ifndef VAPOR__EVAL_H

/* end of file */
