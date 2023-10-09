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
#include "quadustui.h"
#include <quadus.h>
#include <ruleset/linequeue.h>
#include <stdbool.h>
#include <string.h>
#include <ui.h>

void initUiData(uiState *data)
{
	data->useDisplayPlayfield = false;
	data->lines.lines = 0;
}

static void onLineFilled(qdsGame *game, int y)
{
	uiState *data = qdsGetUiData(game);
	qdsQueueLine(&data->lines, y);
}

static void removeLineFromDisplay(qdsGame *game, int y)
{
	if (y >= 22) return;
	uiState *data = qdsGetUiData(game);
	memset(data->displayPlayfield[y], 0, sizeof(qdsLine));
}

static bool postLock(qdsGame *game)
{
	uiState *data = qdsGetUiData(game);
	if (data->lines.lines == 0) return true;

	memcpy(data->displayPlayfield, qdsGetPlayfield(game), sizeof(qdsLine[22]));
	qdsForeachPendingLine(game, &data->lines, removeLineFromDisplay);
	data->useDisplayPlayfield = true;
	return true;
}

static bool onLineClear(qdsGame *game, int y)
{
	uiState *data = qdsGetUiData(game);
	data->useDisplayPlayfield = false;
	return true;
}

qdsUserInterface ui = { .events = {
							.onLineFilled = onLineFilled,
							.postLock = postLock,
							.onLineClear = onLineClear,
						} };
