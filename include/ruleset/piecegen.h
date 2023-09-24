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
/**
 * Various piece generators.
 */
#ifndef QDS__RULESET_PIECEGEN_H
#define QDS__RULESET_PIECEGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../quadus.h"
#include "rand.h"

/**
 * State for the standard "7-bag" piece generator.
 */
struct qdsBag
{
	qdsRandState rng;
	qdsTile pieces[14];
	unsigned char head;
};

#define QDS_BAG_DEPTH 7

/**
 * Initialize a standard piece generator.
 */
QDS_API void qdsBagInit(struct qdsBag *q, unsigned seed);

/**
 * Peek into a standard piece generator.
 */
QDS_API int qdsBagPeek(const struct qdsBag *q, int pos);

/**
 * Draw a piece from a standard piece generator.
 */
QDS_API int qdsBagDraw(struct qdsBag *q);

#ifdef __cplusplus
}
#endif

#endif /* !QDS__RULESET_PIECEGEN_H */
