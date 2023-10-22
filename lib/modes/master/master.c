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

static const struct speedData
{
	int level;
	int gravity;
} speedData[] = {
	{ 0, 1024 },	 { 30, 1536 },	   { 35, 2048 },		 { 40, 2560 },
	{ 50, 3072 },	 { 60, 4096 },	   { 70, 8192 },		 { 80, 12288 },
	{ 90, 16384 },	 { 100, 20480 },   { 120, 24576 },		 { 140, 28672 },
	{ 160, 32768 },	 { 170, 36864 },   { 200, 1024 },		 { 220, 8192 },
	{ 230, 16384 },	 { 233, 24576 },   { 236, 32768 },		 { 239, 40960 },
	{ 243, 49152 },	 { 247, 57344 },   { 251, 65536 },		 { 300, 131072 },
	{ 330, 196608 }, { 360, 262144 },  { 400, 327680 },		 { 420, 262144 },
	{ 450, 196608 }, { 500, 2097152 }, { INT_MAX, INT_MAX },
};

static const struct timingData
{
	short level;
	short das;
	short lock;
	short lineClear;
	short are;
	short lineAre;
} timingData[] = {
	{ 0, 16, 30, 40, 27, 27 },	 { 500, 10, 30, 25, 27, 27 },
	{ 600, 10, 30, 16, 27, 18 }, { 700, 10, 30, 12, 18, 14 },
	{ 800, 10, 30, 6, 14, 8 },	 { 900, 8, 17, 6, 14, 8 },
	{ 1000, 8, 17, 6, 8, 8 },	 { 1100, 8, 15, 6, 7, 7 },
	{ 1200, 8, 15, 6, 6, 6 },	 { SHRT_MAX, 1, 1, 1, 1, 1 },
};

static const short sectionThresholds[]
	= { 100, 200, 300, 400, 500, 600, 700, 800, 900, 999, SHRT_MAX };
static const short sectionCoolThresholds[]
	= { 70, 170, 270, 370, 470, 570, 670, 770, 870, 970, SHRT_MAX };

static const int baseSectionCoolTime[] = {
	TIME(0, 50), TIME(0, 50), TIME(0, 47), TIME(0, 43),
	TIME(0, 43), TIME(0, 40), TIME(0, 40), TIME(0, 36),
	TIME(0, 36), 0,			  0,
};

static const int levelAdvance[] = { 0, 1, 2, 4, 6 };

static void *init(void)
{
	struct modeData *data = malloc(sizeof(struct modeData));
	if (!data) return NULL;

	data->phase = PHASE_MAIN;
	data->level = 0;
	data->section = 0;
	data->cools = 0;
	data->sectionTime = 0;
	data->lastCoolTime = baseSectionCoolTime[0];
	data->areType = ARE_TYPE_NORMAL;
	data->held = false;
	data->cool = false;
	data->speedIndex = 0;
	data->timingIndex = 0;
	data->lines = 0;
	return data;
}

static void cycle(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);

	if (data->phase == PHASE_MAIN) {
		data->time += 1;
		data->sectionTime += 1;
	}
}

static int effectiveLevel(struct modeData *data)
{
	return data->level + data->cools * 100;
}

static int addLevel(struct modeData *data, int level, bool nextSection)
{
	if (data->level + level > 999) {
		data->level = 999;
	} else if (data->level + level >= sectionThresholds[data->section]) {
		if (!nextSection)
			data->level = sectionThresholds[data->section] - 1;
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

	/* section cool check */
	if (data->level < sectionCoolThresholds[data->section]
		&& data->level + level >= sectionCoolThresholds[data->section]) {
		if (data->sectionTime <= data->lastCoolTime + TIME(0, 2)) {
			data->cool = true;
			data->lastCoolTime = data->sectionTime;
		} else {
			data->cool = false;
			data->lastCoolTime = baseSectionCoolTime[data->section + 1];
		}
	}

	while (effectiveLevel(data) >= speedData[data->speedIndex + 1].level)
		data->speedIndex += 1;
	while (effectiveLevel(data) >= timingData[data->timingIndex + 1].level)
		data->timingIndex += 1;
	while (data->level >= sectionThresholds[data->section]) data->section += 1;

	return data->level;
}

static bool onHold(qdsGame *game, int piece)
{
	struct modeData *data = qdsGetModeData(game);
	data->held = true;
	return true;
}

static bool onSpawn(qdsGame *game, int piece)
{
	struct modeData *data = qdsGetModeData(game);

	/* check for main mode end condition */
	if (data->level >= 999) qdsEndGame(game);

	/* normal spawn sequence */
	data->areType = ARE_TYPE_NORMAL;
	if (!data->held) addLevel(data, 1, false);
	data->held = false;
	return true;
}

static void onLineFilled(qdsGame *game, int y)
{
	((struct modeData *)qdsGetModeData(game))->lines += 1;
}

static void postLock(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);
	addLevel(data, levelAdvance[data->lines], true);
	data->lines = 0;
}

static bool onLineClear(qdsGame *game, int y)
{
	struct modeData *data = qdsGetModeData(game);
	data->areType = ARE_TYPE_LINE;
	return true;
}

static void onTopOut(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);
	data->phase = PHASE_GAME_OVER;
}

static int call(qdsGame *game, unsigned long req, void *argp)
{
	struct modeData *data = qdsGetModeData(game);
	int speed = speedData[data->speedIndex].gravity;
	const struct timingData *timings = &timingData[data->timingIndex];
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
			if (sectionThresholds[data->section] == SHRT_MAX)
				*(int *)argp = 999;
			else
				*(int *)argp = sectionThresholds[data->section];
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
		case QDS_GETNEXTCOUNT:
			*(int *)argp = 3;
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
		.onLineFilled = onLineFilled,
		.postLock = postLock,
		.onLineClear = onLineClear,
		.onTopOut = onTopOut,
	},
	.call = call,
};
