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
#include <game.h>
#include <playfield.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <mode.h>
#include <rs.h>

QDS_API qdsTile (*qdsGetPlayfield(qdsPlayfield *p))[10]
{
	assert((p != NULL));
	return p->playfield;
}

QDS_API void qdsGetActive(qdsPlayfield *p,
						  int *x,
						  int *y,
						  int *type,
						  unsigned *orientation)
{
	assert((p != NULL));
	if (x) *x = p->x;
	if (y) *y = p->y;
	if (type) *type = p->piece;
	if (orientation) *orientation = p->orientation;
}

QDS_API int qdsGetNextPiece(qdsPlayfield *p, int pos)
{
	assert((p != NULL));
	return p->rs->getPiece(p->rsData, pos);
}

QDS_API int qdsGetHeldPiece(qdsPlayfield *p)
{
	assert((p != NULL));
	return p->hold;
}

QDS_API const qdsRuleset *qdsGetRuleset(qdsPlayfield *p)
{
	assert((p != NULL));
	return p->rs;
}

QDS_API void qdsSetRuleset(qdsPlayfield *p, const qdsRuleset *rs)
{
	assert((p != NULL));

	if (p->rsData != NULL) p->rs->destroy(p->rsData);
	p->rsData = rs->init();
	p->rs = rs;
}

QDS_API void *qdsGetRulesetData(qdsPlayfield *p)
{
	assert((p != NULL));
	return p->rsData;
}

QDS_API const qdsGamemode *qdsGetMode(qdsPlayfield *p)
{
	assert((p != NULL));
	return p->mode;
}

QDS_API void qdsSetMode(qdsPlayfield *p, const qdsGamemode *mode)
{
	assert((p != NULL));

	if (p->modeData != NULL) p->mode->destroy(p->modeData);
	p->modeData = mode->init();
	p->mode = mode;
}

QDS_API void *qdsGetModeData(qdsPlayfield *p)
{
	assert((p != NULL));
	return p->modeData;
}

QDS_API int qdsCall(qdsPlayfield *p, unsigned long req, ...)
{
	assert((p != NULL));

	int result;
	va_list ap;

	va_start(ap, req);
	if (p->rs && p->rs->call && (result = p->rs->call(p, req, ap)) != -ENOTTY) {
		va_end(ap);
		return result;
	}
	va_end(ap);

	va_start(ap, req);
	if (p->mode && p->mode->call
		&& (result = p->mode->call(p, req, ap)) != -ENOTTY) {
		va_end(ap);
		return result;
	}
	va_end(ap);

	return -ENOTTY;
}
