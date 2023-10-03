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
#include <ruleset/piecegen.h>

#include <string.h>

#include <piece.h>
#include <ruleset/rand.h>

static void genBag(qdsTile *q, qdsRandState *rng)
{
	const qdsTile pieces[] = {
		QDS_PIECE_I, QDS_PIECE_J, QDS_PIECE_L, QDS_PIECE_O,
		QDS_PIECE_S, QDS_PIECE_T, QDS_PIECE_Z,
	};

	memcpy(q, pieces, sizeof(pieces));
	for (int i = 6; i > 0; --i) {
		int n = qdsRand(rng) % (i + 1);
		qdsTile tmp = q[i];
		q[i] = q[n];
		q[n] = tmp;
	}
}

QDS_API void qdsBagInit(struct qdsBag *q, unsigned seed)
{
	qdsSrand(seed, &q->rng);
	genBag(q->pieces, &q->rng);
	genBag(q->pieces + 7, &q->rng);
	q->head = 0;
}

QDS_API int qdsBagPeek(const struct qdsBag *q, int pos)
{
	return q->pieces[(q->head + pos) % 14];
}

QDS_API int qdsBagDraw(struct qdsBag *q)
{
	int p = q->pieces[q->head];
	q->head = (q->head + 1) % 14;
	if (q->head % 7 == 0) {
		genBag(&q->pieces[(q->head + 7) % 14], &q->rng);
	}
	return p;
}
