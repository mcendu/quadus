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
/**
 * Interface definition for Quadus rulesets.
 */
#ifndef QDS__RS_H
#define QDS__RS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include "piece.h"
#include "quadus.h"

typedef struct qdsRuleset
{
	/**
	 * Allocate and initialize data used by the ruleset.
	 */
	void *(*init)();
	/**
	 * Deallocate data used by the ruleset.
	 */
	void (*destroy)(void *rsData);

	qdsSpawnCallback *onSpawn;
	qdsMoveCallback *onMove;
	qdsRotateCallback *onRotate;
	qdsDropCallback *onDrop;
	qdsLockCallback *onLock;
	qdsHoldCallback *onHold;
	qdsLineFilledCallback *onLineFilled;
	qdsLineClearCallback *onLineClear;
	qdsTopOutCallback *onTopOut;

	qdsCustomCall *call;

	/**
	 * Advance the game state by one cycle.
	 */
	void (*doGameCycle)(qdsGame *game, unsigned int input);

	/**
	 * Get the horizontal spawn position.
	 */
	int (*spawnX)(qdsGame *game);
	/**
	 * Get the vertical spawn position.
	 */
	int (*spawnY)(qdsGame *game);

	/**
	 * Get a piece in the piece queue.
	 */
	int (*getPiece)(void *rsData, int position);
	/**
	 * Remove and return the topmost piece from the piece queue.
	 */
	int (*shiftPiece)(void *rsData);

	/**
	 * Get the shape of a piece.
	 */
	const qdsCoords *(*getShape)(int type, int orientation);
	/**
	 * Check if a piece can be rotated. The kick offset is
	 * returned in x and y.
	 */
	int (*canRotate)(qdsGame *game, int rotation, int *x, int *y);
} qdsRuleset;

/**
 * Spawn a specified piece. If the specified piece is 0, draw from
 * the piece queue. Returns whether the newly spawned active piece
 * doesn't overlap the playfield.
 */
QDS_API bool qdsSpawn(qdsGame *, int type);
/**
 * Teleport the active mino by a specified offset. Returns whether
 * the teleportation succeeded.
 */
QDS_API bool qdsTeleport(qdsGame *, int x, int y);
/**
 * Horizontally move the active piece by up to a specified offset.
 * Returns the actual offset moved.
 */
QDS_API int qdsMove(qdsGame *p, int offset);
/**
 * Drop the active piece by up to a specified number of tiles.
 * Returns the actual amount dropped.
 *
 * Specifying a negative distance is equivalent to specifying 0.
 */
QDS_API int qdsDrop(qdsGame *p, int type, int distance);
/**
 * Rotate the active mino. Returns whether the rotation succeeded,
 * and whether the rotation is considered a twist.
 */
QDS_API int qdsRotate(qdsGame *, int rotation);
/**
 * Locks the active mino and make it part of the playfield.
 * Returns whether the lock succeeded.
 */
QDS_API bool qdsLock(qdsGame *);
/**
 * Swaps the active mino with the held mino. If no mino is held, pull
 * from the piece queue instead. Returns whether holding succeeded and
 * whether the new active mino overlaps with the playfield.
 */
QDS_API int qdsHold(qdsGame *);
/**
 * Clear a line from the playfieled if it is filled. Returns whether
 * the line is cleared.
 *
 * When clearing multiple lines, this function should be called top
 * line first to prevent off-by-one bugs.
 */
QDS_API bool qdsClearLine(qdsGame *, int y);
/**
 * Insert lines into the bottom of the playfield. Returns false if
 * this results in a top-out, true otherwise.
 */
QDS_API bool qdsAddLines(qdsGame *, const qdsLine *lines, size_t height);

/**
 * Check if the active mino does not overlap with the playfield after
 * moving and rotating.
 */
QDS_API bool qdsCanRotate(qdsGame *, int x, int y, int rotation);
/**
 * Check if the active mino does not overlap with the playfield after
 * moving.
 */
#define qdsCanMove(p, x, y) (qdsCanRotate((p), (x), (y), 0))
/**
 * Check if the active mino overlaps the playfield.
 */
#define qdsOverlaps(p) (!qdsCanRotate((p), 0, 0, 0))
/**
 * Check if the active mino is grounded. The active mino is grounded
 * if it is unable to drop any further.
 */
#define qdsGrounded(p) \
	(!qdsCanRotate((p), 0, -1, 0) || !qdsCanRotate((p), 0, 0, 0))

/**
 * Get a pointer to the ruleset's data.
 */
QDS_API void *qdsGetRulesetData(const qdsGame *);

/*
 * Built-in rulesets.
 */

QDS_API extern const qdsRuleset qdsRulesetStandard;

#ifdef __cplusplus
}
#endif

#endif /* !QDS__RS_H */
