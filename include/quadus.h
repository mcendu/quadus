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
typedef struct qdsUserInterface qdsUserInterface;

/**
 * Represents coordinates in a piece shape.
 *
 * A shape is an array of tiles, represented as coordinates relative to
 * its rotation center, ending with { 127, 127 }.
 */
typedef struct
#if defined(__GNUC__)
	__attribute__((aligned(2)))
#endif
	qdsCoords
{
	signed char x;
	signed char y;
} qdsCoords;

/**
 * Called on attempt to spawn a piece. Return false to cancel the
 * piece spawn.
 */
typedef bool qdsSpawnCallback(qdsGame *game, int piece);
/**
 * Called on attempt to move the piece. Return false to cancel the move.
 */
typedef bool qdsMoveCallback(qdsGame *game, int offset);
/**
 * Called on attempt to rotate the piece. Return false to cancel the
 * rotation.
 */
typedef bool qdsRotateCallback(qdsGame *game, int rotation, int twist);
/**
 * Called on attempt to make the piece fall down. Return false to
 * prevent this.
 */
typedef bool qdsDropCallback(qdsGame *game, int type, int distance);
/**
 * Called on attempt to lock the piece. Return false to cancel the
 * lock.
 */
typedef bool qdsLockCallback(qdsGame *game);
/**
 * Called after piece lockdown.
 */
typedef void qdsPostLockCallback(qdsGame *game);
/**
 * Called on attempt to hold the piece. Return false to cancel the hold.
 */
typedef bool qdsHoldCallback(qdsGame *game, int piece);
/**
 * Called when a line is filled.
 */
typedef void qdsLineFilledCallback(qdsGame *game, int y);
/**
 * Called on attempt to delete a line. Return false to cancel the line
 * deletion.
 */
typedef bool qdsLineClearCallback(qdsGame *game, int y);
/**
 * Called on game over.
 */
typedef void qdsTopOutCallback(qdsGame *game);

/**
 * Call a ruleset or gamemode defined function.
 *
 * If the function is not defined for the ruleset or gamemode, -ENOTTY
 * is returned and ap is not modified. Otherwise, the state of ap is
 * undefined. In both cases, the caller should free ap using va_end().
 */
typedef int qdsCustomCall(qdsGame *, unsigned long req, void *argp);

typedef struct qdsEventTable
{
	qdsSpawnCallback *onSpawn;
	qdsMoveCallback *onMove;
	qdsRotateCallback *onRotate;
	qdsDropCallback *onDrop;
	qdsLockCallback *onLock;
	qdsHoldCallback *onHold;
	qdsLineFilledCallback *onLineFilled;
	qdsLineClearCallback *onLineClear;
	qdsTopOutCallback *onTopOut;
	qdsPostLockCallback *postLock;
} qdsEventTable;

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
 * Get the currently bound application interface.
 */
QDS_API const qdsUserInterface *qdsGetUi(const qdsGame *);
/**
 * Bind an application interface.
 */
QDS_API void qdsSetUi(qdsGame *, const qdsUserInterface *ui, void *data);

/**
 * Get the playfield.
 */
QDS_API qdsLine *qdsGetPlayfield(qdsGame *);
/**
 * Get the height of the stack.
 */
QDS_API int qdsGetFieldHeight(const qdsGame *);
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
 * Get the orientation of the active piece.
 */
QDS_API int qdsGetActiveOrientation(const qdsGame *);
/**
 * Get the shape of a specific piece type.
 */
QDS_API const qdsCoords *qdsGetShape(const qdsGame *,
									 int piece,
									 int orientation);
/**
 * Get the shape of the active piece.
 */
#define qdsGetActiveShape(game) (qdsGetShape(game, -1, -1))
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
QDS_API int qdsGetGhostY(const qdsGame *);
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

/*
 * Built-in rulesets.
 */

QDS_API extern const qdsRuleset qdsRulesetStandard;

/*
 * Built-in gamemodes.
 */

QDS_API extern const qdsGamemode qdsModeMarathon;

#ifdef __cplusplus
}
#endif

#endif /* !QDS__GAME_H */
