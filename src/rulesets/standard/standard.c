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
#include <quadus.h>
#include <ruleset/standard.h>

#include <calls.h>
#include <piece.h>
#include <ruleset.h>
#include <ruleset/input.h>
#include <ruleset/linequeue.h>
#include <ruleset/piecegen.h>
#include <ruleset/rand.h>
#include <ruleset/twist.h>

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

static const qdsCoords kicksNormalClockwise[4][8] = {
	{ { 0, 0 }, { -1, 0 }, { -1, +1 }, { 0, -2 }, { -1, -2 }, { 127, 127 } },
	{ { 0, 0 }, { +1, 0 }, { +1, -1 }, { 0, +2 }, { +1, +2 }, { 127, 127 } },
	{ { 0, 0 }, { +1, 0 }, { +1, +1 }, { 0, -2 }, { +1, -2 }, { 127, 127 } },
	{ { 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, +2 }, { -1, +2 }, { 127, 127 } },
};

static const qdsCoords kicksNormalCounterClockwise[4][8] = {
	{ { 0, 0 }, { +1, 0 }, { +1, +1 }, { 0, -2 }, { +1, -2 }, { 127, 127 } },
	{ { 0, 0 }, { +1, 0 }, { +1, -1 }, { 0, +2 }, { +1, +2 }, { 127, 127 } },
	{ { 0, 0 }, { -1, 0 }, { -1, +1 }, { 0, -2 }, { -1, -2 }, { 127, 127 } },
	{ { 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, +2 }, { -1, +2 }, { 127, 127 } },
};

static const qdsCoords (*kicksNormal[2])[8] = {
	kicksNormalClockwise,
	kicksNormalCounterClockwise,
};

static const qdsCoords kicksIClockwise[4][8] = {
	{ { 0, 0 }, { -2, 0 }, { +1, 0 }, { -2, -1 }, { +1, +2 }, { 127, 127 } },
	{ { 0, 0 }, { -1, 0 }, { +2, 0 }, { -1, +2 }, { +2, -1 }, { 127, 127 } },
	{ { 0, 0 }, { +2, 0 }, { -1, 0 }, { +2, +1 }, { -1, -2 }, { 127, 127 } },
	{ { 0, 0 }, { +1, 0 }, { -2, 0 }, { +1, -2 }, { -2, +1 }, { 127, 127 } },
};

static const qdsCoords kicksICounterClockwise[4][8] = {
	{ { 0, 0 }, { -1, 0 }, { +2, 0 }, { -1, +2 }, { +2, -1 }, { 127, 127 } },
	{ { 0, 0 }, { +2, 0 }, { -1, 0 }, { +2, +1 }, { -1, -2 }, { 127, 127 } },
	{ { 0, 0 }, { +1, 0 }, { -2, 0 }, { +1, -2 }, { -2, +1 }, { 127, 127 } },
	{ { 0, 0 }, { -2, 0 }, { +1, 0 }, { -2, -1 }, { +1, +2 }, { 127, 127 } },
};

static const qdsCoords (*kicksI[2])[8] = {
	kicksIClockwise,
	kicksICounterClockwise,
};

static const struct
{
	const qdsPiecedef *shape;
	const qdsCoords (**kicks)[8];
} pieces[] = {
	[QDS_PIECE_NONE] = { &qdsPieceNone, kicksNormal },
	[QDS_PIECE_I] = { &qdsPieceI, kicksI },
	[QDS_PIECE_J] = { &qdsPieceJ, kicksNormal },
	[QDS_PIECE_L] = { &qdsPieceL, kicksNormal },
	[QDS_PIECE_O] = { &qdsPieceO, kicksNormal },
	[QDS_PIECE_S] = { &qdsPieceS, kicksNormal },
	[QDS_PIECE_T] = { &qdsPieceT, kicksNormal },
	[QDS_PIECE_Z] = { &qdsPieceZ, kicksNormal },
};

static const int dropScore[4][5] = {
	[0] = { 0, 100, 300, 500, 800 },
	[QDS_ROTATE_NORMAL] = { 0, 100, 300, 500, 800 },
	[QDS_ROTATE_TWIST] = { 400, 800, 1200, 1600, 2000 },
	[QDS_ROTATE_TWIST_MINI] = { 100, 200, 400, 600, 800 },
};

#define DEFAULT_LOCKTIME 30
#define DEFAULT_GRAVITY (65536 / 60)

/**
 * Run active cycle game logic.
 */
static void doActiveCycle(standardData *data, qdsGame *game, unsigned input);
/**
 * Run delay cycle game logic.
 */
static void doDelayCycle(standardData *data, qdsGame *game, unsigned input);

/**
 * Transition from delay to active state.
 */
static void spawnPiece(standardData *data, qdsGame *game, unsigned int input);
/**
 * End line delay, clear pending lines and transition to the next state.
 */
static void clearLines(standardData *data, qdsGame *game, unsigned int input);
/**
 * Process horizontal piece movement.
 */
static void doMovement(standardData *data, qdsGame *game, unsigned int input);
/**
 * Process piece rotation.
 */
static void doRotate(standardData *data, qdsGame *game, unsigned int input);
/**
 * Process piece holding.
 */
static void doHold(standardData *data, qdsGame *game, unsigned int input);
/**
 * Process gravity, soft dropping and hard dropping.
 */
static void doGravity(standardData *data, qdsGame *game, unsigned int input);
/**
 * Process locking.
 */
static void doLock(standardData *data, qdsGame *game);
/**
 * Reset lock timer.
 */
static void resetLock(standardData *data, qdsGame *game, bool refresh);
/**
 * Add to score a number of points multiplied by level.
 */
static int addLevelMultipliedScore(standardData *data,
								   qdsGame *game,
								   int points);

static void *init(void)
{
	standardData *data = malloc(sizeof(standardData));
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
	data->resetsLeft = 15;
	data->twistCheckResult = 0;
	data->held = false;
	data->b2b = false;

	data->inputState.lastInput = 0;
	data->inputState.direction = 0;

	qdsBagInit(&data->gen, time(NULL));

	data->pendingLines.lines = 0;

	return data;
}

static const qdsCoords *getShape(int type, int orientation)
{
	type %= 8;
	return ((const qdsCoords **)(pieces[type].shape))[orientation];
}

static int addLevelMultipliedScore(standardData *data, qdsGame *game, int pts)
{
	int level;
	if (qdsCall(game, QDS_GETLEVEL, &level) < 0) level = 1;
	data->score += pts * level;
	return data->score;
}

static void gameCycle(qdsGame *game, unsigned int input)
{
	standardData *data = qdsGetRulesetData(game);
	input = qdsFilterInput(game, &data->inputState, input);

	switch (data->status) {
		case STATUS_ACTIVE:
			doActiveCycle(data, game, input);
			break;
		case STATUS_PREGAME:
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

static void doActiveCycle(standardData *data, qdsGame *game, unsigned int input)
{
	doHold(data, game, input);
	doMovement(data, game, input);
	doRotate(data, game, input);
	doGravity(data, game, input);
}

static void doDelayCycle(standardData *data, qdsGame *game, unsigned int input)
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

static void spawnPiece(standardData *data, qdsGame *game, unsigned int input)
{
	data->status = STATUS_ACTIVE;
	data->delayInput = 0;
	data->held = false;

	qdsSpawn(game, 0);

	doHold(data, game, input);
	doRotate(data, game, input);
	doGravity(data, game, input & QDS_INPUT_SOFT_DROP);
}

static bool onSpawn(qdsGame *game, int _)
{
	standardData *data = qdsGetRulesetData(game);
	data->subY = 0;
	data->twistCheckResult = 0;
	resetLock(data, game, true);
	return true;
}

static void clearLines(standardData *data, qdsGame *game, unsigned int input)
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

static void doMovement(standardData *data, qdsGame *game, unsigned int input)
{
	if (input & QDS_INPUT_LEFT) {
		if (qdsGrounded(game)) resetLock(data, game, false);
		qdsMove(game, -1);
	} else if (input & QDS_INPUT_RIGHT) {
		if (qdsGrounded(game)) resetLock(data, game, false);
		qdsMove(game, 1);
	}

	data->twistCheckResult = 0;
}

static void doRotate(standardData *data, qdsGame *game, unsigned int input)
{
	int rotation;
	if (input & QDS_INPUT_ROTATE_C) {
		if (qdsGrounded(game)) resetLock(data, game, false);
		rotation = QDS_ROTATION_CLOCKWISE;
	} else if (input & QDS_INPUT_ROTATE_CC) {
		if (qdsGrounded(game)) resetLock(data, game, false);
		rotation = QDS_ROTATION_COUNTERCLOCKWISE;
	} else {
		return;
	}

	int twistCheckResult;
	if ((twistCheckResult = qdsRotate(game, rotation)) == QDS_ROTATE_FAILED)
		return;
	data->twistCheckResult = twistCheckResult;
}

static int canRotate(qdsGame *game, int rotation, int *x, int *y)
{
	if (rotation == 0) return QDS_ROTATE_FAILED;

	int piece = qdsGetActivePieceType(game) % 8;
	int orientation = qdsGetActiveOrientation(game);
	const qdsCoords *kicks;

	if (rotation > 0) {
		rotation = 1;
		kicks = pieces[piece].kicks[1][orientation];
	} else {
		rotation = -1;
		kicks = pieces[piece].kicks[0][orientation];
	}

	for (const qdsCoords *k = kicks; !(k->x == 127 && k->y == 127); ++k) {
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

static void doHold(standardData *data, qdsGame *game, unsigned int input)
{
	if (!(input & QDS_INPUT_HOLD)) return;

	bool infinihold;
	if (data->held
		&& (qdsCall(game, QDS_GETINFINIHOLD, &infinihold) < 0 || !infinihold))
		return;
	qdsHold(game);
	data->held = true;
}

static void doGravity(standardData *data, qdsGame *game, unsigned int input)
{
	if (input & QDS_INPUT_HARD_DROP) {
		qdsDrop(game, QDS_DROP_HARD, 48);
		return doLock(data, game);
	}

	if (qdsGrounded(game) && --data->lockTimer == 0) {
		return doLock(data, game);
	} else {
		int gravity, dropType;
		if (input & QDS_INPUT_SOFT_DROP) {
			if (qdsCall(game, QDS_GETSDG, &gravity) < 0) gravity = 32768;
			dropType = QDS_DROP_SOFT;
		} else {
			if (qdsCall(game, QDS_GETGRAVITY, &gravity) < 0)
				gravity = DEFAULT_GRAVITY;
			dropType = QDS_DROP_GRAVITY;
		}

		data->subY += gravity;
		qdsDrop(game, dropType, data->subY / 65536);
		if (qdsGrounded(game))
			data->subY = 0;
		else
			data->subY %= 65536;
	}
}

static bool onDrop(qdsGame *game, int type, int distance)
{
	standardData *data = qdsGetRulesetData(game);
	if (type == QDS_DROP_HARD) data->score += distance * 2;
	if (type == QDS_DROP_SOFT) data->score += distance;
	if (distance > 0) data->twistCheckResult = 0;
	return true;
}

static void doLock(standardData *data, qdsGame *game)
{
	if (!qdsLock(game)) return;

	int lines = data->pendingLines.lines > 4 ? 4 : data->pendingLines.lines;
	int points = dropScore[data->twistCheckResult][lines];
	data->b2b = lines >= 4 || data->twistCheckResult >= 2;
	if (data->b2b) points += points / 2;
	addLevelMultipliedScore(data, game, points);

	qdsInterruptRepeat(game, &data->inputState);

	unsigned int delay;
	if (lines > 0) {
		/* add combo bonus */
		addLevelMultipliedScore(data, game, 50 * data->combo++);
		/* enter line delay */
		if (qdsCall(game, QDS_GETLINEDELAY, &delay) < 0) delay = 30;
		if (delay == 0) return clearLines(data, game, 0);
		data->status = STATUS_LINEDELAY;
		data->statusTime = delay;
	} else {
		/* break combo */
		data->combo = 0;
		/* go to lock delay */
		if (qdsCall(game, QDS_GETARE, &delay) < 0 || delay == 0) {
			return spawnPiece(data, game, data->delayInput);
		} else {
			data->status = STATUS_LOCKDELAY;
			data->statusTime = delay;
		}
	}
}

static void resetLock(standardData *data, qdsGame *game, bool refresh)
{
	if (refresh) {
		int resets;
		if (qdsCall(game, QDS_GETRESETS, &resets) < 0) resets = 15;
		data->resetsLeft = resets;
	} else {
		if (data->resetsLeft == 0) return;
		data->resetsLeft -= 1;
	}

	int lockTime;
	if (qdsCall(game, QDS_GETLOCKTIME, &lockTime) < 0)
		lockTime = DEFAULT_LOCKTIME;
	data->lockTimer = lockTime;
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

static void onTopOut(qdsGame *game)
{
	((standardData *)qdsGetRulesetData(game))->status = STATUS_GAMEOVER;
}

static void onLineFilled(qdsGame *game, int y)
{
	standardData *data = qdsGetRulesetData(game);
	qdsQueueLine(&data->pendingLines, y);
	data->lines += 1;
}

static int rulesetCall(qdsGame *game, unsigned long call, void *argp)
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
		case QDS_GETGRAVITY:
			*(int *)argp = DEFAULT_GRAVITY;
			return 0;
		case QDS_GETDAS:
			*(int *)argp = QDS_DEFAULT_DAS;
			return 0;
		case QDS_GETARR:
			*(int *)argp = QDS_DEFAULT_ARR;
			return 0;
		case QDS_GETDCD:
			*(int *)argp = QDS_DEFAULT_DCD;
			return 0;
		case QDS_GETSDG:
			*(int *)argp = 32768;
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
			*(bool *)argp = false;
			return 0;
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
	.onSpawn = onSpawn,
	.onDrop = onDrop,
	.onLineFilled = onLineFilled,
	.onTopOut = onTopOut,
	.call = rulesetCall,
};
