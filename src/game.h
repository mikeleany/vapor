/******************************************************************************
 * $Id: game.h 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Maintain the game history.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__GAME_H
#define VAPOR__GAME_H

#include "vapor.h"
#include "chess.h"
#include "zobrist.h"

#define MAX_GAME_PLIES  1024
#define MAX_ZHIST_LEN   128

extern zobrist ZobHistory[MAX_ZHIST_LEN];
extern int ZHistLength;

/* pointer to the current position */
extern const position *CurPos;

/******************************************************************************
 * void resetGame(void);
 * DESCRIPTION
 *    Resets the game to the standard starting position. Clears any previous
 *    game history.
 * RETURN VALUE
 *    Does not return a value.
 */
void resetGame(void);

/******************************************************************************
 * int setGamePos(const char *FENStr);
 * PARAMETERS
 *    FENStr -- FEN of the position to set.
 * DESCRIPTION
 *    Sets the current position to that given by FENStr. Clears any previous
 *    game history.
 * RETURN VALUE
 *    Returns 0 for success or -1 for failure.
 */
int setGamePos(const char *FENStr);

/******************************************************************************
 * int makeGameMove(const char *MoveStr);
 * PARAMETERS
 *    MoveStr -- The move to make in coordinate notation.
 * DESCRIPTION
 *    Makes the move on the game board, updating the current position and game
 *    history as appropriate.
 * RETURN VALUE
 *    Returns 0 for success or -1 for failure.
 */
int makeGameMove(const char *MoveStr);

#endif // #ifndef VAPOR__GAME_H

/* end of file */
