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
#include <errno.h>
#include <quadus.h>
#include <quadus/calls.h>
#include <quadus/ruleset.h>
#include <quadus/ruleset/input.h>
#include <quadus/ruleset/utils.h>
#include <string.h>

QDS_API void qdsInitRulesetState(qdsRulesetState *state)
{
	state->status = QDS_STATUS_INIT;
	state->statusTime = 1;
	state->subY = 0;
	state->lockTimer = 30;
	state->resetsLeft = 15;
	state->delayInput = 0;
	state->twistCheckResult = 0;
	state->clearType = 0;
	memset(&state->pendingLines, 0, sizeof(state->pendingLines));
	state->held = false;
	state->b2b = false;
	state->reset = false;
	state->pause = false;
	state->spawning = false;
}

QDS_API void qdsProcessSpawn(qdsRulesetState *restrict state,
							 qdsGame *restrict game,
							 unsigned int input)
{
	state->status = QDS_STATUS_ACTIVE;
	state->delayInput = 0;
	state->held = false;

	state->spawning = true;
	qdsSpawn(game, 0);

	qdsProcessHold(state, game, input);
	qdsProcessRotation(state, game, input);
	if (qdsOverlaps(game)) qdsEndGame(game);

	state->spawning = false;

	qdsProcessGravity(state, game, input & QDS_INPUT_SOFT_DROP);
}

QDS_API void qdsHandleSpawn(qdsRulesetState *restrict state)
{
	state->subY = 0;
	state->twistCheckResult = 0;
}

QDS_API void qdsProcessHold(qdsRulesetState *restrict data,
							qdsGame *restrict game,
							unsigned int input)
{
	if (!(input & QDS_INPUT_HOLD)) return;

	if (data->held && (qdsCall(game, QDS_GETINFINIHOLD, NULL) <= 0)) return;
	qdsHold(game);
	data->held = true;
}

QDS_API int qdsProcessMovement(qdsRulesetState *restrict data,
							   qdsGame *restrict game,
							   unsigned int input)
{
	int direction;

	if (input & QDS_INPUT_LEFT) {
		direction = -1;
	} else if (input & QDS_INPUT_RIGHT) {
		direction = 1;
	} else {
		return 0;
	}

	int distance = qdsMove(game, direction);
	if (distance) data->twistCheckResult = 0;
	return distance;
}

QDS_API int qdsProcessRotation(qdsRulesetState *restrict data,
							   qdsGame *restrict game,
							   unsigned int input)
{
	int rotation;
	if (input & QDS_INPUT_ROTATE_C) {
		rotation = QDS_ROTATION_CLOCKWISE;
	} else if (input & QDS_INPUT_ROTATE_CC) {
		rotation = QDS_ROTATION_COUNTERCLOCKWISE;
	} else {
		return QDS_ROTATE_FAILED;
	}

	int rotateResult = qdsRotate(game, rotation);
	if (rotateResult != QDS_ROTATE_FAILED)
		data->twistCheckResult = rotateResult;
	return rotateResult;
}

QDS_API void qdsProcessGravity(qdsRulesetState *restrict state,
							   qdsGame *restrict game,
							   unsigned int input)
{
	if (qdsGrounded(game)) {
		state->subY = 0;
		if (--state->lockTimer == 0) return qdsProcessLock(state, game);
	} else {
		int gravity, dropType = QDS_DROP_GRAVITY;
		if (qdsCall(game, QDS_GETGRAVITY, &gravity) < 0) gravity = 65536 / 60;
		if (input & QDS_INPUT_SOFT_DROP) {
			int softDropGravity;
			if (qdsCall(game, QDS_GETSDG, &softDropGravity) < 0)
				softDropGravity = 32768;
			if (softDropGravity > gravity) {
				gravity = softDropGravity;
				dropType = QDS_DROP_SOFT;
			}
		} else {
			dropType = QDS_DROP_GRAVITY;
		}

		state->subY += gravity;
		qdsDrop(game, dropType, state->subY / 65536);
		if (qdsGrounded(game))
			state->subY = 0;
		else
			state->subY %= 65536;
	}
}

QDS_API void qdsProcessLock(qdsRulesetState *restrict state,
							qdsGame *restrict game)
{
	/* save active piece data for QDS_GETCLEARTYPE use */
	state->clearType = qdsGetActivePieceType(game) << 16;

	/* the rest is normal lock course */
	if (!qdsLock(game)) return;

	int lines = state->pendingLines.lines;
	unsigned int delay;

	if (lines > 0) {
		/* go to line delay */
		if (qdsCall(game, QDS_GETLINEDELAY, &delay) < 0 || delay == 0)
			return qdsProcessLineClear(state, game, 0);
		state->status = QDS_STATUS_LINEDELAY;
		state->statusTime = delay;
	} else {
		/* go to lock delay */
		if (qdsCall(game, QDS_GETARE, &delay) < 0 || delay == 0) {
			return qdsProcessSpawn(state, game, 0);
		} else {
			state->status = QDS_STATUS_LOCKDELAY;
			state->statusTime = delay;
		}
	}
}

QDS_API unsigned int qdsCheckLockType(qdsRulesetState *restrict state,
									  qdsGame *restrict game)
{
	unsigned int clearType = state->clearType;
	clearType &= 0xffff0000;

	int lines = state->pendingLines.lines;
	clearType |= lines;

	if (state->twistCheckResult >= QDS_ROTATE_TWIST)
		clearType |= state->twistCheckResult << 8;

	if (lines > 0) {
		/* check for perfect clear */
		if (qdsGetFieldHeight(game) == lines)
			clearType |= QDS_LINECLEAR_ALLCLEAR;

		/* check for back-to-back */
		bool b2b = lines >= 4 || state->twistCheckResult >= QDS_ROTATE_TWIST;
		if (b2b && state->b2b) clearType |= QDS_LINECLEAR_B2B;
		state->b2b = b2b;
	}

	state->clearType = clearType;
	return clearType;
}

QDS_API void qdsProcessLineClear(qdsRulesetState *restrict state,
								 qdsGame *restrict game,
								 unsigned int input)
{
	qdsClearQueuedLines(game, &state->pendingLines);

	unsigned int are;
	if (qdsCall(game, QDS_GETARE, &are) < 0 || are == 0) {
		return qdsProcessSpawn(state, game, state->delayInput);
	} else {
		state->status = QDS_STATUS_LOCKDELAY;
		state->statusTime = are;
	}
}

QDS_API void qdsProcessDelay(qdsRulesetState *restrict state,
							 qdsGame *restrict game,
							 unsigned int input)
{
	/* initial rotation */
	if (input & (QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC)) {
		state->delayInput &= ~(QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC);
		state->delayInput |= input & (QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC);
	}

	/* initial hold */
	if (input & QDS_INPUT_HOLD) {
		/* allow initial hold to be cancelled */
		state->delayInput ^= QDS_INPUT_HOLD;
	}

	state->statusTime -= 1;
}

QDS_API void qdsRulesetCycle(qdsRulesetState *restrict state,
							 qdsGame *restrict game,
							 void (*activeCycle)(qdsRulesetState *restrict,
												 qdsGame *restrict,
												 unsigned int input),
							 unsigned int input)
{
	if (state->pause) {
		state->pause = false;
		state->status = QDS_STATUS_PAUSE;
	}

	switch (state->status) {
		case QDS_STATUS_ACTIVE:
			return activeCycle(state, game, input);
		case QDS_STATUS_INIT:
		case QDS_STATUS_PREGAME:
		case QDS_STATUS_PAUSE:
		case QDS_STATUS_LOCKDELAY:
			qdsProcessDelay(state, game, input);
			if (state->statusTime == 0) {
				qdsProcessSpawn(state, game, state->delayInput);
			}
			break;
		case QDS_STATUS_LINEDELAY:
			qdsProcessDelay(state, game, input);
			if (state->statusTime == 0) {
				qdsProcessLineClear(state, game, input);
			}
			break;
	}
}

QDS_API int qdsUtilCallHandler(qdsRulesetState *state,
							   qdsGame *game,
							   unsigned long call,
							   void *argp)
{
	switch (call) {
		case QDS_CANHOLD:
			return !state->held || qdsCall(game, QDS_GETINFINIHOLD, NULL) > 0;
		case QDS_GETCLEARTYPE:
			*(int *)argp = state->clearType;
			return 0;
		case QDS_PAUSE:
			state->pause = true;
			state->statusTime = *(int *)argp;
			return 0;
		default:
			return -ENOTTY;
	}
}
