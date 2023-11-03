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
#include <quadus/piece.h>
#include <quadus/piecegen/tgm.h>
#include <string.h>

#define HISTOGRAPH_SIZE 7

typedef struct qdsTgmGen__histographEntry histographEntry;

static inline int cmpHistographEntries(histographEntry *restrict l,
									   histographEntry *restrict r)
{
	return l->drought - r->drought;
}

static void siftDown(histographEntry *heap, size_t node)
{
	/* check for leaf node */
	if (node > HISTOGRAPH_SIZE >> 1) return;

	size_t l = node << 1;
	size_t r = l + 1;
	size_t child;
	if (r > HISTOGRAPH_SIZE || cmpHistographEntries(heap + l, heap + r) >= 0)
		child = l;
	else
		child = r;

	if (cmpHistographEntries(heap + node, heap + child) < 0) {
		histographEntry tmp;
		memcpy(&tmp, heap + node, sizeof(histographEntry));
		memcpy(heap + node, heap + child, sizeof(histographEntry));
		memcpy(heap + child, &tmp, sizeof(histographEntry));
		siftDown(heap, child);
	}
}

static void updateHistograph(struct qdsTgmGen *gen, int draw)
{
	/* find entry representing the drawn piece */
	int pos = 1;
	for (int i = 1; i <= 7; ++i) {
		if (gen->histograph[i].piece == draw) {
			pos = i;
			break;
		}
	}

	for (int i = 0; i < 8; ++i) gen->histograph[i].drought += 1;
	gen->histograph[pos].drought = 0;
	siftDown(gen->histograph, pos);
}

static int drawBag(struct qdsTgmGen *gen)
{
	int piece;

	int bagpos = qdsRand(&gen->rand) % 35;
	for (piece = 0; bagpos >= 0 && piece < 7; ++piece)
		bagpos -= gen->bag[piece];

	return piece;
}

static int draw(struct qdsTgmGen *gen, int tries)
{
	/* draw */
	int piece = drawBag(gen);
	gen->bag[piece - 1] -= 1;
	/* with only 1 try the draw is always accepted */
	if (tries > 1)
		for (int i = 0; i < 4; ++i)
			if (gen->history[i] == piece) {
				/* put most droughted piece back */
				gen->bag[gen->histograph[1].piece - 1] += 1;

				return draw(gen, tries - 1);
			}

	/* update bag data */
	updateHistograph(gen, piece);
	gen->bag[gen->histograph[1].piece - 1] += 1;

	/* update history */
	gen->history[gen->hisHead] = piece;
	gen->hisHead = (gen->hisHead + 1) % 4;

	return piece;
}

QDS_API int qdsTgmGenPeek(const struct qdsTgmGen *q, unsigned pos)
{
	return q->queue[(q->queueHead + pos) % 8];
}

QDS_API int qdsTgmGenDraw(struct qdsTgmGen *q)
{
	int result = q->queue[q->queueHead];
	q->queue[q->queueHead++] = draw(q, 6);
	q->queueHead %= 8;
	return result;
}

static const histographEntry initialHistograph[]
	= { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 },
		{ 4, 0 }, { 5, 0 }, { 6, 0 }, { 7, 0 } };
static const qdsTile initialHistory[]
	= { QDS_PIECE_S, QDS_PIECE_S, QDS_PIECE_Z, QDS_PIECE_Z };
static const qdsTile initialDraws[4]
	= { QDS_PIECE_I, QDS_PIECE_J, QDS_PIECE_L, QDS_PIECE_T };

QDS_API void qdsTgmGenInit(struct qdsTgmGen *q, unsigned seed)
{
	qdsSrand(seed, &q->rand);
	memcpy(q->histograph, initialHistograph, sizeof(q->histograph));
	memset(q->bag, 5, sizeof(q->bag));
	memcpy(q->history, initialHistory, sizeof(q->history));
	q->hisHead = 1;
	q->queueHead = 0;

	/* never deal S, Z or O as the first piece */
	int initialDraw = qdsRand(&q->rand) % 4;
	q->queue[0] = q->history[0] = initialDraws[initialDraw];

	/* rest of queue filled as usual */
	for (int i = 1; i < 8; ++i) {
		q->queue[i] = draw(q, 6);
	}
}
