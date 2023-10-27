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
#include "arcade.h"
#include <config.h>
#include <quadus.h>

#include <calls.h>
#include <piece.h>
#include <piecegen/tgm.h>
#include <ruleset.h>
#include <ruleset/input.h>
#include <ruleset/linequeue.h>
#include <ruleset/twist.h>

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_LOCKTIME 30
#define DEFAULT_GRAVITY (65536 / 60)

static const signed char kickOrderCw[] = { 1, -1, SCHAR_MAX };
static const signed char kickOrderCcw[] = { -1, 1, SCHAR_MAX };
static const signed char kickOrderCwI[] = { -1, SCHAR_MAX };
static const signed char kickOrderCcwI[] = { 1, SCHAR_MAX };

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

/**
 * Run active cycle game logic.
 */
static void doActiveCycle(arcadeData *data, qdsGame *game, unsigned input);
/**
 * Run delay cycle game logic.
 */
static void doDelayCycle(arcadeData *data, qdsGame *game, unsigned input);

/**
 * Transition from delay to active state.
 */
static void spawnPiece(arcadeData *data, qdsGame *game, unsigned int input);
/**
 * End line delay, clear pending lines and transition to the next state.
 */
static void clearLines(arcadeData *data, qdsGame *game, unsigned int input);
/**
 * Process horizontal piece movement.
 */
static void doMovement(arcadeData *data, qdsGame *game, unsigned int input);
/**
 * Process piece rotation.
 */
static void doRotate(arcadeData *data, qdsGame *game, unsigned int input);
/**
 * Process piece holding.
 */
static void doHold(arcadeData *data, qdsGame *game, unsigned int input);
/**
 * Process gravity, soft dropping and hard dropping.
 */
static void doGravity(arcadeData *data, qdsGame *game, unsigned int input);
/**
 * Process locking.
 */
static void doLock(arcadeData *data, qdsGame *game);
/**
 * Reset lock timer.
 */
static bool resetLock(arcadeData *data, qdsGame *game);

static void *init(void)
{
	arcadeData *data = malloc(sizeof(arcadeData));
	if (!data) return NULL;

	data->time = 0;
	data->lines = 0;
	data->score = 0;
	data->combo = 0;
	data->subY = 0;
	data->status = STATUS_PREGAME;
	data->statusTime = 1;
	data->delayInput = 0;
	data->lockTimer = 30;
	data->twistCheckResult = 0;
	data->clearType = 0;
	data->held = false;
	data->b2b = false;
	data->pause = false;

	data->inputState.lastInput = 0;
	data->inputState.direction = 0;

	qdsTgmGenInit(&data->gen, time(NULL));

	data->pendingLines.lines = 0;

	return data;
}

static const qdsCoords *getShape(int type, int orientation)
{
	type %= 8;
	return ((const qdsCoords **)(pieces[type].piece))[orientation];
}

static void gameCycle(qdsGame *restrict game, unsigned int input)
{
	arcadeData *data = qdsGetRulesetData(game);
	input = qdsFilterInput(game, &data->inputState, input);

	if (data->pause) {
		data->pause = false;
		data->status = STATUS_PAUSE;
	}

	switch (data->status) {
		case STATUS_ACTIVE:
			doActiveCycle(data, game, input);
			break;
		case STATUS_PREGAME:
		case STATUS_PAUSE:
		case STATUS_LOCKDELAY:
			doDelayCycle(data, game, input);
			if (data->statusTime == 0) {
				spawnPiece(data, game, data->delayInput);
			}
			break;
		case STATUS_LINEDELAY:
			doDelayCycle(data, game, input);
			if (data->statusTime == 0) {
				clearLines(data, game, input);
			}
			break;
		default: /* game over or invalid state */
			return;
	}

	data->time += 1;
}

static void doActiveCycle(arcadeData *restrict data,
						  qdsGame *restrict game,
						  unsigned int input)
{
	doHold(data, game, input);
	doRotate(data, game, input);
	doMovement(data, game, input);
	doGravity(data, game, input);
}

static void doDelayCycle(arcadeData *restrict data,
						 qdsGame *restrict game,
						 unsigned int input)
{
	/* initial rotation */
	if (input & (QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC)) {
		data->delayInput &= ~(QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC);
		data->delayInput |= input & (QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC);
	}

	/* initial hold */
	if (input & QDS_INPUT_HOLD) {
		/* allow initial hold to be cancelled */
		data->delayInput ^= QDS_INPUT_HOLD;
	}

	data->statusTime -= 1;
}

static void spawnPiece(arcadeData *restrict data,
					   qdsGame *restrict game,
					   unsigned int input)
{
	data->status = STATUS_ACTIVE;
	data->delayInput = 0;
	data->held = false;

	qdsSpawn(game, 0);

	doHold(data, game, input);
	doRotate(data, game, input);

	if (qdsOverlaps(game)) qdsEndGame(game);

	doGravity(data, game, input & QDS_INPUT_SOFT_DROP);
}

static bool onSpawn(qdsGame *restrict game, int piece)
{
	arcadeData *data = qdsGetRulesetData(game);
	data->subY = 0;
	data->twistCheckResult = 0;
	resetLock(data, game);
	return true;
}

static void clearLines(arcadeData *restrict data,
					   qdsGame *restrict game,
					   unsigned int input)
{
	qdsClearQueuedLines(game, &data->pendingLines);

	unsigned int are;
	if (qdsCall(game, QDS_GETARE, &are) < 0 || are == 0) {
		return spawnPiece(data, game, data->delayInput);
	} else {
		data->status = STATUS_LOCKDELAY;
		data->statusTime = are;
	}
}

static void doMovement(arcadeData *restrict data,
					   qdsGame *restrict game,
					   unsigned int input)
{
	int direction;

	if (input & QDS_INPUT_LEFT) {
		direction = -1;
	} else if (input & QDS_INPUT_RIGHT) {
		direction = 1;
	} else {
		return;
	}

	if (!qdsMove(game, direction)) return;
	data->twistCheckResult = 0;
}

static void doRotate(arcadeData *restrict data,
					 qdsGame *restrict game,
					 unsigned int input)
{
	int rotation;
	if (input & QDS_INPUT_ROTATE_C) {
		rotation = QDS_ROTATION_CLOCKWISE;
	} else if (input & QDS_INPUT_ROTATE_CC) {
		rotation = QDS_ROTATION_COUNTERCLOCKWISE;
	} else {
		return;
	}

	int rotateResult = qdsRotate(game, rotation);
	if (rotateResult == QDS_ROTATE_FAILED) return;
	data->twistCheckResult = rotateResult;
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
		case QDS_PIECE_S:
		case QDS_PIECE_T:
		case QDS_PIECE_Z:
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

static void doHold(arcadeData *restrict data,
				   qdsGame *restrict game,
				   unsigned int input)
{
	if (!(input & QDS_INPUT_HOLD)) return;

	if (data->held && (qdsCall(game, QDS_GETINFINIHOLD, NULL) <= 0)) return;
	qdsHold(game);
	data->held = true;
}

static void doGravity(arcadeData *restrict data,
					  qdsGame *restrict game,
					  unsigned int input)
{
	if (qdsGrounded(game)) {
		if (input & QDS_INPUT_SOFT_DROP || --data->lockTimer <= 0)
			return doLock(data, game);
	} else {
		int gravity, dropType;
		if (qdsCall(game, QDS_GETGRAVITY, &gravity) < 0)
			gravity = DEFAULT_GRAVITY;

		if (input & QDS_INPUT_SOFT_DROP) {
			if (qdsCall(game, QDS_GETSDG, &gravity) < 0 && gravity < 65536)
				gravity = 65536;
			dropType = QDS_DROP_SOFT;
		} else {
			dropType = QDS_DROP_GRAVITY;
		}

		data->subY += gravity;
		qdsDrop(game, dropType, data->subY / 65536);
		if (qdsGrounded(game))
			data->subY = 0;
		else
			data->subY %= 65536;

		resetLock(data, game);
	}

	if (input & QDS_INPUT_HARD_DROP) {
		qdsDrop(game, QDS_DROP_HARD, 48);
	}
}

static bool onDrop(qdsGame *restrict game, int type, int distance)
{
	arcadeData *data = qdsGetRulesetData(game);
	if (distance > 0) data->twistCheckResult = 0;
	return true;
}

static void addLockScore(qdsGame *restrict game)
{
	arcadeData *restrict data = qdsGetRulesetData(game);

	unsigned int clearType = data->clearType;

	int lines = data->pendingLines.lines;
	clearType |= lines;

	if (data->twistCheckResult >= QDS_ROTATE_TWIST)
		clearType |= data->twistCheckResult << 8;

	if (lines > 0) {
		/* check for perfect clear */
		if (qdsGetFieldHeight(game) == lines)
			clearType |= QDS_LINECLEAR_ALLCLEAR;

		/* check for back-to-back */
		bool b2b = lines >= 4 || data->twistCheckResult >= QDS_ROTATE_TWIST;
		if (b2b && data->b2b) clearType |= QDS_LINECLEAR_B2B;
		data->b2b = b2b;

		if (lines >= 2) ++data->combo;
	} else {
		/* break combo */
		data->combo = 0;
	}

	data->clearType = clearType;
}

static void doLock(arcadeData *restrict data, qdsGame *restrict game)
{
	/* save active piece data for QDS_GETCLEARTYPE use */
	data->clearType = qdsGetActivePieceType(game) << 16;

	/* the rest is normal lock course */
	if (!qdsLock(game)) return;
	qdsInterruptRepeat(game, &data->inputState);

	int lines = data->pendingLines.lines;
	unsigned int delay;

	if (lines > 0) {
		/* go to line delay */
		if (qdsCall(game, QDS_GETLINEDELAY, &delay) < 0 || delay == 0)
			return clearLines(data, game, 0);
		data->status = STATUS_LINEDELAY;
		data->statusTime = delay;
	} else {
		/* go to lock delay */
		if (qdsCall(game, QDS_GETARE, &delay) < 0 || delay == 0) {
			return spawnPiece(data, game, data->delayInput);
		} else {
			data->status = STATUS_LOCKDELAY;
			data->statusTime = delay;
		}
	}
}

static bool resetLock(arcadeData *restrict data, qdsGame *restrict game)
{
	int lockTime;
	if (qdsCall(game, QDS_GETLOCKTIME, &lockTime) < 0)
		lockTime = DEFAULT_LOCKTIME;
	data->lockTimer = lockTime;
	return true;
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
	((arcadeData *)qdsGetRulesetData(game))->status = STATUS_GAMEOVER;
}

static void onLineFilled(qdsGame *restrict game, int y)
{
	arcadeData *data = qdsGetRulesetData(game);
	qdsQueueLine(&data->pendingLines, y);
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
		case QDS_CANHOLD:
			return !data->held || qdsCall(game, QDS_GETINFINIHOLD, NULL) > 0;
		case QDS_GETCLEARTYPE:
			*(int *)argp = data->clearType;
			return 0;
		case QDS_PAUSE:
			data->pause = true;
			data->statusTime = *(int *)argp;
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
