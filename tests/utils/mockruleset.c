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
#include <stdbool.h>
#include <stdlib.h>

#include <piece.h>
#include <playfield.h>
#include <rs.h>

#include "game.h"
#include "mockruleset.h"

/**
 * Define mock ruleset and gamemode event handlers.
 *
 * WARNING: This is a hack. the comma operator should always be wrapped
 * in parentheses. Do not insert a semicolon after this macro.
 */
#define EVENT_HANDLER(name, rettype, body, game, ...)    \
	static rettype name##Rs(__VA_ARGS__)                 \
	{                                                    \
		struct mockRulesetData *data = (game)->rsData;   \
		body                                             \
	}                                                    \
	static rettype name##Mode(__VA_ARGS__)               \
	{                                                    \
		struct mockRulesetData *data = (game)->modeData; \
		body                                             \
	}

static void *init()
{
	struct mockRulesetData *data = calloc(1, sizeof(struct mockRulesetData));
	return data;
}

static void destroy(void *data)
{
	free(data);
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
	qdsPlayfield *game,
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
	qdsPlayfield *game,
	int offset)

EVENT_HANDLER(
	onRotate,
	bool,
	{
		data->rotateCount++;
		return !data->blockRotate;
	},
	game,
	qdsPlayfield *game,
	int rotation)

EVENT_HANDLER(
	onDrop,
	bool,
	{
		data->dropCount++;
		return !data->blockDrop;
	},
	game,
	qdsPlayfield *game,
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
	qdsPlayfield *game)

EVENT_HANDLER(
	onHold,
	bool,
	{
		data->holdCount++;
		return !data->blockHold;
	},
	game,
	qdsPlayfield *game)

EVENT_HANDLER(
	onLineFill,
	void,
	{ data->lineFillCount++; },
	game,
	qdsPlayfield *game,
	int y)

EVENT_HANDLER(
	onLineClear,
	bool,
	{
		data->lineClearCount++;
		return !data->blockLineClear;
	},
	game,
	qdsPlayfield *game,
	int y);

EVENT_HANDLER(
	onTopOut,
	void,
	{ data->topOutCount++; },
	game,
	qdsPlayfield *game);

static void mockGameCycle(qdsPlayfield *game, unsigned int input)
{
	struct mockRulesetData *data = game->rsData;

	data->cycleCount += 1;
	data->lastInput = input;
}

static int spawnX(qdsPlayfield *game)
{
	return 4;
}

static int spawnY(qdsPlayfield *game)
{
	return 20;
}

static int getPiece(void *d, int pos)
{
	return QDS_PIECE_O;
}

static int shiftPiece(void *data)
{
	++((struct mockRulesetData *)data)->shiftCount;
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

static int rotationCheck(qdsPlayfield *p, int r, int *x, int *y)
{
	if (!qdsPlayfieldCanRotate(p, 0, 0, r)) return QDS_PLAYFIELD_ROTATE_FAILED;

	if (x) *x = 0;
	if (y) *y = 0;
	return QDS_PLAYFIELD_ROTATE_NORMAL;
}

const qdsRuleset *mockRuleset = &(const qdsRuleset){
	.init = init,
	.destroy = destroy,
	.onSpawn = onSpawnRs,
	.onMove = onMoveRs,
	.onRotate = onRotateRs,
	.onDrop = onDropRs,
	.onLock = onLockRs,
	.onHold = onHoldRs,
	.onLineFilled = onLineFillRs,
	.onLineClear = onLineClearRs,
	.onTopOut = onTopOutRs,
	.doGameCycle = mockGameCycle,
	.spawnX = spawnX,
	.spawnY = spawnY,
	.getPiece = getPiece,
	.shiftPiece = shiftPiece,
	.getShape = getShape,
	.canRotate = rotationCheck,
};

const qdsRuleset *noHandlerRuleset = &(const qdsRuleset){
	.init = init,
	.destroy = destroy,
	.doGameCycle = mockGameCycle,
	.spawnX = spawnX,
	.spawnY = spawnY,
	.getPiece = getPiece,
	.shiftPiece = shiftPiece,
	.getShape = getShape,
	.canRotate = rotationCheck,
};

const qdsGamemode *mockGamemode = &(const qdsGamemode){
	.init = init,
	.destroy = destroy,
	.onSpawn = onSpawnMode,
	.onMove = onMoveMode,
	.onRotate = onRotateMode,
	.onDrop = onDropMode,
	.onLock = onLockMode,
	.onHold = onHoldMode,
	.onLineFilled = onLineFillMode,
	.onLineClear = onLineClearMode,
	.onTopOut = onTopOutMode,
};

const qdsGamemode *noHandlerGamemode = &(const qdsGamemode){
	.init = init,
	.destroy = destroy,
};
