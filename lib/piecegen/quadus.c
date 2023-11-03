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
#include <quadus/piecegen/quadus.h>
#include <quadus/ruleset/rand.h>

#define INITIAL_WEIGHT 7

static qdsTile draw(struct qdsQuadusGen *q)
{
	int n = qdsRand(&q->rand) % 7 * INITIAL_WEIGHT;

	/* find piece to deal */
	int p;
	for (p = 0; n >= 0 && p < 7; ++p) n -= q->weights[p];
	--p; /* fencepost */

	/* reweight */
	q->weights[p] -= 7;
	for (int i = 0; i < 7; ++i) {
		q->weights[i] += 1;
	}

	return p + 1;
}

QDS_API void qdsQuadusGenInit(struct qdsQuadusGen *q, unsigned seed)
{
	qdsSrand(seed, &q->rand);
	for (int i = 0; i < 7; ++i) q->weights[i] = INITIAL_WEIGHT;
	q->head = 0;

	for (int i = 0; i < sizeof(q->queue); ++i) q->queue[i] = draw(q);
}

QDS_API int qdsQuadusGenPeek(const struct qdsQuadusGen *q, int pos)
{
	return q->queue[(q->head + pos) % 8];
}

QDS_API int qdsQuadusGenDraw(struct qdsQuadusGen *q)
{
	int piece = q->queue[q->head];
	q->queue[q->head] = draw(q);
	q->head = (q->head + 1) % 8;
	return piece;
}
