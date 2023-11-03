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
#include <quadus.h>
#include <quadus/piece.h>

#include <stdint.h>
#include <string.h>

#define TIME_CREDITS (TIME_CREDITS_TRANSITION + FRAMETIME(0, 55, 30))

static const unsigned char creditsLineScoreFading[] = { 4, 8, 12, 26 };
static const unsigned char creditsLineScoreInvisible[] = { 10, 20, 30, 100 };

static void cycle(qdsGame *game, struct modeData *data)
{
	data->sectionTime += 1;
	if (data->sectionTime > TIME_CREDITS) {
		if (data->phase == PHASE_CREDITS_INVISIBLE)
			data->creditsPoints += 160;
		else
			data->creditsPoints += 50;
		data->phase = PHASE_GAME_OVER;
		qdsEndGame(game);
	}
}

static void onLineFilled(qdsGame *game, struct modeData *data, int y)
{
	data->lines += 1;
}

static void postLock(qdsGame *game, struct modeData *data)
{
	if (data->lines) {
		int l = data->lines - 1;
		if (l > 3) l = 3;
		if (data->phase == PHASE_CREDITS_INVISIBLE)
			data->creditsPoints += creditsLineScoreInvisible[l];
		else
			data->creditsPoints += creditsLineScoreFading[l];
	}
	data->lines = 0;
}

static int getSpeed(struct modeData *data)
{
	return 2097152;
}

const struct timingData creditsTimings = { 1200, 8, 15, 6, 6, 6 };

static const struct timingData *getTimings(struct modeData *data)
{
	return &creditsTimings;
}

static void setTileTime(qdsGame *game, struct modeData *data)
{
	int cx, cy;
	const qdsCoords *shape = qdsGetActiveShape(game);
	qdsGetActivePosition(game, &cx, &cy);

	QDS_SHAPE_FOREACH (i, shape) {
		int x = cx + i->x, y = cy + i->y;
		data->tileTime[y][x] = data->sectionTime;
	}
}

static void clearTileTimeLine(qdsGame *game, struct modeData *data, int y)
{
	int height = qdsGetFieldHeight(game);
	if (y > height) return;
	int lineNum = height-- - y - 1;
	memmove(
		data->tileTime[y], data->tileTime[y + 1], lineNum * sizeof(int[10]));
	memset(data->tileTime[height], 0, sizeof(int[10]));
}

static uint_fast16_t fadeVisibility(struct modeData *data, int line)
{
	uint_fast16_t visibility = 0;
	for (int i = 0; i < 10; ++i) {
		if (data->sectionTime < data->tileTime[line][i] + TIME(0, 5))
			visibility |= 1 << i;
	}
	return visibility;
}

const struct phase SHARED(phaseCreditsFading) = {
	.getSpeed = getSpeed,
	.getTimings = getTimings,
	.getVisibility = fadeVisibility,
	.onCycle = cycle,
	.onLock = setTileTime,
	.onLineClear = clearTileTimeLine,

	.onLineFilled = onLineFilled,
	.postLock = postLock,
};

const struct phase SHARED(phaseCreditsInvisible) = {
	.getSpeed = getSpeed,
	.getTimings = getTimings,
	.getVisibility = SHARED(invisible),
	.onCycle = cycle,

	.onLineFilled = onLineFilled,
	.postLock = postLock,
};

const struct phase SHARED(phaseGameOver) = {
	.getSpeed = getSpeed,
	.getTimings = getTimings,
	.getVisibility = SHARED(visible),
};
