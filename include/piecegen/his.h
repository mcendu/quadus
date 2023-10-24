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
#ifndef QDS__PIECEGEN_HIS_H
#define QDS__PIECEGEN_HIS_H

#include "../quadus.h"
#include "../ruleset/rand.h"

/**
 * Draw a piece while performing history checking.
 */
QDS_API int qdsDrawHistory(qdsTile *history,
						   int *head,
						   int length,
						   int tries,
						   int (*gen)(void *st),
						   void *restrict st);

/**
 * Data for an H4Rx generator.
 */
struct qdsHis
{
	qdsTile queue[8];
	qdsTile history[4];
	int queueHead;
	int hisHead;
	qdsRandState rng;
};

QDS_API void qdsHisInit(struct qdsHis *q, unsigned seed);
QDS_API int qdsHisPeek(const struct qdsHis *q, int pos);
QDS_API int qdsHisDraw(struct qdsHis *q);

#endif /* !QDS__PIECEGEN_HIS_H */
