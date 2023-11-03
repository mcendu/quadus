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
#include <quadus.h>
#include <quadus/piece.h>
#include <quadus/piecegen/his.h>
#include <quadus/ruleset/rand.h>
#include <stdalign.h>
#include <string.h>

QDS_API int qdsDrawHistory(qdsTile *restrict his,
						   int *restrict head,
						   int len,
						   int tries,
						   int (*gen)(void *st),
						   void *restrict st)
{
	/* roll */
	int result = gen(st);
	if (tries > 1)
		for (int i = 0; i < len; ++i)
			if (his[i] == result)
				return qdsDrawHistory(his, head, len, tries - 1, gen, st);

	/* update history */
	his[*head] = result;
	*head = (*head + 1) % len;
	return result;
}

static const qdsTile initialHistory[4]
	= { QDS_PIECE_Z, QDS_PIECE_Z, QDS_PIECE_S, QDS_PIECE_S };
static const qdsTile initialDraws[4]
	= { QDS_PIECE_I, QDS_PIECE_J, QDS_PIECE_L, QDS_PIECE_T };

static int rand(void *rand)
{
	return (qdsRand(rand) % 7) + 1;
}

QDS_API void qdsHisInit(struct qdsHis *q, unsigned seed)
{
	q->queueHead = 0;
	q->hisHead = 1;
	memcpy(q->history, initialHistory, 4);
	qdsSrand(seed, &q->rng);

	/* never deal S, Z or O as the first piece in TGM tradition */
	int initialDraw = qdsRand(&q->rng) % 4;
	q->queue[0] = q->history[0] = initialDraws[initialDraw];

	/* rest of queue filled as usual */
	for (int i = 1; i < 8; ++i) {
		q->queue[i]
			= qdsDrawHistory(q->history, &q->hisHead, 4, 6, rand, &q->rng);
	}
}

QDS_API int qdsHisPeek(const struct qdsHis *q, int pos)
{
	return q->queue[(q->queueHead + pos) % 8];
}

QDS_API int qdsHisDraw(struct qdsHis *q)
{
	int result = q->queue[q->queueHead];
	q->queue[q->queueHead++]
		= qdsDrawHistory(q->history, &q->hisHead, 4, 6, rand, &q->rng);
	q->queueHead %= 8;
	return result;
}
