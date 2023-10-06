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
#include "mockruleset.h"

#include <stdbool.h>
#include <stdlib.h>

#include <piece.h>
#include <quadus.h>
#include <ruleset.h>

/**
 * Define mock ruleset and gamemode event handlers.
 *
 * WARNING: This is a hack. the comma operator should always be wrapped
 * in parentheses. Do not insert a semicolon after this macro.
 *
 * Unfortunately the only alternative for this macro is to declare
 * multiple handlers with nearly identical code, which is tedious and
 * can lend to more issues than this inelegant thing.
 */
#define EVENT_HANDLER(name, rettype, body, game, ...)           \
	static rettype name##Rs(__VA_ARGS__)                        \
	{                                                           \
		struct mockRulesetData *data = qdsGetRulesetData(game); \
		body                                                    \
	}                                                           \
	static rettype name##Mode(__VA_ARGS__)                      \
	{                                                           \
		struct mockRulesetData *data = qdsGetModeData(game);    \
		body                                                    \
	}

static void *init(void)
{
	struct mockRulesetData *data = calloc(1, sizeof(struct mockRulesetData));
	return data;
}

EVENT_HANDLER(
	onSpawn,
	bool,
	{
		data->spawnCount++;
		data->spawnType = type;
		return !data->blockSpawn;
	},
	game,
	qdsGame *game,
	int type)

EVENT_HANDLER(
	onMove,
	bool,
	{
		data->moveCount++;
		data->moveOffset = offset;
		return !data->blockMove;
	},
	game,
	qdsGame *game,
	int offset)

EVENT_HANDLER(
	onRotate,
	bool,
	{
		data->rotateCount++;
		data->rotation = rotation;
		return !data->blockRotate;
	},
	game,
	qdsGame *game,
	int rotation)

EVENT_HANDLER(
	onDrop,
	bool,
	{
		data->dropCount++;
		data->dropType = type;
		data->dropDistance = distance;
		return !data->blockDrop;
	},
	game,
	qdsGame *game,
	int type,
	int distance)

EVENT_HANDLER(
	onLock,
	bool,
	{
		data->lockCount++;
		return !data->blockLock;
	},
	game,
	qdsGame *game)

EVENT_HANDLER(
	onHold,
	bool,
	{
		data->holdCount++;
		data->holdPiece = piece;
		return !data->blockHold;
	},
	game,
	qdsGame *game,
	int piece)

EVENT_HANDLER(
	onLineFill,
	void,
	{ data->lineFillCount++; },
	game,
	qdsGame *game,
	int y)

EVENT_HANDLER(
	onLineClear,
	bool,
	{
		data->lineClearCount++;
		data->lineCleared = y;
		return !data->blockLineClear;
	},
	game,
	qdsGame *game,
	int y);

EVENT_HANDLER(
	onTopOut,
	void,
	{ data->topOutCount++; },
	game,
	qdsGame *game);

static void mockGameCycle(qdsGame *game, unsigned int input)
{
	struct mockRulesetData *data = qdsGetRulesetData(game);

	data->cycleCount += 1;
	data->lastInput = input;
}

static int spawnX(qdsGame *game)
{
	return 4;
}

static int spawnY(qdsGame *game)
{
	return 20;
}

static int getPiece(struct mockRulesetData *data, int pos)
{
	data->queuePos = pos;
	return QDS_PIECE_O;
}

static int shiftPiece(struct mockRulesetData *data)
{
	++data->shiftCount;
	return QDS_PIECE_O;
}

static const qdsPiecedef *standardShapes[] = {
	&qdsPieceNone, &qdsPieceI, &qdsPieceJ, &qdsPieceL,
	&qdsPieceO,	   &qdsPieceS, &qdsPieceT, &qdsPieceZ,
};

static const qdsCoords *getShape(int type, int o)
{
	type %= 8;
	o %= 4;
	const qdsPiecedef *def = standardShapes[type];
	return ((const qdsCoords **)def)[o];
}

static int rotationCheck(qdsGame *p, int r, int *x, int *y)
{
	if (!qdsCanRotate(p, 0, 0, r)) return QDS_ROTATE_FAILED;

	*x = 0;
	*y = 0;
	return QDS_ROTATE_NORMAL;
}

const qdsRuleset *mockRuleset = &(const qdsRuleset){
	.init = init,
	.destroy = free,
	.events = {
		.onSpawn = onSpawnRs,
		.onMove = onMoveRs,
		.onRotate = onRotateRs,
		.onDrop = onDropRs,
		.onLock = onLockRs,
		.onHold = onHoldRs,
		.onLineFilled = onLineFillRs,
		.onLineClear = onLineClearRs,
		.onTopOut = onTopOutRs,
	},
	.doGameCycle = mockGameCycle,
	.spawnX = spawnX,
	.spawnY = spawnY,
	.getPiece = (int (*)(void *, int))getPiece,
	.shiftPiece = (int (*)(void *))shiftPiece,
	.getShape = getShape,
	.canRotate = rotationCheck,
};

const qdsRuleset *noHandlerRuleset = &(const qdsRuleset){
	.init = init,
	.destroy = free,
	.doGameCycle = mockGameCycle,
	.spawnX = spawnX,
	.spawnY = spawnY,
	.getPiece = (int (*)(void *, int))getPiece,
	.shiftPiece = (int (*)(void *))shiftPiece,
	.getShape = getShape,
	.canRotate = rotationCheck,
};

const qdsGamemode *mockGamemode = &(const qdsGamemode){
	.init = init,
	.destroy = free,
	.events = {
		.onSpawn = onSpawnMode,
		.onMove = onMoveMode,
		.onRotate = onRotateMode,
		.onDrop = onDropMode,
		.onLock = onLockMode,
		.onHold = onHoldMode,
		.onLineFilled = onLineFillMode,
		.onLineClear = onLineClearMode,
		.onTopOut = onTopOutMode,
	},
};

const qdsGamemode *noHandlerGamemode = &(const qdsGamemode){
	.init = init,
	.destroy = free,
};
