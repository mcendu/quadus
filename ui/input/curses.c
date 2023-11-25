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
#include "input.h"
#include "quadustui.h"
#include <curses.h>
#include <quadus.h>
#include <signal.h>

static unsigned int readCursesInput(unsigned int *last, void *_)
{
	unsigned int input = 0;
	unsigned int ch;
	while ((ch = getch()) != ERR) {
		switch (ch) {
			case KEY_LEFT:
				input |= QDS_INPUT_LEFT;
				input |= INPUT_UI_LEFT;
				break;
			case KEY_RIGHT:
				input |= QDS_INPUT_RIGHT;
				input |= INPUT_UI_RIGHT;
				break;
			case KEY_DOWN:
				input |= QDS_INPUT_SOFT_DROP;
				input |= INPUT_UI_DOWN;
				break;
			case KEY_UP:
				input |= QDS_INPUT_HARD_DROP;
				input |= INPUT_UI_UP;
				break;
			case '\n':
				input |= INPUT_UI_CONFIRM;
				break;
			case ' ':
				input |= QDS_INPUT_HARD_DROP;
				input |= INPUT_UI_CONFIRM;
				break;
			case 'z':
			case 'Z':
			case 'c':
			case 'C':
				input |= QDS_INPUT_ROTATE_CC;
				break;
			case 'x':
			case 'X':
				input |= QDS_INPUT_ROTATE_C;
				break;
			case 'a':
			case 'A':
				input |= QDS_INPUT_HOLD;
				break;
			case 'q':
			case 'Q':
				raise(SIGINT);
				break;
		}
	}
	*last = input;
	return input;
}

const struct inputHandler cursesInput = {
	.read = readCursesInput,
};
