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
#include "master.h"
#include <calls.h>
#include <mode.h>
#include <quadus.h>

#include <limits.h>
#include <stdint.h>

static void cycle(qdsGame *game, struct modeData *data)
{
	data->time += 1;
	data->sectionTime += 1;
}

static int effectiveLevel(struct modeData *data)
{
	return data->level + data->cools * 100;
}

static int addLevel(struct modeData *data, int level, bool nextSection)
{
	/* section cool check */
	if (data->level < SHARED(sectionCoolThresholds)[data->section]
		&& data->level + level
			   >= SHARED(sectionCoolThresholds)[data->section]) {
		if (data->sectionTime <= data->lastCoolTime + TIME(0, 2)) {
			data->cool = true;
			data->lastCoolTime = data->sectionTime;
			data->message = "Cool!";
			data->messageTime = TIME(0, 3);
		} else {
			data->cool = false;
			data->lastCoolTime = SHARED(baseSectionCoolTime)[data->section + 1];
		}
	}

	if (data->level + level > 999) {
		data->level = 999;
	} else if (data->level + level
			   >= SHARED(sectionThresholds)[data->section]) {
		if (!nextSection)
			data->level = SHARED(sectionThresholds)[data->section] - 1;
		else {
			data->level += level;
			data->sectionTime = 0;

			/* apply section cool */
			if (data->cool) ++data->cools;
			data->cool = false;
		};
	} else {
		data->level += level;
	}

	while (effectiveLevel(data)
		   >= SHARED(speedData)[data->speedIndex + 1].level)
		data->speedIndex += 1;
	while (effectiveLevel(data)
		   >= SHARED(timingData)[data->timingIndex + 1].level)
		data->timingIndex += 1;
	while (data->level >= SHARED(sectionThresholds)[data->section])
		data->section += 1;

	return data->level;
}

static void onHold(qdsGame *game, struct modeData *data)
{
	data->held = true;
}

static bool onSpawn(qdsGame *game, struct modeData *data)
{
	/* check for main mode end condition */
	if (data->level >= 999) {
		int time = TIME_CREDITS_TRANSITION;
		qdsClearPlayfield(game);
		qdsCall(game, QDS_PAUSE, &time);

		if (data->cools >= 9)
			data->phase = PHASE_CREDITS_INVISIBLE;
		else
			data->phase = PHASE_CREDITS_FADE;

		data->sectionTime = 0;
		return false;
	}

	/* normal spawn sequence */
	data->areType = ARE_TYPE_NORMAL;
	if (!data->held) addLevel(data, 1, false);
	data->held = false;
	return true;
}

static void onLineFilled(qdsGame *game, struct modeData *data, int y)
{
	data->lines += 1;
}

static void postLock(qdsGame *game, struct modeData *data)
{
	addLevel(data, SHARED(levelAdvance)[data->lines], true);
	data->lines = 0;
}

static int getSpeed(struct modeData *data)
{
	return SHARED(speedData)[data->speedIndex].gravity;
}

static const struct timingData *getTimings(struct modeData *data)
{
	return &SHARED(timingData)[data->timingIndex];
}

const struct phase SHARED(phaseMain) = {
	.getSpeed = getSpeed,
	.getTimings = getTimings,
	.getVisibility = SHARED(visible),
	.onCycle = cycle,
	.onSpawn = onSpawn,
	.onHold = onHold,
	.onLineFilled = onLineFilled,
	.postLock = postLock,
};
