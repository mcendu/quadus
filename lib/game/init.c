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
#include "game.h"
#include <config.h>
#include <quadus.h>
#include <quadus/mode.h>
#include <quadus/ruleset.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

QDS_API qdsGame *qdsNewGame(void)
{
	qdsGame *p = aligned_alloc(alignof(qdsGame), sizeof(qdsGame));
	if (!p) return p;
	qdsInitGame(p);
	return p;
}

QDS_API void qdsDestroyGame(qdsGame *p)
{
	qdsCleanupGame(p);
	free(p);
}

QDS_API void qdsInitGame(qdsGame *p)
{
	assert((p != NULL));
	memset(p->playfield, 0, sizeof(p->playfield));
	p->piece = QDS_PIECE_NONE;
	p->orientation = QDS_ORIENTATION_BASE;
	p->height = 0;
	p->hold = 0;
	p->rs = NULL;
	p->rsData = NULL;
	p->mode = NULL;
	p->modeData = NULL;
	p->ui = NULL;
	p->uiData = NULL;
};

QDS_API void qdsCleanupGame(qdsGame *p)
{
	if (p->rs && p->rsData) {
		p->rs->destroy(p->rsData);
		p->rsData = NULL;
	}
	if (p->mode && p->modeData) {
		p->mode->destroy(p->modeData);
		p->modeData = NULL;
	}
}
