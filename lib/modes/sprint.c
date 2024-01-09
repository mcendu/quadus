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
#include <config.h>
#include <quadus.h>
#include <quadus/calls.h>
#include <quadus/mode.h>

#include <errno.h>
#include <stdlib.h>

#define LINE_TARGET 40

struct modeData
{
	unsigned int time;
	int lines;
	bool gameOver : 1;
};

static void *init(void)
{
	struct modeData *data = malloc(sizeof(struct modeData));
	if (!data) return NULL;
	data->time = 0;
	data->lines = 0;
	data->gameOver = false;
	return data;
}

static void onCycle(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);

	if (!data->gameOver && data->lines < 40) data->time += 1;
}

static bool onSpawn(qdsGame *game, int piece)
{
	struct modeData *data = qdsGetModeData(game);
	if (data->lines >= 40) {
		qdsEndGame(game);
		return false;
	}
	return true;
}

static void onTopOut(qdsGame *game)
{
	struct modeData *data = qdsGetModeData(game);
	data->gameOver = true;
}

static void onLineFilled(qdsGame *game, int y)
{
	struct modeData *data = qdsGetModeData(game);
	++data->lines;
}

static int call(qdsGame *game, unsigned long req, void *argp)
{
	struct modeData *data = qdsGetModeData(game);

	switch (req) {
		case QDS_GETMODENAME:
			*(const char **)argp = "Sprint";
			return 0;
		case QDS_GETTIME:
			*(unsigned int *)argp = data->time;
			return 0;
		case QDS_GETGRAVITY:
			*(int *)argp = 1092;
			return 0;
		case QDS_GETSDF:
			*(int *)argp = 30;
			return 0;
		case QDS_GETLEVEL:
			*(int *)argp = 1;
			return 0;
		case QDS_GETSUBLEVEL:
			*(int *)argp = data->lines;
			return 0;
		case QDS_GETLEVELTARGET:
			*(int *)argp = 40;
			return 0;
		case QDS_GETLINEDELAY:
			*(int *)argp = 0;
			return 0;
	}

	return -ENOTTY;
}

QDS_API const qdsGamemode qdsModeSprint = {
	.init = init,
	.destroy = free,
	.events = {
		.onCycle = onCycle,
		.onSpawn = onSpawn,
		.onLineFilled = onLineFilled,
		.onTopOut = onTopOut,
	},
	.call = call,
};
