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

#include <mino.h>
#include <mode.h>
#include <rs.h>

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

QDS_API void qdsPlayfieldAdvance(qdsPlayfield *p, unsigned int input)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	/* the ruleset has full control of how a game cycle should be */
	p->rs->doGameCycle(p, input);
}

QDS_API bool qdsPlayfieldSpawn(qdsPlayfield *p, int type)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));

	if (type == 0) type = p->rs->shiftPiece(p->rsData);

	EMIT(p, onSpawn, p, type);

	p->active = (qdsMino){ type, 0 };
	p->x = p->rs->spawnX(p);
	p->y = p->rs->spawnY(p);

	bool overlaps = qdsPlayfieldOverlaps(p);
	if (overlaps) {
		if (p->rs->onTopOut) p->rs->onTopOut(p);
		if (p->mode->onTopOut) p->mode->onTopOut(p);
	}

	return !overlaps;
}

QDS_API bool qdsPlayfieldTeleport(qdsPlayfield *p, int x, int y)
{
	assert((p != NULL));
	if (!qdsPlayfieldCanMove(p, x, y)) return false;

	p->x += x;
	p->y += y;
	return true;
}

QDS_API int qdsPlayfieldMove(qdsPlayfield *p, int offset)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	int i;
	if (offset < 0) {
		for (i = -1; i > offset; --i) {
			if (qdsPlayfieldCanMove(p, i, 0)) i -= 1;
		}
	} else {
		for (i = 1; i < offset; ++i) {
			if (qdsPlayfieldCanMove(p, i, 0)) i += 1;
		}
	}

	if (p->rs->onMove && !p->rs->onMove(p, i)) return 0;
	if (p->mode->onMove && !p->mode->onMove(p, i)) return 0;
	p->x += i;
	return i;
}

QDS_API int qdsPlayfieldDrop(qdsPlayfield *p, int type, int distance)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	int i;
	for (i = 1; i < distance; ++i) {
		if (qdsPlayfieldCanMove(p, 0, -1)) p->y -= 1;
	}

	if (p->rs->onDrop && !p->rs->onDrop(p, type, distance)) return 0;
	if (p->mode->onDrop && !p->mode->onDrop(p, type, distance)) return 0;
	return i;
}

QDS_API int qdsPlayfieldRotate(qdsPlayfield *p, int rotation)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	int x, y;
	int result = p->rs->canRotate(p, rotation, &x, &y);
	if (result == QDS_PLAYFIELD_ROTATE_FAILED)
		return QDS_PLAYFIELD_ROTATE_FAILED;

	if (p->rs->onRotate && !p->rs->onRotate(p, rotation))
		return QDS_PLAYFIELD_ROTATE_FAILED;
	if (p->mode->onRotate && !p->mode->onRotate(p, rotation))
		return QDS_PLAYFIELD_ROTATE_FAILED;

	p->x += x;
	p->y += y;
	p->active.orientation = (p->active.orientation + rotation) % 4;
	return result;
}

QDS_API bool qdsPlayfieldLock(qdsPlayfield *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	if (!qdsPlayfieldGrounded(p)) return false;

	if (p->rs->onLock && !p->rs->onLock(p)) return false;
	if (p->mode->onLock && !p->mode->onLock(p)) return false;

	const qdsCoords *shape
		= p->rs->getShape(p->active.type, p->active.orientation);
	for (const qdsCoords *b = shape; !(b->x == 127 && b->y == 127); ++b) {
		int x = p->x + b->x;
		int y = p->y + b->y;
		p->playfield[y][x] = p->active.type; /* for piece coloring */

		if (y > p->top) p->top = y;

		if (lineFilled(p, y)) {
			if (p->rs->onLineFilled) p->rs->onLineFilled(p, y);
			if (p->mode->onLineFilled) p->mode->onLineFilled(p, y);
		}
	}
	p->active = (qdsMino){ 0, 0 };
	return true;
}

QDS_API int qdsPlayfieldHold(qdsPlayfield *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	if (p->rs->onHold && !p->rs->onHold(p)) return QDS_PLAYFIELD_HOLD_BLOCKED;
	if (p->mode->onHold && !p->mode->onHold(p))
		return QDS_PLAYFIELD_HOLD_BLOCKED;

	int active = p->active.type;
	/* spawn already draws from the queue when hold is empty */
	bool overlaps = !qdsPlayfieldSpawn(p, p->hold);
	p->hold = active;

	if (overlaps) return QDS_PLAYFIELD_HOLD_TOPOUT;
	return QDS_PLAYFIELD_HOLD_SUCCESS;
}

QDS_API bool qdsPlayfieldClear(qdsPlayfield *p, int y)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	assert((p->mode != NULL));
	if (!lineFilled(p, y)) return false;
	if (p->rs->onLineClear && !p->rs->onLineClear(p, y)) return false;
	if (p->mode->onLineClear && !p->mode->onLineClear(p, y)) return false;

	int lineNum = (p->top)-- - y;
	memmove(
		p->playfield[y], p->playfield[y + 1], lineNum * sizeof(qdsTile[10]));
	return true;
}

QDS_API bool qdsPlayfieldCanRotate(qdsPlayfield *p, int x, int y, int rotation)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	rotation = (rotation + p->active.orientation) % 4;
	x += p->x;
	y += p->y;

	const qdsCoords *shape
		= p->rs->getShape(p->active.type, p->active.orientation);
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
	while (qdsPlayfieldCanMove(p, 0, i - 1)) i -= 1;
	return p->y + i;
}