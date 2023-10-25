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
#include <errno.h>
#include <limits.h>
#include <mode.h>
#include <quadus.h>
#include <ruleset/linequeue.h>
#include <stdlib.h>
#include <time.h>

const struct phase *SHARED(phases)[] = {
	&SHARED(phaseMain),
	&SHARED(phaseCreditsFading),
	&SHARED(phaseCreditsInvisible),
	&SHARED(phaseGameOver),
};

static void *init(void)
{
	struct modeData *data = aligned_alloc(16, sizeof(struct modeData));
	if (!data) return NULL;

	data->time = 0;
	data->sectionTime = 0;
	data->lastCoolTime = SHARED(baseSectionCoolTime)[0];

	data->phase = PHASE_MAIN;
	data->level = 0;
	data->section = 0;
	data->cools = 0;

	data->grade = 0;
	data->gradePoints = 0;
	data->decayTimer = 0;
	data->combo = 0;

	data->speedIndex = 0;
	data->timingIndex = 0;

	data->creditsPoints = 0;

	data->lines = 0;
	data->areType = ARE_TYPE_NORMAL;
	data->held = false;
	data->cool = false;

	data->message = "";
	data->messageTime = 0;

	qdsTgmGenInit(&data->gen, time(NULL));

	return data;
}

static void cycle(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);

	if (data->messageTime > 0)
		--data->messageTime;
	else
		data->message = "";

	if (!SHARED(phases)[data->phase]->onCycle) return;
	SHARED(phases)[data->phase]->onCycle(game, data);
}

static bool onHold(qdsGame *game, int piece)
{
	struct modeData *data = qdsGetModeData(game);
	if (SHARED(phases)[data->phase]->onHold)
		SHARED(phases)[data->phase]->onHold(game, data);
	return true;
}

static bool onSpawn(qdsGame *game, int piece)
{
	struct modeData *data = qdsGetModeData(game);
	if (SHARED(phases)[data->phase]->onSpawn)
		return SHARED(phases)[data->phase]->onSpawn(game, data);
	return true;
}

static bool onLock(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);
	if (SHARED(phases)[data->phase]->onLock)
		SHARED(phases)[data->phase]->onLock(game, data);
	return true;
}

static void onLineFilled(qdsGame *game, int y)
{
	struct modeData *data = qdsGetModeData(game);
	if (SHARED(phases)[data->phase]->onLineFilled)
		SHARED(phases)[data->phase]->onLineFilled(game, data, y);
}

static void postLock(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);
	if (SHARED(phases)[data->phase]->postLock)
		SHARED(phases)[data->phase]->postLock(game, data);
}

static bool onLineClear(qdsGame *game, int y)
{
	struct modeData *data = qdsGetModeData(game);
	if (SHARED(phases)[data->phase]->onLineClear)
		SHARED(phases)[data->phase]->onLineClear(game, data, y);
	return true;
}

static void onTopOut(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);
	data->phase = PHASE_GAME_OVER;
}

uint_fast16_t SHARED(visible)(struct modeData *data, int y)
{
	return 0x03ff;
}

uint_fast16_t SHARED(invisible)(struct modeData *data, int y)
{
	return 0;
}

static int peek(const void *data, int pos)
{
	return qdsTgmGenPeek(&((struct modeData *)data)->gen, pos);
}

static int draw(void *data)
{
	return qdsTgmGenDraw(&((struct modeData *)data)->gen);
}

static int call(qdsGame *game, unsigned long req, void *argp)
{
	struct modeData *data = qdsGetModeData(game);

	int speed = SHARED(phases)[data->phase]->getSpeed(data);
	const struct timingData *timings
		= SHARED(phases)[data->phase]->getTimings(data);
	switch (req) {
		case QDS_GETMODENAME:
			*(const char **)argp = "Master";
			return 0;
		case QDS_GETTIME:
			*(int *)argp = data->time;
			return 0;
		case QDS_GETLEVEL:
			*(int *)argp = data->section + 1;
			return 0;
		case QDS_GETSUBLEVEL:
			*(int *)argp = data->level;
			return 0;
		case QDS_GETLEVELTARGET:
			if (SHARED(sectionThresholds)[data->section] == SHRT_MAX)
				*(int *)argp = 999;
			else
				*(int *)argp = SHARED(sectionThresholds)[data->section];
			return 0;
		case QDS_GETGRADE:
			*(int *)argp = SHARED(getGrade)(data);
			return 0;
		case QDS_GETGRADETEXT:
			*(const char **)argp = SHARED(gradeNames)[SHARED(getGrade)(data)];
			return 0;
		case QDS_GETMESSAGE:
			*(const char **)argp = data->message;
			return 0;
		case QDS_GETARE:
			if (data->areType == ARE_TYPE_LINE)
				*(int *)argp = timings->lineAre;
			else
				*(int *)argp = timings->are;
			return 0;
		case QDS_GETLINEDELAY:
			*(int *)argp = timings->lineClear;
			return 0;
		case QDS_GETDAS:
			*(int *)argp = timings->das;
			return 0;
		case QDS_GETARR:
			*(int *)argp = 1;
			return 0;
		case QDS_GETDCD:
			*(int *)argp = 1;
			return 0;
		case QDS_GETGRAVITY:
			*(int *)argp = speed;
			return 0;
		case QDS_GETSDG:
			*(int *)argp = speed > 65536 ? speed : 65536;
			return 0;
		case QDS_GETLOCKTIME:
			*(int *)argp = timings->lock;
			return 0;
		case QDS_GETCOMBO:
			*(int *)argp = data->combo;
			return 0;
		case QDS_GETNEXTCOUNT:
			*(int *)argp = 3;
			return 0;
		case QDS_GETVISIBILITY:
			*(uint_fast16_t *)argp = SHARED(phases)[data->phase]->getVisibility(
				data, *(uint_fast16_t *)argp);
			return 0;
		case QDS_SHOWGHOST:
			*(bool *)argp = data->level < 100;
			return 0;
	}
	return -ENOTTY;
}

const qdsGamemode qdsModeMaster = {
	.init = init,
	.destroy = free,
	.events = {
		.onCycle = cycle,
		.onSpawn = onSpawn,
		.onHold = onHold,
		.onLock = onLock,
		.onLineFilled = onLineFilled,
		.postLock = postLock,
		.onLineClear = onLineClear,
		.onTopOut = onTopOut,
	},
	.getPiece = peek,
	.shiftPiece = draw,
	.call = call,
};
