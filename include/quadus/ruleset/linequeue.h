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
 * Utilities for buffers holding pending lines to clear.
 */
#ifndef QDS__RULESET_LINES_H
#define QDS__RULESET_LINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <quadus.h>

struct qdsPendingLines
{
	unsigned char lines;
	unsigned char h[47];
};

/**
 * Queue lines for removal.
 *
 * This API does not bounds-check; you should ensure yourself that the
 * input does not cause the queue to overflow.
 */
QDS_API void qdsQueueLine(struct qdsPendingLines *q, int y);

/**
 * Execute an action on all pending lines. This clears the queue.
 */
QDS_API int qdsForeachPendingLine(qdsGame *game,
								  struct qdsPendingLines *q,
								  void (*action)(qdsGame *, int y));

/**
 * Clear all pending lines.
 */
#define qdsClearQueuedLines(game, q) \
	(qdsForeachPendingLine(game, q, (void (*)(qdsGame *, int))qdsClearLine))

#ifdef __cplusplus
}
#endif

#endif /* !QDS__RULESET_LINES_H */
