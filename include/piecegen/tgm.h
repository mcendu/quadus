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
#ifndef QDS__PIECEGEN_TGM_H
#define QDS__PIECEGEN_TGM_H

#include "../quadus.h"
#include "../ruleset/rand.h"

#include <stdalign.h>

struct qdsTgmGen
{
	qdsRandState rand;
	qdsTile queue[8];
	alignas(16) struct qdsTgmGen__histographEntry
	{
		qdsTile piece;
		unsigned char drought;
	} histograph[8];
	unsigned char bag[8];
	qdsTile history[4];
	int hisHead;
	int queueHead;
};

QDS_API void qdsTgmGenInit(struct qdsTgmGen *q, unsigned seed);
QDS_API int qdsTgmGenPeek(const struct qdsTgmGen *q, unsigned pos);
QDS_API int qdsTgmGenDraw(struct qdsTgmGen *q);

#endif
