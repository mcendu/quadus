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
#include "rulesets/arcade.h"
#include <config.h>
#include <quadus.h>
#include <quadus/calls.h>
#include <quadus/piece.h>
#include <quadus/piecegen/tgm.h>
#include <quadus/ruleset.h>
#include <quadus/ruleset/input.h>
#include <quadus/ruleset/linequeue.h>
#include <quadus/ruleset/twist.h>

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_LOCKTIME 30
#define DEFAULT_GRAVITY (65536 / 60)

static const signed char kickOrderCw[] = { 1, -1, SCHAR_MAX };
static const signed char kickOrderCcw[] = { -1, 1, SCHAR_MAX };
static const signed char kickOrderCwI[] = { 1, SCHAR_MAX };
static const signed char kickOrderCcwI[] = { -1, SCHAR_MAX };

static const struct
{
	const qdsPiecedef *piece;
	const signed char *kickOrderCw;
	const signed char *kickOrderCcw;
} pieces[] = {
	{ &qdsPieceNone, kickOrderCw, kickOrderCcw },
	{ &RSSYM(pieceI), kickOrderCwI, kickOrderCcwI },
	{ &RSSYM(pieceJ), kickOrderCw, kickOrderCcw },
	{ &RSSYM(pieceL), kickOrderCw, kickOrderCcw },
	{ &RSSYM(pieceO), kickOrderCw, kickOrderCcw },
	{ &RSSYM(pieceS), kickOrderCw, kickOrderCcw },
	{ &RSSYM(pieceT), kickOrderCw, kickOrderCcw },
	{ &RSSYM(pieceZ), kickOrderCw, kickOrderCcw },
};

static void *init(void)
{
	arcadeData *data = malloc(sizeof(arcadeData));
	if (!data) return NULL;

	qdsInitRulesetState(&data->baseState);

	data->time = 0;
	data->lines = 0;
	data->score = 0;
	data->combo = 0;

	data->inputState.lastInput = 0;
	data->inputState.direction = 0;

	qdsTgmGenInit(&data->gen, time(NULL));

	return data;
}

static int checkTwist(qdsGame *restrict game, int rotation, int x, int y)
{
	if (qdsCheckTwistImmobile(game, x, y, rotation))
		return QDS_ROTATE_TWIST;
	else
		return QDS_ROTATE_NORMAL;
};

static int canRotate(qdsGame *restrict game,
					 int rotation,
					 int *restrict x,
					 int *restrict y)
{
	if (rotation == 0) return QDS_ROTATE_FAILED;

	int piece = qdsGetActivePieceType(game) % 8;
	const signed char *kicks;

	if (rotation > 0) {
		rotation = 1;
		kicks = pieces[piece].kickOrderCw;
	} else {
		rotation = -1;
		kicks = pieces[piece].kickOrderCcw;
	}

	*y = 0;

	if (qdsCanRotate(game, 0, 0, rotation)) {
		*x = 0;
		return checkTwist(game, rotation, *x, *y);
	}

	int cx, cy;
	switch (piece) {
		case QDS_PIECE_O:
			return QDS_ROTATE_FAILED;
		case QDS_PIECE_J:
		case QDS_PIECE_L:
		case QDS_PIECE_T:
			/* center-column rule */
			qdsGetActivePosition(game, &cx, &cy);
			for (int dy = 2; dy >= 0; --dy)
				for (int dx = -1; dx <= 1; ++dx)
					if (qdsGetTile(game, cx + dx, cy + dy)) {
						if (dx == 0)
							return QDS_ROTATE_FAILED;
						else
							goto kick;
					}
	}

kick:
	for (const signed char *k = kicks; *k != SCHAR_MAX; ++k) {
		if (qdsCanRotate(game, *k, 0, rotation)) {
			*x = *k;
			return checkTwist(game, rotation, *x, *y);
		}
	}

	return QDS_ROTATE_FAILED;
}

static bool resetLock(arcadeData *restrict data, qdsGame *restrict game)
{
	int lockTime;
	if (qdsCall(game, QDS_GETLOCKTIME, &lockTime) < 0)
		lockTime = DEFAULT_LOCKTIME;
	data->baseState.lockTimer = lockTime;
	return true;
}

static const qdsCoords *getShape(int type, int orientation)
{
	type %= 8;
	return ((const qdsCoords **)(pieces[type].piece))[orientation];
}

static bool onSpawn(qdsGame *restrict game, int piece)
{
	arcadeData *data = qdsGetRulesetData(game);
	qdsHandleSpawn(&data->baseState);
	return true;
}

static void doGravity(arcadeData *restrict data,
					  qdsGame *restrict game,
					  unsigned int input)
{
	if (input & QDS_INPUT_HARD_DROP) {
		qdsDrop(game, QDS_DROP_HARD, 48);
	}

	if (qdsGrounded(game) && (input & QDS_INPUT_SOFT_DROP)) {
		return qdsProcessLock(&data->baseState, game);
	}

	return qdsProcessGravity(&data->baseState, game, input);
}

static bool onDrop(qdsGame *game, int type, int dy)
{
	if (dy > 0) resetLock(qdsGetRulesetData(game), game);
	return true;
}

static void addLockScore(qdsGame *restrict game)
{
	arcadeData *restrict data = qdsGetRulesetData(game);
	qdsCheckLockType(&data->baseState, game);

	if (data->baseState.pendingLines.lines > 0) {

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
	qdsProcessRotation(data, game, input);
	qdsProcessMovement(data, game, input);
	doGravity((arcadeData *)data, game, input);
}

static void gameCycle(qdsGame *restrict game, unsigned int input)
{
	arcadeData *data = qdsGetRulesetData(game);

	unsigned int effective
		= qdsFilterDirections(game, &data->inputState, input);

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
	return qdsTgmGenPeek(&((arcadeData *)data)->gen, pos);
}

static int drawNext(void *data)
{
	return qdsTgmGenDraw(&((arcadeData *)data)->gen);
}

static void onTopOut(qdsGame *restrict game)
{
	((arcadeData *)qdsGetRulesetData(game))->baseState.status
		= QDS_STATUS_GAMEOVER;
}

static void onLineFilled(qdsGame *restrict game, int y)
{
	arcadeData *data = qdsGetRulesetData(game);
	qdsQueueLine(&data->baseState.pendingLines, y);
	data->lines += 1;
}

static int getSoftDropGravity(qdsGame *game, int *result)
{
	int sdf, g;
	if (qdsCall(game, QDS_GETSDF, &sdf) < 0) return -ENOTTY;
	if (qdsCall(game, QDS_GETGRAVITY, &g) < 0) g = DEFAULT_GRAVITY;

	*result = g * sdf;
	return 0;
}

static int rulesetCall(qdsGame *restrict game, unsigned long call, void *argp)
{
	arcadeData *data = qdsGetRulesetData(game);

	int baseCallReturn = qdsUtilCallHandler(&data->baseState, game, call, argp);
	if (baseCallReturn >= 0) return baseCallReturn;

	switch (call) {
		case QDS_GETRULESETNAME:
			*(const char **)argp = "Arcade";
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
		case QDS_GETGRAVITY:
			*(int *)argp = DEFAULT_GRAVITY;
			return 0;
		case QDS_GETDCD:
			*(int *)argp = QDS_DEFAULT_DCD;
			return 0;
		case QDS_GETSDG:
			return getSoftDropGravity(game, argp);
		case QDS_GETARR:
			*(unsigned int *)argp = 1;
			return 0;
		case QDS_GETARE:
			*(unsigned int *)argp = 0;
			return 0;
		case QDS_GETLINEDELAY:
			*(unsigned int *)argp = 30;
			return 0;
		case QDS_GETNEXTCOUNT:
			*(int *)argp = 7;
			return 0;
		case QDS_GETINFINIHOLD:
			return false;
		case QDS_GETLOCKTIME:
			*(int *)argp = 30;
			return 0;
		case QDS_GETRESETS:
			*(int *)argp = 15;
			return 0;
		default:
			return -ENOTTY;
	}
}

QDS_API const qdsRuleset qdsRulesetArcade = {
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
		.postLock = addLockScore,
		.onTopOut = onTopOut,
	},
	.call = rulesetCall,
};
