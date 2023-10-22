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
#include "piecegen/quadus.h"
#include <calls.h>
#include <mode.h>
#include <quadus.h>

#include <errno.h>
#include <stdint.h>
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
	{ 2097152, 30, 30, 10 },
	/* 21..30 */
	{ 2097152, 27, 30, 9 },
	{ 2097152, 23, 30, 9 },
	{ 2097152, 18, 30, 9 },
	{ 2097152, 13, 30, 9 },
	{ 2097152, 9, 30, 9 },
	{ 2097152, 6, 29, 8 },
	{ 2097152, 6, 28, 8 },
	{ 2097152, 6, 26, 8 },
	{ 2097152, 6, 24, 7 },
	{ 2097152, 6, 21, 7 },
	/* 31..35 */
	{ 2097152, 6, 18, 7 },
	{ 2097152, 6, 14, 6 },
	{ 2097152, 6, 10, 6 },
	{ 2097152, 6, 6, 5 },
	{ 2097152, 6, 2, 5 },
};

static void *init(void)
{
	modeData *data = malloc(sizeof(modeData));
	if (!data) return NULL;
	data->level = 0;
	data->lines = 0;
	qdsQuadusGenInit(&data->rng, time(NULL));
	return data;
}

static int peek(const void *data, int pos)
{
	return qdsQuadusGenPeek(&((const modeData *)data)->rng, pos);
}

static int draw(void *data)
{
	return qdsQuadusGenDraw(&((modeData *)data)->rng);
}

static void onLineFilled(qdsGame *game, int y)
{
	modeData *data = qdsGetModeData(game);
	data->lines += 1;
	data->level = data->lines / 10;
	if (data->level > 34) data->level = 34;
}

static int modeCall(qdsGame *game, unsigned long call, void *argp)
{
	modeData *data = qdsGetModeData(game);
	const struct levelData *lvl = &speedCurve[data->level];
	switch (call) {
		case QDS_GETMODENAME:
			*(const char **)argp = "Invisible";
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
		case QDS_GETVISIBILITY:
			*(uint_fast16_t *)argp = 0;
			return 0;
	}
	return -ENOTTY;
}

QDS_API const qdsGamemode qdsModeInvisible = {
	.init = init,
	.destroy = free,
	.getPiece = peek,
	.shiftPiece = draw,
	.events = { .onLineFilled = onLineFilled },
	.call = modeCall,
};
