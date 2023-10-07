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
#include <stdarg.h>

#include "quadustui.h"
#include "ui.h"

static const char tileFilled[] = "[]";
static const char tileEmpty[] = ". ";
static const char tileGhost[] = "::";
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
	wattr_set(w, 0, 0, NULL);
	waddch(w, boundary);
	for (int x = 0; x < 10; ++x) {
		const char *tile = line[x] ? tileFilled : tileEmpty;
		wattr_set(w, 0, line[x] % 8, NULL);
		waddstr(w, tile);
	}
	wattr_set(w, 0, 0, NULL);
	waddch(w, boundary);
}

static void piece(WINDOW *w,
				  int cy,
				  int cx,
				  const qdsGame *game,
				  int type,
				  int orientation,
				  attr_t attr,
				  const char *tile)
{
	if (type < 0) type = qdsGetActivePieceType(game);
	const qdsCoords *shape = qdsGetShape(game, type, orientation);

	wattr_set(w, attr, type % 8, NULL);
	QDS_SHAPE_FOREACH (i, shape) {
		int x = cx + 2 * i->x;
		int y = cy - i->y;
		mvwaddstr(w, y, x, tile);
	}
	wattr_set(w, 0, 0, NULL);
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
	const qdsLine *playfield;
	uiData *data = qdsGetUiData(game);
	if (data->useDisplayPlayfield)
		playfield = data->displayPlayfield;
	else
		playfield = qdsGetPlayfield(game);

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

	int x, y;
	x = left + 1 + 2 * qdsGetActiveX(game);
	/* ghost piece */
	y = top + playfieldBaseY - qdsGetGhostY(game);
	piece(w, y, x, game, -1, -1, 0, tileGhost);
	/* active piece */
	y = top + playfieldBaseY - qdsGetActiveY(game);
	piece(w, y, x, game, -1, -1, A_BOLD, tileFilled);
	wmove(w, y, x);
}

static void queuedPiece(WINDOW *w,
						int top,
						int left,
						qdsGame *game,
						int type,
						int attr)
{
	/* clear corresponding screen area */
	for (int y = 0; y < 3; ++y) mvwaddstr(w, top + y, left, "        ");

	int cx = left + 2;
	int cy = top + 1;
	piece(w, cy, cx, game, type, 0, attr, tileFilled);
}

static void hold(WINDOW *w, int top, int left, qdsGame *game)
{
	queuedPiece(w, top, left, game, qdsGetHeldPiece(game), A_BOLD);
}

static void next(WINDOW *w, int top, int left, qdsGame *game)
{
	int nextCount;
	if (qdsCall(game, QDS_GETNEXTCOUNT, &nextCount) < 0) nextCount = 1;
	if (nextCount > 6) nextCount = 6;
	for (int i = 0; i < nextCount; ++i) {
		attr_t attr = i == 0 ? A_BOLD : 0;
		queuedPiece(w, top + 3 * i, left, game, qdsGetNextPiece(game, i), attr);
	}
}

static void stat(WINDOW *w,
				 int top,
				 int left,
				 const char *name,
				 const char *format,
				 ...)
{
	va_list ap;
	va_start(ap, format);
	mvwaddstr(w, top, left, name);
	wmove(w, top + 1, left);
	vw_printw(w, format, ap);
	va_end(ap);
}

static void statTime(WINDOW *w, int top, int left, const char *name, int time)
{
	int times = (time / 60) % 60;
	int timem = time / (60 * 60);
	stat(w, top, left, name, "   %.2d:%.2d", timem, times);
}

void gameView(WINDOW *w, int top, int left, qdsGame *game)
{
	int score, time, lines;
	if (qdsCall(game, QDS_GETSCORE, &score) < 0) score = 0;
	if (qdsCall(game, QDS_GETTIME, &time) < 0) time = 0;
	if (qdsCall(game, QDS_GETLINES, &lines) < 0) lines = 0;
	stat(w, top + 12, left + 2, "LINES", "%8d", lines);
	stat(w, top + 15, left + 2, "SCORE", "%8d", score);
	statTime(w, top + 18, left + 2, "TIME", time);

	next(w, top + 2, left + 34, game);
	hold(w, top + 2, left + 2, game);
	playfield(w, top, left + 11, game);
}
