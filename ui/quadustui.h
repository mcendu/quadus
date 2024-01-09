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

#include "config.h"

#include <curses.h>
#include <quadus.h>
#include <quadus/ruleset/linequeue.h>
#include <quadus/ui.h>
#include <setjmp.h>
#include <stdalign.h>

#define PAIR_ACCENT 8

#define INPUT_UI_UP (1 << 16)
#define INPUT_UI_DOWN (1 << 17)
#define INPUT_UI_LEFT (1 << 18)
#define INPUT_UI_RIGHT (1 << 19)
#define INPUT_UI_CONFIRM (1 << 20)
#define INPUT_UI_BACK (1 << 21)
#define INPUT_UI_MENU (1 << 22)

typedef struct rect
{
	int x;
	int y;
	int w;
	int h;
} rect;

typedef struct screen screen;

typedef struct uiState
{
	const screen *currentScreen;
	void *screenData;

	const qdsRuleset *ruleset;
	const qdsGamemode *mode;

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
extern void changeScreen(uiState *, const screen *screen);

#ifdef HAVE_UDEV
#include <libudev.h>
#include <poll.h>
extern int searchKeyboards(struct pollfd *fds,
						   size_t capacity,
						   struct udev *udev);
#endif

extern qdsUserInterface ui;
extern jmp_buf cleanupJump;

extern const screen screenMainMenu;
extern const screen screenGame;
extern const screen screenModeSelect;

#endif /* UI_H */
