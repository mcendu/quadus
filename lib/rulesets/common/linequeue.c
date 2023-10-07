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
#include <ruleset/linequeue.h>

#include <ruleset.h>
#include <stddef.h>

QDS_API void qdsQueueLine(struct qdsPendingLines *q, int y)
{
	q->h[q->lines++] = y;
}

inline static void siftdown(size_t node, unsigned char *heap, size_t size)
{
	size_t l, r, child, parent = node;

	while (parent <= size >> 1) {
		l = parent << 1;
		r = l + 1;
		child = (r > size || heap[l] >= heap[r]) ? l : r;

		if (heap[parent] >= heap[child]) break;
		unsigned char tmp = heap[child];
		heap[child] = heap[parent];
		heap[parent] = tmp;
		parent = child;
	}
}

QDS_API int qdsForeachPendingLine(qdsGame *restrict game,
								  struct qdsPendingLines *restrict q,
								  void (*action)(qdsGame *, int y))
{
	/* heap sort variation */
	int size = q->lines;

	/* turn pending lines into max heap */
	unsigned char *heap = q->h - 1;
	for (size_t i = size >> 1; i >= 1; --i) {
		siftdown(i, heap, size);
	}

	/* pop lines */
	while (size > 0) {
		action(game, heap[1]);
		heap[1] = heap[size--];
		siftdown(1, heap, size);
	}

	int linesCleared = q->lines;
	q->lines = 0;
	return linesCleared;
}
