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

#include <game.h>
#include <piece.h>

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

#ifdef __cplusplus
}
#endif

#endif /* !QDS__RS_H */
