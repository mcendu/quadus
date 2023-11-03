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
#ifndef UI_H
#define UI_H

#include <curses.h>
#include <quadus.h>
#include <quadus/ruleset/linequeue.h>
#include <quadus/ui.h>
#include <setjmp.h>
#include <stdalign.h>

#define PAIR_ACCENT 8

typedef struct uiState
{
	qdsGame *game;

	const struct inputHandler *inputHandler;
	void *inputData;
	unsigned int input;
	unsigned int time;

	qdsLine displayPlayfield[22];
	bool useDisplayPlayfield : 1;
	bool topOut : 1;
	struct qdsPendingLines lines;

	unsigned int topOutTime;
} uiState;

extern void initUiData(uiState *data);

extern void gameView(WINDOW *w, int top, int left, qdsGame *game);

extern qdsUserInterface ui;
extern jmp_buf cleanupJump;

#endif /* UI_H */
