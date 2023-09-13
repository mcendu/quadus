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

#include <stdbool.h>

#include <mino.h>
#include <qdsbuild.h>

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

/*
 * Results for rotation.
 */

/** The rotation failed. */
#define QDS_PLAYFIELD_ROTATE_FAILED 0
/** The rotation succeeded. */
#define QDS_PLAYFIELD_ROTATE_NORMAL 1
/** The rotation succeeded and is recognized as a twist. */
#define QDS_PLAYFIELD_ROTATE_TWIST 2
/** The rotation succeeded and is recognized as a mini twist. */
#define QDS_PLAYFIELD_ROTATE_TWIST_MINI 3

/**
 * A tile on the playfield.
 */
typedef unsigned char qdsTile;

/**
 * The game state of Quadus.
 */
typedef struct qdsPlayfield qdsPlayfield;
/**
 * Defines the rules of Quadus.
 */
typedef struct qdsRuleset qdsRuleset;
/**
 * The game mode.
 */
typedef struct qdsGamemode qdsGamemode;

typedef bool qdsSpawnCallback(qdsPlayfield *game, int piece);
typedef bool qdsMoveCallback(qdsPlayfield *game, int offset);
typedef bool qdsRotateCallback(qdsPlayfield *game, int rotation);
typedef bool qdsDropCallback(qdsPlayfield *game, int type, int distance);
typedef bool qdsLockCallback(qdsPlayfield *game);
typedef bool qdsHoldCallback(qdsPlayfield *game);
typedef void qdsLineFilledCallback(qdsPlayfield *game, int y);
typedef bool qdsLineClearCallback(qdsPlayfield *game, int y);
typedef void qdsTopOutCallback(qdsPlayfield *game);

/**
 * Initialize the game state.
 */
QDS_API void qdsPlayfieldInit(qdsPlayfield *);

/**
 * Clean up the game state.
 */
QDS_API void qdsPlayfieldCleanup(qdsPlayfield *);

/**
 * Advance the game state by one cycle.
 */
QDS_API void qdsPlayfieldAdvance(qdsPlayfield *, unsigned int input);

/**
 * Get the current ruleset.
 */
QDS_API const qdsRuleset *qdsPlayfieldGetRuleset(qdsPlayfield *);
/**
 * Set the game's ruleset.
 */
QDS_API void qdsPlayfieldSetRuleset(qdsPlayfield *, const qdsRuleset *ruleset);

/**
 * Get the current game mode.
 */
QDS_API const qdsGamemode *qdsPlayfieldGetMode(qdsPlayfield *);
/**
 * Set the game's mode.
 */
QDS_API void qdsPlayfieldSetMode(qdsPlayfield *, const qdsGamemode *mode);

/**
 * Get the playfield.
 */
QDS_API qdsTile (*qdsPlayfieldGetPlayfield(qdsPlayfield *))[10];
/**
 * Get the active piece.
 */
QDS_API qdsMino *qdsPlayfieldGetActive(qdsPlayfield *, int *x, int *y);
/**
 * Get the piece at a specific position in the queue.
 */
QDS_API int qdsPlayfieldGetNextPiece(qdsPlayfield *, int pos);
/**
 * Get the held piece.
 */
QDS_API int qdsPlayfieldGetHeldPiece(qdsPlayfield *);

/**
 * Get data defined by the ruleset and the game mode.
 */
QDS_API void *qdsPlayfieldGet(qdsPlayfield *, int key);

#ifdef __cplusplus
}
#endif

#endif /* !QDS__GAME_H */
