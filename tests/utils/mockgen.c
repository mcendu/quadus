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
#include "mockgen.h"

#include <mode.h>
#include <quadus.h>

#include <assert.h>
#include <stdlib.h>

typedef struct mockGenData
{
	const int *sequence;
	int count;
	int next;
} mockGenData;

void setMockSequence(qdsGame *restrict game, const int *restrict seq, int count)
{
	assert((qdsGetMode(game) == &mockGenMode));
	mockGenData *data = qdsGetModeData(game);
	data->sequence = seq;
	data->count = count;
	data->next = 0;
}

static void *init()
{
	mockGenData *data = malloc(sizeof(mockGenData));
	if (!data) return NULL;
	data->sequence = NULL;
	data->count = 0;
	data->next = 0;
	return data;
}

static int peek(const mockGenData *data, int pos)
{
	return data->sequence[(data->next + pos) % data->count];
}

static int draw(mockGenData *data)
{
	int piece = data->sequence[data->next];
	data->next += 1;
	data->next %= data->count;
	return piece;
}

const qdsGamemode mockGenMode = {
	.init = init,
	.destroy = free,
	.getPiece = (int (*)(const void *, int))peek,
	.shiftPiece = (int (*)(void *))draw,
};
