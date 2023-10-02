/*
 * Copyright (c) 2023 McEndu
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef QDS__GAME_H
#define QDS__GAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>

#include "qdsbuild.h"

#define QDS_INPUT_LEFT 1
#define QDS_INPUT_RIGHT 2
#define QDS_INPUT_ROTATE_C 4
#define QDS_INPUT_ROTATE_CC 8
#define QDS_INPUT_SOFT_DROP 16
#define QDS_INPUT_HARD_DROP 32
#define QDS_INPUT_HOLD 64
#define QDS_INPUT_CUSTOM 128

#define QDS_DROP_GRAVITY 0
#define QDS_DROP_HARD 1
#define QDS_DROP_SOFT 2

#define QDS_ROTATION_NONE 0
#define QDS_ROTATION_CLOCKWISE 1
#define QDS_ROTATION_COUNTERCLOCKWISE -1

#define QDS_ORIENTATION_BASE 0
#define QDS_ORIENTATION_C 1
#define QDS_ORIENTATION_FLIP 2
#define QDS_ORIENTATION_CC 3

#define QDS_ROTATE_FAILED 0
#define QDS_ROTATE_NORMAL 1
#define QDS_ROTATE_TWIST 2
#define QDS_ROTATE_TWIST_MINI 3

#define QDS_HOLD_SUCCESS 0
#define QDS_HOLD_BLOCKED -1
#define QDS_HOLD_TOPOUT 1

/**
 * A tile on the playfield.
 */
typedef unsigned char qdsTile;
/**
 * A line in the playfield.
 */
typedef qdsTile qdsLine[16];

/**
 * The game state of Quadus.
 */
typedef struct qdsGame qdsGame;
/**
 * Defines the rules of Quadus.
 */
typedef struct qdsRuleset qdsRuleset;
/**
 * The game mode.
 */
typedef struct qdsGamemode qdsGamemode;

typedef bool qdsSpawnCallback(qdsGame *game, int piece);
typedef bool qdsMoveCallback(qdsGame *game, int offset);
typedef bool qdsRotateCallback(qdsGame *game, int rotation);
typedef bool qdsDropCallback(qdsGame *game, int type, int distance);
typedef bool qdsLockCallback(qdsGame *game);
typedef bool qdsHoldCallback(qdsGame *game, int piece);
typedef void qdsLineFilledCallback(qdsGame *game, int y);
typedef bool qdsLineClearCallback(qdsGame *game, int y);
typedef void qdsTopOutCallback(qdsGame *game);

/**
 * Call a ruleset or gamemode defined function.
 *
 * If the function is not defined for the ruleset or gamemode, -ENOTTY
 * is returned and ap is not modified. Otherwise, the state of ap is
 * undefined. In both cases, the caller should free ap using va_end().
 */
typedef int qdsCustomCall(qdsGame *, unsigned long req, void *argp);

/**
 * Allocate and initialize a game state.
 */
QDS_API qdsGame *qdsNewGame();
/**
 * Deallocate a game state.
 */
QDS_API void qdsDestroyGame(qdsGame *);

/**
 * Initialize the game state.
 */
QDS_API void qdsInitGame(qdsGame *);
/**
 * Clean up the game state.
 */
QDS_API void qdsCleanupGame(qdsGame *);

/**
 * Advance the game state by one cycle.
 */
QDS_API void qdsRunCycle(qdsGame *, unsigned int input);

/**
 * Get the current ruleset.
 */
QDS_API const qdsRuleset *qdsGetRuleset(const qdsGame *);
/**
 * Set the game's ruleset.
 */
QDS_API void qdsSetRuleset(qdsGame *, const qdsRuleset *ruleset);

/**
 * Get the current game mode.
 */
QDS_API const qdsGamemode *qdsGetMode(const qdsGame *);
/**
 * Set the game's mode.
 */
QDS_API void qdsSetMode(qdsGame *, const qdsGamemode *mode);

/**
 * Get the playfield.
 */
QDS_API const qdsLine *qdsGetPlayfield(const qdsGame *);
/**
 * Get a tile in the playfield. Effectively `qdsGetPlayfield(game)[y][x]`
 * with bounds checking.
 */
QDS_API qdsTile qdsGetTile(const qdsGame *, int x, int y);
/**
 * Get the active piece's type.
 */
QDS_API int qdsGetActivePieceType(const qdsGame *);
/**
 * Get the position of the active piece.
 */
QDS_API void qdsGetActivePosition(const qdsGame *, int *x, int *y);
/**
 * Get the horizontal position of the active piece.
 */
QDS_API int qdsGetActiveX(const qdsGame *);
/**
 * Get the vertical position of the active piece.
 */
QDS_API int qdsGetActiveY(const qdsGame *);
/**
 * Get the vertical position of the ghost piece.
 */
QDS_API int qdsGetGhostY(qdsGame *);
/**
 * Get the orientation of the active piece.
 */
QDS_API int qdsGetActiveOrientation(const qdsGame *);
/**
 * Get the piece at a specific position in the queue.
 */
QDS_API int qdsGetNextPiece(const qdsGame *, int pos);
/**
 * Get the held piece.
 */
QDS_API int qdsGetHeldPiece(const qdsGame *);

/**
 * Call a ruleset-defined function. Returns 0 on success and non-zero
 * on failure.
 */
QDS_API int qdsCall(qdsGame *, unsigned long req, void *argp);

#ifdef __cplusplus
}
#endif

#endif /* !QDS__GAME_H */
