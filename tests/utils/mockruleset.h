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
#ifndef MOCKRULESET_H
#define MOCKRULESET_H

#include <stdbool.h>

#include <mode.h>
#include <ruleset.h>

/**
 * Ruleset for mocking in unit tests.
 *
 * This ruleset uses standard natural rotations, but without wall kicks;
 * in addition, the piece generator always deals the O piece.
 */
extern const qdsRuleset *mockRuleset;

/**
 * Ruleset for mocking in unit tests. Similar to mockRuleset, but with
 * all event handlers being NULL.
 */
extern const qdsRuleset *noHandlerRuleset;

/**
 * Gamemode for mocking in unit tests.
 */
extern const qdsGamemode *mockGamemode;

/**
 * Gamemode for mocking in unit tests. Similar to mockRuleset, but with
 * all event handlers being NULL.
 */
extern const qdsGamemode *noHandlerGamemode;

/**
 * Data used by mockRuleset and mockGamemode.
 */
typedef struct mockRulesetData
{
	int spawnCount;
	int moveCount;
	int rotateCount;
	int dropCount;
	int lockCount;
	int holdCount;
	int lineFillCount;
	int lineClearCount;
	int topOutCount;

	bool blockSpawn : 1;
	bool blockMove : 1;
	bool blockRotate : 1;
	bool blockDrop : 1;
	bool blockLock : 1;
	bool blockHold : 1;
	bool blockLineClear : 1;

	int cycleCount;
	unsigned int lastInput;
	int shiftCount;

	int spawnType;
	int moveOffset;
	int rotation;
	int dropType;
	int dropDistance;
	int holdPiece;
	int lineCleared;
	int queuePos;
} mockRulesetData;

#endif /* !MOCKRULESET_H */
