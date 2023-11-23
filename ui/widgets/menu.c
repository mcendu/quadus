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
#include "quadustui.h"
#include <curses.h>
#include <stddef.h>

struct menuItem
{
	const char *name;
	void *data;
};

void menu(WINDOW *restrict w,
		  const rect *restrict rect,
		  const struct menuItem *restrict menu,
		  size_t selection)
{
	size_t itemcount = 0;
	for (const struct menuItem *i = menu; i->name != NULL; ++i) itemcount++;

	size_t scroll = 0;
	if (itemcount > rect->h && selection > rect->h / 2) {
		if (scroll + rect->h > itemcount)
			scroll = itemcount - rect->h;
		else
			scroll = selection - rect->h / 2;
	}

	for (int i = 0; i < rect->h; ++i) {
		const struct menuItem *item = &menu[scroll + i];

		wmove(w, rect->y + i, rect->x);
		if (scroll + i == selection)
			wattr_set(w, A_REVERSE, 0, NULL);
		else
			wattr_set(w, 0, 0, NULL);
		waddnstr(w, item->name, rect->w);
	}
}

size_t updateSelection(size_t old,
					   const struct menuItem *restrict menu,
					   unsigned int input)
{
	if (input & INPUT_UI_UP && old - 1 >= 0)
		return old - 1;
	else if (input & INPUT_UI_DOWN && menu[old + 1].name != NULL)
		return old + 1;
	else
		return old;
}
