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
#include <qdsbuild.h>

#include <game.h>
#include <playfield.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <mode.h>
#include <rs.h>
#include <threads.h>

#define EMIT(p, e, ...)                            \
	do {                                           \
		if (p->rs->e) (p->rs->e)(__VA_ARGS__);     \
		if (p->mode->e) (p->mode->e)(__VA_ARGS__); \
	} while (0)

#define EMIT_CANCELLABLE(p, e, cancel_retval, ...)        \
	do {                                                  \
		if ((p->rs->e) && !(p->rs->e)(__VA_ARGS__)) {     \
			return (cancel_retval);                       \
		}                                                 \
		if ((p->mode->e) && !(p->mode->e)(__VA_ARGS__)) { \
			return (cancel_retval);                       \
		}                                                 \
	} while (0)

static bool lineFilled(qdsPlayfield *p, int y)
{
	for (int i = 0; i < 10; ++i) {
		if (p->playfield[y][i] == 0) return false;
	}

	return true;
}

QDS_API void qdsRunCycle(qdsPlayfield *p, unsigned int input)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	/* the ruleset has full control of how a game cycle should be */
	p->rs->doGameCycle(p, input);
}

QDS_API bool qdsSpawn(qdsPlayfield *p, int type)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));

	if (type == 0) type = p->rs->shiftPiece(p->rsData);

	EMIT_CANCELLABLE(p, onSpawn, false, p, type);

	p->piece = type;
	p->orientation = 0;
	p->x = p->rs->spawnX(p);
	p->y = p->rs->spawnY(p);

	bool overlaps = qdsOverlaps(p);
	if (overlaps) EMIT(p, onTopOut, p);

	return !overlaps;
}

QDS_API bool qdsTeleport(qdsPlayfield *p, int x, int y)
{
	assert((p != NULL));
	if (!qdsCanMove(p, x, y)) return false;

	p->x += x;
	p->y += y;
	return true;
}

QDS_API int qdsMove(qdsPlayfield *p, int offset)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	int i;
	if (offset < 0) {
		for (i = 0; i > offset; --i) {
			if (!qdsCanMove(p, i - 1, 0)) break;
		}
	} else {
		for (i = 0; i < offset; ++i) {
			if (!qdsCanMove(p, i + 1, 0)) break;
		}
	}

	EMIT_CANCELLABLE(p, onMove, 0, p, i);
	p->x += i;
	return i;
}

QDS_API int qdsDrop(qdsPlayfield *p, int type, int distance)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	int i;
	for (i = 0; i < distance; ++i) {
		if (!qdsCanMove(p, 0, -(i + 1))) break;
	}

	EMIT_CANCELLABLE(p, onDrop, 0, p, type, i);
	p->y -= i;
	return i;
}

QDS_API int qdsRotate(qdsPlayfield *p, int rotation)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	int x, y;
	int result = p->rs->canRotate(p, rotation, &x, &y);
	if (result == QDS_ROTATE_FAILED) return QDS_ROTATE_FAILED;

	EMIT_CANCELLABLE(p, onRotate, QDS_ROTATE_FAILED, p, rotation);

	p->x += x;
	p->y += y;
	p->orientation = (p->orientation + rotation) % 4;
	return result;
}

QDS_API bool qdsLock(qdsPlayfield *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));

	if (!qdsGrounded(p)) return false;
	EMIT_CANCELLABLE(p, onLock, false, p);

	const qdsCoords *shape = p->rs->getShape(p->piece, p->orientation);
	for (const qdsCoords *b = shape; !(b->x == 127 && b->y == 127); ++b) {
		int x = p->x + b->x;
		int y = p->y + b->y;
		if (x < 0 || x >= 10 || y < 0 || y >= 48) continue;
		p->playfield[y][x] = p->piece; /* for piece coloring */

		if (y > p->top) p->top = y;

		if (lineFilled(p, y)) EMIT(p, onLineFilled, p, y);
	}
	p->piece = 0;
	p->orientation = 0;
	return true;
}

QDS_API int qdsHold(qdsPlayfield *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	EMIT_CANCELLABLE(p, onHold, QDS_HOLD_BLOCKED, p, p->piece);

	int active = p->piece;
	/* spawn already draws from the queue when hold is empty */
	bool overlaps = !qdsSpawn(p, p->hold);
	p->hold = active;

	if (overlaps) return QDS_HOLD_TOPOUT;
	return QDS_HOLD_SUCCESS;
}

QDS_API bool qdsClearLine(qdsPlayfield *p, int y)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	if (!lineFilled(p, y)) return false;
	EMIT_CANCELLABLE(p, onLineClear, false, p, y);

	int lineNum = (p->top)-- - y;
	memmove(
		p->playfield[y], p->playfield[y + 1], lineNum * sizeof(qdsTile[10]));
	return true;
}

QDS_API bool qdsCanRotate(qdsPlayfield *p, int x, int y, int rotation)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	rotation = (unsigned)(rotation + p->orientation) % 4;
	x += p->x;
	y += p->y;

	const qdsCoords *shape = p->rs->getShape(p->piece, rotation);
	for (const qdsCoords *b = shape; !(b->x == 127 && b->y == 127); ++b) {
		int bx = x + b->x;
		int by = y + b->y;

		if (by < 0 || by >= 48) return false;
		if (bx < 0 || bx >= 10) return false;
		if (p->playfield[by][bx] != 0) return false;
	}

	return true;
}

QDS_API int qdsPlayfieldGetGhostY(qdsPlayfield *p)
{
	assert((p != NULL));
	int i = 0;
	while (qdsCanMove(p, 0, i - 1)) i -= 1;
	return p->y + i;
}