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
#include <quadus.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <mode.h>
#include <ruleset.h>
#include <ui.h>

#define EMIT(p, e, ...)                                                       \
	do {                                                                      \
		if (p->rs->events.e) (p->rs->events.e)(__VA_ARGS__);                  \
		if (p->mode && (p->mode->events.e)) (p->mode->events.e)(__VA_ARGS__); \
		if (p->ui && (p->ui->events.e)) (p->ui->events.e)(__VA_ARGS__);       \
	} while (0)

#define EMIT_CANCELLABLE(p, e, cancel_retval, ...)                    \
	do {                                                              \
		if ((p->rs->events.e) && !(p->rs->events.e)(__VA_ARGS__)) {   \
			return (cancel_retval);                                   \
		}                                                             \
		if (p->mode && (p->mode->events.e)                            \
			&& !(p->mode->events.e)(__VA_ARGS__)) {                   \
			return (cancel_retval);                                   \
		}                                                             \
		if (p->ui && p->ui->events.e) (p->ui->events.e)(__VA_ARGS__); \
	} while (0)

/**
 * Draw a piece from the piece queue.
 */
static int shiftPiece(const qdsGame *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));

	if (p->mode && p->mode->shiftPiece) return p->mode->shiftPiece(p->modeData);
	return p->rs->shiftPiece(p->rsData);
}

/**
 * Check if a line is filled.
 */
static bool lineFilled(qdsGame *p, int y)
{
	for (int i = 0; i < 10; ++i) {
		if (p->playfield[y][i] == 0) return false;
	}

	return true;
}

QDS_API void qdsRunCycle(qdsGame *p, unsigned int input)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	/* the ruleset has full control of how a game cycle should be */
	p->rs->doGameCycle(p, input);
	EMIT(p, onCycle, p);
}

QDS_API bool qdsSpawn(qdsGame *p, int type)
{
	assert((p != NULL));
	assert((p->rs != NULL));

	if (type == 0) type = shiftPiece(p);

	EMIT_CANCELLABLE(p, onSpawn, false, p, type);

	p->piece = type;
	p->orientation = 0;
	p->x = p->rs->spawnX(p);
	p->y = p->rs->spawnY(p);

	return true;
}

QDS_API bool qdsTeleport(qdsGame *p, int x, int y)
{
	assert((p != NULL));
	if (!qdsCanMove(p, x, y)) return false;

	p->x += x;
	p->y += y;
	return true;
}

QDS_API int qdsMove(qdsGame *p, int offset)
{
	assert((p != NULL));
	assert((p->rs != NULL));
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

QDS_API int qdsDrop(qdsGame *p, int type, int distance)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	int i;
	for (i = 0; i < distance; ++i) {
		if (!qdsCanMove(p, 0, -(i + 1))) break;
	}

	EMIT_CANCELLABLE(p, onDrop, 0, p, type, i);
	p->y -= i;
	return i;
}

QDS_API int qdsRotate(qdsGame *p, int rotation)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	int x, y;
	int result = p->rs->canRotate(p, rotation, &x, &y);
	if (result == QDS_ROTATE_FAILED) return QDS_ROTATE_FAILED;

	EMIT_CANCELLABLE(p, onRotate, QDS_ROTATE_FAILED, p, rotation, result);

	p->x += x;
	p->y += y;
	p->orientation = (p->orientation + rotation) % 4;
	return result;
}

QDS_API bool qdsLock(qdsGame *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));

	if (!qdsGrounded(p)) return false;
	EMIT_CANCELLABLE(p, onLock, false, p);

	const qdsCoords *shape = p->rs->getShape(p->piece, p->orientation);
	for (const qdsCoords *b = shape; !(b->x == 127 && b->y == 127); ++b) {
		int x = p->x + b->x;
		int y = p->y + b->y;
		if (x < 0 || x >= 10 || y < 0 || y >= 48) continue;
		p->playfield[y][x] = p->piece; /* for piece coloring */

		if (y >= p->height) p->height = y + 1;

		if (lineFilled(p, y)) EMIT(p, onLineFilled, p, y);
	}
	p->piece = 0;
	p->orientation = 0;
	EMIT(p, postLock, p);
	return true;
}

QDS_API int qdsHold(qdsGame *p)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	EMIT_CANCELLABLE(p, onHold, QDS_HOLD_BLOCKED, p, p->piece);

	int active = p->piece;
	/* spawn already draws from the queue when hold is empty */
	bool overlaps = !qdsSpawn(p, p->hold);
	p->hold = active;

	if (overlaps) return QDS_HOLD_TOPOUT;
	return QDS_HOLD_SUCCESS;
}

QDS_API bool qdsClearLine(qdsGame *p, int y)
{
	assert((p != NULL));
	assert((p->rs != NULL));
	EMIT_CANCELLABLE(p, onLineClear, false, p, y);

	if (y >= p->height) return true;
	int lineNum = p->height-- - y - 1;
	memmove(p->playfield[y], p->playfield[y + 1], lineNum * sizeof(qdsLine));
	memset(p->playfield[p->height], 0, sizeof(qdsLine));
	return true;
}

QDS_API bool qdsAddLines(qdsGame *restrict p,
						 const qdsLine *restrict src,
						 size_t count)
{
	assert((p != NULL));
	assert((p->rs != NULL));

	bool topout = false;
	int playfieldRows = p->height; /* number of rows to copy */

	if (count > 48) {
		topout = true;
		count = 48;
		playfieldRows = 0;
	} else if (playfieldRows + count > 48) {
		topout = true;
		playfieldRows -= (count + playfieldRows) - 48;
	}
	p->height = playfieldRows + count;

	memmove(
		p->playfield[count], p->playfield[0], playfieldRows * sizeof(qdsLine));
	memcpy(p->playfield, src, count * sizeof(qdsLine));

	if (topout) EMIT(p, onTopOut, p);
	return !topout;
}

QDS_API bool qdsCanRotate(const qdsGame *p, int x, int y, int rotation)
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

QDS_API void qdsClearPlayfield(qdsGame *p)
{
	memset(p->playfield, 0, sizeof(p->playfield));
	p->height = 0;
}

QDS_API void qdsEndGame(qdsGame *p)
{
	EMIT(p, onTopOut, p);
}
