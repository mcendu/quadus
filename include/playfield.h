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
 * Low level playfield actions for consumption by rulesets.
 */
#ifndef QDS__PLAYFIELD_H
#define QDS__PLAYFIELD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mode.h>
#include <qdsbuild.h>
#include <rs.h>

/*
 * Return values of qdsPlayfieldHold.
 */

/** Swapping minos succeeded. */
#define QDS_HOLD_SUCCESS 0
/** The swap was blocked. */
#define QDS_HOLD_BLOCKED -1
/** The swapped mino overlapped with the playfield. */
#define QDS_HOLD_TOPOUT 1

/**
 * Definition of qdsPlayfield.
 */
typedef struct qdsPlayfield
{
	qdsTile playfield[48][10];
	int x;
	int y;
	int piece;
	unsigned orientation;
	int top;
	int hold;

	const qdsRuleset *rs;
	void *rsData;
	const qdsGamemode *mode;
	void *modeData;
} qdsPlayfield;

/**
 * Initialize the game state.
 */
QDS_API void qdsInit(qdsPlayfield *);
/**
 * Clean up the game state.
 */
QDS_API void qdsCleanup(qdsPlayfield *);

/**
 * Spawn a specified piece. If the specified piece is 0, draw from
 * the piece queue. Returns whether the newly spawned active piece
 * doesn't overlap the playfield.
 */
QDS_API bool qdsSpawn(qdsPlayfield *, int type);
/**
 * Teleport the active mino by a specified offset. Returns whether
 * the teleportation succeeded.
 */
QDS_API bool qdsTeleport(qdsPlayfield *, int x, int y);
/**
 * Horizontally move the active piece by up to a specified offset.
 * Returns the actual offset moved.
 */
QDS_API int qdsMove(qdsPlayfield *p, int offset);
/**
 * Drop the active piece by up to a specified number of tiles.
 * Returns the actual amount dropped.
 *
 * Specifying a negative distance is equivalent to specifying 0.
 */
QDS_API int qdsDrop(qdsPlayfield *p, int type, int distance);
/**
 * Rotate the active mino. Returns whether the rotation succeeded,
 * and whether the rotation is considered a twist.
 */
QDS_API int qdsRotate(qdsPlayfield *, int rotation);
/**
 * Locks the active mino and make it part of the playfield.
 * Returns whether the lock succeeded.
 */
QDS_API bool qdsLock(qdsPlayfield *);
/**
 * Swaps the active mino with the held mino. If no mino is held, pull
 * from the piece queue instead. Returns whether holding succeeded and
 * whether the new active mino overlaps with the playfield.
 */
QDS_API int qdsHold(qdsPlayfield *);
/**
 * Clear a line from the playfieled if it is filled. Returns whether
 * the line is cleared.
 *
 * When clearing multiple lines, this function should be called top
 * line first to prevent off-by-one bugs.
 */
QDS_API bool qdsClearLine(qdsPlayfield *, int y);

/**
 * Check if the active mino does not overlap with the playfield after
 * moving and rotating.
 */
QDS_API bool qdsCanRotate(qdsPlayfield *, int x, int y, int rotation);
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
 * Get the vertical position of the ghost piece.
 */
QDS_API int qdsGetGhostY(qdsPlayfield *);
/**
 * Check if the active mino is grounded. The active mino is grounded
 * if it is unable to drop any further.
 */
#define qdsGrounded(p) (!qdsCanRotate((p), 0, -1, 0))

/**
 * Get a pointer to the ruleset's data.
 */
QDS_API void *qdsGetRulesetData(qdsPlayfield *);

/**
 * Get a pointer to the gamemode's data.
 */
QDS_API void *qdsGetModeData(qdsPlayfield *);

#ifdef __cplusplus
}
#endif

#endif /* !QDS__PLAYFIELD_H */
