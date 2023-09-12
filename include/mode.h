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
 * Interface definition for Quadus gamemodes.
 */
#ifndef QDS__MODE_H
#define QDS__MODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <game.h>

typedef struct qdsGamemode
{
	/**
	 * Allocate and initialize data used by the game mode.
	 */
	void *(*init)();
	/**
	 * Deallocate data used by the game mode.
	 */
	void (*destroy)(void *modeData);

	qdsSpawnCallback *onSpawn;
	qdsMoveCallback *onMove;
	qdsRotateCallback *onRotate;
	qdsDropCallback *onDrop;
	qdsLockCallback *onLock;
	qdsHoldCallback *onHold;
	qdsLineFilledCallback *onLineFilled;
	qdsLineClearCallback *onLineClear;
	qdsTopOutCallback *onTopOut;

	/**
	 * Get a mode-defined property.
	 * Returns NULL if property is not defined.
	 */
	void *(*getProperty)(qdsPlayfield *game, int key);
} qdsGamemode;

#ifdef __cplusplus
}
#endif

#endif /* !QDS__MODE_H */
