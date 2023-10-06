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

#include <calls.h>
#include <curses.h>
#include <piece.h>
#include <quadus.h>

#include "widget.h"

static const char tileFilled[] = "[]";
static const char tileEmpty[] = ". ";
static const char tileEmptyOverflow[] = "  ";

static void playfieldLine(WINDOW *w,
						  const qdsTile *line,
						  int row,
						  int left,
						  chtype boundary,
						  const char *tileFilled,
						  const char *tileEmpty)
{
	wmove(w, row, left);
	waddch(w, boundary);
	for (int x = 0; x < 10; ++x) {
		const char *tile = line[x] ? tileFilled : tileEmpty;
		waddstr(w, tile);
	}
	waddch(w, boundary);
}

static void playfield(WINDOW *w, int top, int left, const qdsGame *game)
{
	const int playfieldBaseY = 21;

	/* playfield base */
	wmove(w, top + playfieldBaseY + 1, left);
	waddchstr(
		w, ((const chtype[]){ ACS_LLCORNER, ACS_HLINE,	  ACS_HLINE, ACS_HLINE,
							  ACS_HLINE,	ACS_HLINE,	  ACS_HLINE, ACS_HLINE,
							  ACS_HLINE,	ACS_HLINE,	  ACS_HLINE, ACS_HLINE,
							  ACS_HLINE,	ACS_HLINE,	  ACS_HLINE, ACS_HLINE,
							  ACS_HLINE,	ACS_HLINE,	  ACS_HLINE, ACS_HLINE,
							  ACS_HLINE,	ACS_LRCORNER, 0 }));

	/* playfield proper */
	const qdsLine *playfield = qdsGetPlayfield(game);
	for (int y = 0; y < 20; ++y) {
		playfieldLine(w,
					  playfield[y],
					  top + playfieldBaseY - y,
					  left,
					  ACS_VLINE,
					  tileFilled,
					  tileEmpty);
	}
	for (int y = 20; y < 22; ++y) {
		playfieldLine(w,
					  playfield[y],
					  top + playfieldBaseY - y,
					  left,
					  ' ',
					  tileFilled,
					  tileEmptyOverflow);
	}

	/* active piece */
	const qdsCoords *shape = qdsGetActiveShape(game);
	int x, y;
	qdsGetActivePosition(game, &x, &y);
	QDS_SHAPE_FOREACH (i, shape) {
		int tx = x + i->x;
		int ty = y + i->y;
		if (tx < 0 || tx >= 10) return;
		if (ty < 0 || ty >= 22) return;
		mvwaddstr(w, top + playfieldBaseY - ty, left + 1 + 2 * tx, tileFilled);
	}
	wmove(w, top + playfieldBaseY - y, left + 1 + 2 * x);
}

static void queuedPiece(WINDOW *w, int top, int left, qdsGame *game, int piece)
{
	/* clear corresponding screen area */
	for (int y = 0; y < 3; ++y) mvwaddstr(w, top + y, left, "        ");

	int cx = left + 2;
	int cy = top + 1;
	const qdsCoords *shape = qdsGetShape(game, piece, 0);

	QDS_SHAPE_FOREACH (i, shape) {
		int x = cx + 2 * i->x;
		int y = cy - i->y;
		mvwaddstr(w, y, x, tileFilled);
	}
}

static void hold(WINDOW *w, int top, int left, qdsGame *game)
{
	queuedPiece(w, top, left, game, qdsGetHeldPiece(game));
}

static void next(WINDOW *w, int top, int left, qdsGame *game)
{
	int nextCount;
	if (qdsCall(game, QDS_GETNEXTCOUNT, &nextCount) < 0) nextCount = 1;
	if (nextCount > 6) nextCount = 6;
	for (int i = 0; i < nextCount; ++i) {
		queuedPiece(w, top + 3 * i, left, game, qdsGetNextPiece(game, i));
	}
}

void gameView(WINDOW *w, int top, int left, qdsGame *game)
{
	hold(w, top + 2, left + 2, game);
	next(w, top + 2, left + 34, game);
	playfield(w, top, left + 11, game);
}
