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
#include "rulesets/standard.h"
#include <config.h>
#include <quadus.h>
#include <quadus/calls.h>
#include <quadus/piece.h>
#include <quadus/piecegen/bag.h>
#include <quadus/ruleset.h>
#include <quadus/ruleset/input.h>
#include <quadus/ruleset/linequeue.h>
#include <quadus/ruleset/rand.h>
#include <quadus/ruleset/twist.h>
#include <quadus/ruleset/utils.h>

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define KEND                 \
	{                        \
		SCHAR_MAX, SCHAR_MAX \
	}

struct kickData
{
	const qdsCoords (*cw)[8];
	const qdsCoords (*ccw)[8];
};

static const qdsCoords kicksNormalClockwise[4][8] = {
	{ { 0, 0 }, { -1, 0 }, { -1, +1 }, { 0, -2 }, { -1, -2 }, KEND },
	{ { 0, 0 }, { +1, 0 }, { +1, -1 }, { 0, +2 }, { +1, +2 }, KEND },
	{ { 0, 0 }, { +1, 0 }, { +1, +1 }, { 0, -2 }, { +1, -2 }, KEND },
	{ { 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, +2 }, { -1, +2 }, KEND },
};

static const qdsCoords kicksNormalCounterClockwise[4][8] = {
	{ { 0, 0 }, { +1, 0 }, { +1, +1 }, { 0, -2 }, { +1, -2 }, KEND },
	{ { 0, 0 }, { +1, 0 }, { +1, -1 }, { 0, +2 }, { +1, +2 }, KEND },
	{ { 0, 0 }, { -1, 0 }, { -1, +1 }, { 0, -2 }, { -1, -2 }, KEND },
	{ { 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, +2 }, { -1, +2 }, KEND },
};

static const struct kickData kicksNormal = {
	kicksNormalClockwise,
	kicksNormalCounterClockwise,
};

static const qdsCoords kicksIClockwise[4][8] = {
	{ { 0, 0 }, { -2, 0 }, { +1, 0 }, { -2, -1 }, { +1, +2 }, KEND },
	{ { 0, 0 }, { -1, 0 }, { +2, 0 }, { -1, +2 }, { +2, -1 }, KEND },
	{ { 0, 0 }, { +2, 0 }, { -1, 0 }, { +2, +1 }, { -1, -2 }, KEND },
	{ { 0, 0 }, { +1, 0 }, { -2, 0 }, { +1, -2 }, { -2, +1 }, KEND },
};

static const qdsCoords kicksICounterClockwise[4][8] = {
	{ { 0, 0 }, { -1, 0 }, { +2, 0 }, { -1, +2 }, { +2, -1 }, KEND },
	{ { 0, 0 }, { +2, 0 }, { -1, 0 }, { +2, +1 }, { -1, -2 }, KEND },
	{ { 0, 0 }, { +1, 0 }, { -2, 0 }, { +1, -2 }, { -2, +1 }, KEND },
	{ { 0, 0 }, { -2, 0 }, { +1, 0 }, { -2, -1 }, { +1, +2 }, KEND },
};

static const struct kickData kicksI = {
	kicksIClockwise,
	kicksICounterClockwise,
};

static const struct pieceData
{
	const qdsPiecedef *shape;
	const struct kickData *kicks;
} pieces[] = {
	[QDS_PIECE_NONE] = { &qdsPieceNone, &kicksNormal },
	[QDS_PIECE_I] = { &qdsPieceI, &kicksI },
	[QDS_PIECE_J] = { &qdsPieceJ, &kicksNormal },
	[QDS_PIECE_L] = { &qdsPieceL, &kicksNormal },
	[QDS_PIECE_O] = { &qdsPieceO, &kicksNormal },
	[QDS_PIECE_S] = { &qdsPieceS, &kicksNormal },
	[QDS_PIECE_T] = { &qdsPieceT, &kicksNormal },
	[QDS_PIECE_Z] = { &qdsPieceZ, &kicksNormal },
};

static const int dropScore[4][5] = {
	[0] = { 0, 100, 300, 500, 800 },
	[QDS_ROTATE_NORMAL] = { 0, 100, 300, 500, 800 },
	[QDS_ROTATE_TWIST] = { 400, 800, 1200, 1600, 2000 },
	[QDS_ROTATE_TWIST_MINI] = { 100, 200, 400, 600, 800 },
};

static const int allClearBonus[] = { 0, 800, 1200, 1800, 2000 };

#define DEFAULT_LOCKTIME 30
#define DEFAULT_GRAVITY (65536 / 60)

static void *init(void)
{
	standardData *data = malloc(sizeof(standardData));
	if (!data) return NULL;

	qdsInitRulesetState(&data->baseState);

	data->time = 0;
	data->lines = 0;
	data->score = 0;
	data->combo = 0;

	data->inputState.lastInput = 0;
	data->inputState.direction = 0;

	qdsBagInit(&data->gen, time(NULL));

	return data;
}

static const qdsCoords *getShape(int type, int orientation)
{
	type %= 8;
	return ((const qdsCoords **)(pieces[type].shape))[orientation];
}

static int addLevelMultipliedScore(standardData *restrict data,
								   qdsGame *restrict game,
								   int pts)
{
	int level;
	if (qdsCall(game, QDS_GETLEVEL, &level) < 0) level = 1;
	data->score += pts * level;
	return data->score;
}

static bool resetLock(standardData *restrict data,
					  qdsGame *restrict game,
					  bool refresh)
{
	if (refresh) {
		int resets;
		if (qdsCall(game, QDS_GETRESETS, &resets) < 0) resets = 15;
		data->baseState.resetsLeft = resets;
	} else {
		if (data->baseState.resetsLeft == 0) return false;
		data->baseState.resetsLeft -= 1;
	}

	int lockTime;
	if (qdsCall(game, QDS_GETLOCKTIME, &lockTime) < 0)
		lockTime = DEFAULT_LOCKTIME;
	data->baseState.lockTimer = lockTime;
	return true;
}

static bool onSpawn(qdsGame *restrict game, int piece)
{
	standardData *data = qdsGetRulesetData(game);
	qdsHandleSpawn(&data->baseState);
	resetLock(data, game, true);
	return true;
}

static void doMovement(standardData *restrict data,
					   qdsGame *restrict game,
					   unsigned int input)
{
	bool grounded = qdsGrounded(game);
	if (qdsProcessMovement(&data->baseState, game, input) && grounded)
		data->baseState.reset = true;
}

static void doRotate(standardData *restrict data,
					 qdsGame *restrict game,
					 unsigned int input)
{
	bool grounded = qdsGrounded(game);
	if (qdsProcessRotation(&data->baseState, game, input) != QDS_ROTATE_FAILED
		&& grounded)
		data->baseState.reset = true;
}

static int canRotate(qdsGame *restrict game,
					 int rotation,
					 int *restrict x,
					 int *restrict y)
{
	if (rotation == 0) return QDS_ROTATE_FAILED;

	int piece = qdsGetActivePieceType(game) % 8;
	int orientation = qdsGetActiveOrientation(game);
	const qdsCoords *kicks;

	if (rotation > 0) {
		rotation = 1;
		kicks = pieces[piece].kicks->cw[orientation];
	} else {
		rotation = -1;
		kicks = pieces[piece].kicks->ccw[orientation];
	}

	QDS_SHAPE_FOREACH (k, kicks) {
		if (qdsCanRotate(game, k->x, k->y, rotation)) {
			*x = k->x;
			*y = k->y;

			/*
			 * Full twist: passes immobile check
			 * Mini twist: fails immobile but passes three-corner
			 */
			if (qdsCheckTwistImmobile(game, k->x, k->y, rotation))
				return QDS_ROTATE_TWIST;
			if (qdsCheckTwist3Corner(game, k->x, k->y, rotation))
				return QDS_ROTATE_TWIST_MINI;
			return QDS_ROTATE_NORMAL;
		}
	}

	return QDS_ROTATE_FAILED;
}

static void doGravity(standardData *restrict data,
					  qdsGame *restrict game,
					  unsigned int input)
{
	if (input & QDS_INPUT_HARD_DROP) {
		qdsDrop(game, QDS_DROP_HARD, 48);
		return qdsProcessLock(&data->baseState, game);
	}

	bool reset = false;
	if (data->baseState.reset) {
		reset = resetLock(data, game, false);
		data->baseState.reset = false;
	}

	if (!reset || !qdsGrounded(game))
		return qdsProcessGravity(&data->baseState, game, input);
}

static bool onDrop(qdsGame *restrict game, int type, int distance)
{
	standardData *data = qdsGetRulesetData(game);
	if (type == QDS_DROP_HARD) data->score += distance * 2;
	if (type == QDS_DROP_SOFT) data->score += distance;
	if (distance > 0) data->baseState.twistCheckResult = 0;
	return true;
}

static void scoreLineClear(standardData *restrict data,
						   qdsGame *restrict game,
						   unsigned int clearType)
{
	int lines = clearType & QDS_LINECLEAR_MAX;
	int twist = clearType >> 8 & 3;

	if (lines > 4) lines = 4;

	int score = dropScore[twist][lines];
	if (clearType & QDS_LINECLEAR_ALLCLEAR) score += allClearBonus[lines];
	if (clearType & QDS_LINECLEAR_B2B) score += score / 2;
	addLevelMultipliedScore(data, game, score);
}

static void addLockScore(qdsGame *restrict game)
{
	standardData *restrict data = qdsGetRulesetData(game);
	scoreLineClear(data, game, qdsCheckLockType(&data->baseState, game));

	if (data->baseState.pendingLines.lines > 0) {
		/* add combo bonus */
		addLevelMultipliedScore(data, game, 50 * data->combo++);
	} else {
		/* break combo */
		data->combo = 0;
	}
}

static void doActiveCycle(qdsRulesetState *restrict data,
						  qdsGame *restrict game,
						  unsigned int input)
{
	qdsProcessHold(data, game, input);
	/* baseState is the first element of standardData */
	doMovement((standardData *)data, game, input);
	doRotate((standardData *)data, game, input);
	doGravity((standardData *)data, game, input);
}

static void gameCycle(qdsGame *restrict game, unsigned int input)
{
	standardData *data = qdsGetRulesetData(game);

	unsigned int effective = input & ~(data->inputState.lastInput);
	effective |= qdsFilterDirections(game, &data->inputState, input)
				 & ~(QDS_INPUT_HARD_DROP);

	qdsRulesetCycle(&data->baseState, game, doActiveCycle, effective);

	data->time += 1;
}

static int spawnX(qdsGame *game)
{
	return 4;
}

static int spawnY(qdsGame *game)
{
	return 20;
}

static int peekNext(void *data, int pos)
{
	return qdsBagPeek(&((standardData *)data)->gen, pos);
}

static int drawNext(void *data)
{
	return qdsBagDraw(&((standardData *)data)->gen);
}

static void onTopOut(qdsGame *restrict game)
{
	((standardData *)qdsGetRulesetData(game))->baseState.status
		= QDS_STATUS_GAMEOVER;
}

static void onLineFilled(qdsGame *restrict game, int y)
{
	standardData *data = qdsGetRulesetData(game);
	qdsQueueLine(&data->baseState.pendingLines, y);
	data->lines += 1;
}

static int rulesetCall(qdsGame *restrict game, unsigned long call, void *argp)
{
	standardData *data = qdsGetRulesetData(game);

	switch (call) {
		case QDS_GETRULESETNAME:
			*(const char **)argp = "Standard";
			return 0;
		case QDS_GETTIME:
			*(unsigned int *)argp = data->time;
			return 0;
		case QDS_GETLINES:
			*(unsigned int *)argp = data->lines;
			return 0;
		case QDS_GETSCORE:
			*(unsigned int *)argp = data->score;
			return 0;
		case QDS_GETCOMBO:
			*(unsigned int *)argp = data->combo;
			return 0;
		case QDS_GETNEXTCOUNT:
			*(int *)argp = 7;
			return 0;
		case QDS_GETRESETS:
			*(int *)argp = 15;
			return 0;
		default:
			return qdsUtilCallHandler(&data->baseState, game, call, argp);
	}
}

QDS_API const qdsRuleset qdsRulesetStandard = {
	.init = init,
	.destroy = free,
	.spawnX = spawnX,
	.spawnY = spawnY,
	.getPiece = peekNext,
	.shiftPiece = drawNext,
	.getShape = getShape,
	.canRotate = canRotate,
	.doGameCycle = gameCycle,
	.events = {
		.onSpawn = onSpawn,
		.onDrop = onDrop,
		.onLineFilled = onLineFilled,
		.onTopOut = onTopOut,
		.postLock = addLockScore,
	},
	.call = rulesetCall,
};
