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
 * Low level playfield actions for consumption by rulesets.
 */
#ifndef QDS__PLAYFIELD_H
#define QDS__PLAYFIELD_H

#include "quadus.h"
#include <stdalign.h>

#include <mode.h>
#include <ruleset.h>

/**
 * Definition of qdsPlayfield.
 */
struct qdsGame
{
	alignas(sizeof(qdsLine)) qdsLine playfield[48];
	int x;
	int y;
	int piece;
	unsigned orientation;
	int height;
	int hold;

	const qdsRuleset *rs;
	void *rsData;
	const qdsGamemode *mode;
	void *modeData;
	const qdsUserInterface *ui;
	void *uiData;
};

#endif /* !QDS__PLAYFIELD_H */
