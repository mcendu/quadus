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
#include <stdint.h>
#include <string.h>

#include "quadustui.h"
#include "ui.h"

static const char tileFilled[] = "[]";
static const char tileEmpty[] = ". ";
static const char tileGhost[] = "::";
static const char tileEmptyOverflow[] = "  ";

static const char *clearTypes[] = {
	"",			"SINGLE",	"DOUBLE",	"TRIPLE",  "QUADUS",  "QUINTO",
	"SEXTUS",	"SEPTIMUS", "OCTAVUS",	"NONUS",   "DECIMUS", "UNDECIM",
	"DUODECIM", "TRODECIM", "QUADECIM", "QUINDES", "SEDECIM", "SEPTADES",
	"OCTODES",	"NOVDECIM", "ULTIMUS",	"SUPER",   "HYPER",	  "DEITUS",
};

static const char *pieceNames[] = { "", "I", "J", "L", "O", "S", "T", "Z" };

static void clearTicker(WINDOW *w,
						int top,
						int left,
						unsigned clearType,
						int combo)
{
	int lines = clearType & QDS_LINECLEAR_MAX;
	if (lines >= 24) lines = 24;
	const char *clearName = clearTypes[lines];

	for (int i = 0; i < 7; ++i) mvwaddstr(w, top + i, left, "        ");

	if (combo > 1) {
		mvwprintw(w, top, left, "%8d", combo);
		mvwaddstr(w, top + 1, left + 3, "COMBO");
	}

	int row = top + 3;

	if (clearType & QDS_LINECLEAR_B2B) mvwaddstr(w, row++, left, "B2B");
	if (clearType & QDS_LINECLEAR_MINI) mvwaddstr(w, row++, left, "mini");
	if (clearType & QDS_LINECLEAR_TWIST) {
		wmove(w, row++, left);
		wprintw(w, "%s-SPIN", pieceNames[QDS_LINECLEAR_PIECE(clearType)]);
	}
	if (lines >= 4 || clearType & QDS_LINECLEAR_TWIST) {
		mvwaddstr(w, row++, left, clearName);
	}
}

static void gameOverBanner(WINDOW *w, int top, int left)
{
	mvwaddstr(w, top + 0, left + 1, "                    ");
	mvwaddstr(w, top + 1, left + 1, "     GAME  OVER     ");
	mvwaddstr(w, top + 2, left + 1, "                    ");
	mvwaddstr(w, top + 3, left + 1, "  Press Q to quit.  ");
	mvwaddstr(w, top + 4, left + 1, "                    ");
	wmove(w, top + 3, left + 19);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static void topOutAnimation(WINDOW *w, int top, int left, qdsGame *game)
{
	uiState *data = qdsGetUiData(game);
	const qdsLine *playfield = qdsGetPlayfield(game);

	const int playfieldBaseY = 21;
	int animationTime = data->time - data->topOutTime;

	for (unsigned y = 0; y < min(22, animationTime); ++y) {
		for (int x = 0; x < 10; ++x) {
			wmove(w, top + playfieldBaseY - y, left + 2 * x + 1);
			if (playfield[y][x]) waddstr(w, tileFilled);
		}
	}
}

inline static void playfieldLine(WINDOW *w,
								 const qdsTile *line,
								 uint_fast16_t visibility,
								 chtype boundary,
								 const char *tileFilled,
								 const char *tileEmpty)
{
	wattr_set(w, 0, 0, NULL);
	waddch(w, boundary);
	for (int x = 0; x < 10; ++x) {
		int tile = line[x];
		if (!(visibility & (1 << x))) tile = 0;

		const char *presented = tile ? tileFilled : tileEmpty;
		wattr_set(w, 0, tile % 8, NULL);
		waddstr(w, presented);
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

static void playfield(WINDOW *w, int top, int left, qdsGame *game)
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
	uiState *data = qdsGetUiData(game);
	if (data->useDisplayPlayfield)
		playfield = data->displayPlayfield;
	else
		playfield = qdsGetPlayfield(game);

	for (int y = 0; y < 20; ++y) {
		/* same pointer for both input and output: input is y, output is
		 * visibility */
		uint_fast16_t visibility = y;
		if (qdsCall(game, QDS_GETVISIBILITY, &visibility) < 0)
			visibility = 0x03ff; /* all visible except walls */
		wmove(w, top + playfieldBaseY - y, left);
		playfieldLine(
			w, playfield[y], visibility, ACS_VLINE, tileFilled, tileEmpty);
	}
	for (int y = 20; y < 22; ++y) {
		uint_fast16_t visibility = y;
		if (qdsCall(game, QDS_GETVISIBILITY, &visibility) < 0)
			visibility = 0x03ff;
		wmove(w, top + playfieldBaseY - y, left);
		playfieldLine(
			w, playfield[y], visibility, ' ', tileFilled, tileEmptyOverflow);
	}

	if (data->topOut) topOutAnimation(w, top, left, game);

	int x, y;
	x = left + 1 + 2 * qdsGetActiveX(game);

	/* ghost piece */
	bool showGhost;
	if (qdsCall(game, QDS_SHOWGHOST, &showGhost) < 0 || showGhost) {
		y = top + playfieldBaseY - qdsGetGhostY(game);
		piece(w, y, x, game, -1, -1, 0, tileGhost);
	}

	/* active piece */
	y = top + playfieldBaseY - qdsGetActiveY(game);
	piece(w, y, x, game, -1, -1, A_BOLD, tileFilled);
	wmove(w, y, x);

	if (data->topOut && data->time - data->topOutTime > 22)
		return gameOverBanner(w, top + 8, left);
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
	attr_t attrs = 0;
	if (qdsCall(game, QDS_CANHOLD, NULL) > 0) attrs |= A_BOLD;
	queuedPiece(w, top, left, game, qdsGetHeldPiece(game), attrs);
}

static void next(WINDOW *w, int top, int left, qdsGame *game)
{
	int nextCount;
	if (qdsCall(game, QDS_GETNEXTCOUNT, &nextCount) < 0) nextCount = 1;
	if (nextCount > 5) nextCount = 5;
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
	int timef = (time % 60) * 100 / 60;
	int times = (time / 60) % 60;
	int timem = time / (60 * 60);
	stat(w, top, left, name, "%02d:%02d.%02d", timem, times, timef);
}

/*
 * Exponential curve from 1092 (~1/60 tiles/cycle) to 1310720 (20 tiles/cycle)
 * f(x) = 1092e^{x(\ln(1310720) - \ln(1092))}
 */
static const int speedThresholds[]
	= { 2649, 6428, 15594, 37833, 91786, 222684, 540255, 1310720 };

static void speedBar(WINDOW *w, int top, int left, qdsGame *game)
{
	int gravity;
	if (qdsCall(game, QDS_GETGRAVITY, &gravity) < 0) gravity = 0;

	int i;
	wmove(w, top, left);
	wattr_set(w, 0, 8, NULL);
	for (i = 0; i < 8 && speedThresholds[i] <= gravity; ++i)
		waddch(w, ACS_HLINE);
	wattr_set(w, 0, 0, NULL);
	for (; i < 8; ++i) waddch(w, ACS_HLINE);
}

static void message(WINDOW *w, int top, int left, qdsGame *game)
{
	const char *message;
	if (qdsCall(game, QDS_GETMESSAGE, &message) < 0) message = "";
	int len = strlen(message);

	int i = 0, start = (20 - len) / 2;
	wmove(w, top, left);
	for (i = 0; i < 20; ++i) waddch(w, ' ');
	mvwaddstr(w, top, left + start, message);
}

void gameView(WINDOW *w, int top, int left, qdsGame *game)
{
	const char *grade;
	int score, time, level, target, combo;
	unsigned int clearType;
	if (qdsCall(game, QDS_GETSCORE, &score) < 0) score = 0;
	if (qdsCall(game, QDS_GETTIME, &time) < 0) time = 0;
	if (qdsCall(game, QDS_GETSUBLEVEL, &level) < 0) level = 0;
	if (qdsCall(game, QDS_GETLEVELTARGET, &target) < 0) target = 0;
	if (qdsCall(game, QDS_GETGRADETEXT, &grade) < 0) grade = NULL;
	if (qdsCall(game, QDS_GETCLEARTYPE, &clearType) < 0) clearType = 0;
	if (qdsCall(game, QDS_GETCOMBO, &combo) < 0) combo = 0;

	hold(w, top + 3, left + 2, game);
	clearTicker(w, top + 6, left + 2, clearType, combo);
	if (grade) stat(w, top + 13, left + 2, "GRADE", "%s", grade);
	stat(w, top + 16, left + 2, "SCORE", "%8d", score);
	statTime(w, top + 19, left + 2, "TIME", time);

	next(w, top + 3, left + 34, game);
	mvwprintw(w, top + 18, left + 34, "%6d", level);
	speedBar(w, top + 19, left + 34, game);
	if (target != 0) mvwprintw(w, top + 20, left + 34, "%6d", target);

	message(w, top + 23, left + 12, game);
	playfield(w, top, left + 11, game);
}
