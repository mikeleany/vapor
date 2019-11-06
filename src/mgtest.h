/******************************************************************************
 * $Id: mgtest.h 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Tests the move generator.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__MGTEST_H
#define VAPOR__MGTEST_H

#include "vapor.h"

int mgtest(const char *FileName);

int printVariations(const char *Fen, int Depth);

int perftest(const char *Fen, int Depth);

#endif // #ifndef VAPOR__MGTEST_H

/* end of file */
