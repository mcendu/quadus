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
#include <quadus.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <mode.h>
#include <ruleset.h>

QDS_API qdsLine *qdsGetPlayfield(qdsGame *p)
{
	assert((p != NULL));
	return p->playfield;
}

QDS_API void qdsGetActivePosition(qdsGame *p, int *x, int *y)
{
	assert((p != NULL));
	if (x) *x = p->x;
	if (y) *y = p->y;
}

QDS_API int qdsGetActiveX(qdsGame *p)
{
	assert((p != NULL));
	return p->x;
}

QDS_API int qdsGetActiveY(qdsGame *p)
{
	assert((p != NULL));
	return p->y;
}

QDS_API int qdsGetNextPiece(qdsGame *p, int pos)
{
	assert((p != NULL));
	return p->rs->getPiece(p->rsData, pos);
}

QDS_API int qdsGetHeldPiece(qdsGame *p)
{
	assert((p != NULL));
	return p->hold;
}

QDS_API const qdsRuleset *qdsGetRuleset(qdsGame *p)
{
	assert((p != NULL));
	return p->rs;
}

QDS_API void qdsSetRuleset(qdsGame *p, const qdsRuleset *rs)
{
	assert((p != NULL));

	if (p->rsData != NULL) p->rs->destroy(p->rsData);
	p->rsData = rs->init();
	p->rs = rs;
}

QDS_API void *qdsGetRulesetData(qdsGame *p)
{
	assert((p != NULL));
	return p->rsData;
}

QDS_API const qdsGamemode *qdsGetMode(qdsGame *p)
{
	assert((p != NULL));
	return p->mode;
}

QDS_API void qdsSetMode(qdsGame *p, const qdsGamemode *mode)
{
	assert((p != NULL));

	if (p->modeData != NULL) p->mode->destroy(p->modeData);
	p->modeData = mode->init();
	p->mode = mode;
}

QDS_API void *qdsGetModeData(qdsGame *p)
{
	assert((p != NULL));
	return p->modeData;
}

QDS_API int qdsCall(qdsGame *p, unsigned long req, void *argp)
{
	assert((p != NULL));

	int result;

	if (p->mode && p->mode->call
		&& (result = p->mode->call(p, req, argp)) != -ENOTTY) {
		return result;
	}

	if (p->rs && p->rs->call
		&& (result = p->rs->call(p, req, argp)) != -ENOTTY) {
		return result;
	}

	return -ENOTTY;
}
