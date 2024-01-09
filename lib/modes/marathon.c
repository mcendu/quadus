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
#include "modes/marathon.h"
#include <config.h>
#include <quadus.h>
#include <quadus/calls.h>
#include <quadus/mode.h>
#include <quadus/piecegen/quadus.h>

#include <errno.h>
#include <stdlib.h>
#include <time.h>

struct levelData
{
	unsigned int gravity;
	unsigned char lineDelay;
	unsigned char lockTime;
	unsigned char das;
};

static const struct levelData speedCurve[] = {
	/* 1..10 */
	{ 1092, 30, 30, 12 },
	{ 1377, 30, 30, 12 },
	{ 1768, 30, 30, 12 },
	{ 2310, 30, 30, 12 },
	{ 3075, 30, 30, 12 },
	{ 4168, 30, 30, 12 },
	{ 5758, 30, 30, 12 },
	{ 8106, 30, 30, 12 },
	{ 11634, 30, 30, 12 },
	{ 17026, 30, 30, 12 },
	/* 11..20 */
	{ 25415, 30, 30, 12 },
	{ 38708, 30, 30, 12 },
	{ 60168, 30, 30, 12 },
	{ 95483, 30, 30, 12 },
	{ 154742, 30, 30, 12 },
	{ 256186, 30, 30, 10 },
	{ 434424, 30, 30, 10 },
	{ 749596, 30, 30, 10 },
	{ 1325716, 30, 30, 10 },
	{ 2097152, 24, 30, 10 },
	/* 21..30 */
	{ 2097152, 19, 30, 9 },
	{ 2097152, 15, 30, 9 },
	{ 2097152, 12, 30, 9 },
	{ 2097152, 10, 30, 9 },
	{ 2097152, 8, 30, 9 },
	{ 2097152, 6, 27, 8 },
	{ 2097152, 5, 24, 8 },
	{ 2097152, 4, 22, 8 },
	{ 2097152, 3, 20, 7 },
	{ 2097152, 3, 18, 7 },
	/* 31..40 */
	{ 2097152, 3, 16, 7 },
	{ 2097152, 3, 14, 6 },
	{ 2097152, 3, 13, 6 },
	{ 2097152, 3, 12, 6 },
	{ 2097152, 3, 10, 6 },
	{ 2097152, 3, 9, 6 },
	{ 2097152, 3, 8, 6 },
	{ 2097152, 3, 7, 6 },
	{ 2097152, 3, 6, 6 },
	{ 2097152, 3, 2, 6 },
};

static void *init(void)
{
	modeData *data = malloc(sizeof(modeData));
	if (!data) return NULL;
	data->level = 0;
	data->lines = 0;
	data->time = 0;
	data->gameOver = false;
	return data;
}

static void onCycle(qdsGame *game)
{
	modeData *data = qdsGetModeData(game);
	if (!data->gameOver) data->time += 1;
}

static void onLineFilled(qdsGame *game, int y)
{
	modeData *data = qdsGetModeData(game);
	data->lines += 1;
	int level = data->lines / 10;
	data->level = level > 39 ? 39 : level;
}

static void onTopOut(qdsGame *game)
{
	modeData *data = qdsGetModeData(game);
	data->gameOver = true;
}

static int modeCall(qdsGame *game, unsigned long call, void *argp)
{
	modeData *data = qdsGetModeData(game);
	const struct levelData *lvl = &speedCurve[data->level];
	switch (call) {
		case QDS_GETMODENAME:
			*(const char **)argp = "Marathon";
			return 0;
		case QDS_GETTIME:
			*(unsigned int *)argp = data->time;
			return 0;
		case QDS_GETLEVEL:
			*(int *)argp = data->level + 1;
			return 0;
		case QDS_GETSUBLEVEL:
			*(int *)argp = data->lines;
			return 0;
		case QDS_GETLEVELTARGET:
			*(int *)argp = (data->level + 1) * 10;
			return 0;
		case QDS_GETGRAVITY:
			*(int *)argp = lvl->gravity;
			return 0;
		case QDS_GETSDG:
			*(int *)argp = lvl->gravity * 20;
			return 0;
		case QDS_GETSDF:
			*(int *)argp = 20;
			return 0;
		case QDS_GETLINEDELAY:
			*(int *)argp = lvl->lineDelay;
			return 0;
		case QDS_GETLOCKTIME:
			*(int *)argp = lvl->lockTime;
			return 0;
		case QDS_GETDAS:
		case QDS_GETDCD:
			*(int *)argp = lvl->das;
			return 0;
		case QDS_GETNEXTCOUNT:
			*(int *)argp = 8;
			return 0;
	}
	return -ENOTTY;
}

static int modeInvisibleCall(qdsGame *game, unsigned long call, void *argp)
{
	if (call == QDS_GETMODENAME) {
		*(const char **)argp = "Invisible Marathon";
		return 0;
	} else if (call == QDS_GETVISIBILITY) {
		*(uint_fast16_t *)argp = 0;
		return 0;
	}

	return modeCall(game, call, argp);
}

QDS_API const qdsGamemode qdsModeMarathon = {
	.init = init,
	.destroy = free,
	.events = {
		.onCycle = onCycle,
		.onLineFilled = onLineFilled,
		.onTopOut = onTopOut,
	},
	.call = modeCall,
};

QDS_API const qdsGamemode qdsModeInvisible = {
	.init = init,
	.destroy = free,
	.events = {
		.onCycle = onCycle,
		.onLineFilled = onLineFilled,
		.onTopOut = onTopOut,
	},
	.call = modeInvisibleCall,
};
