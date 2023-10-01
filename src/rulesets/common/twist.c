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
#include <quadus.h>
#include <ruleset.h>
#include <ruleset/twist.h>

#include <stdbool.h>

QDS_API bool qdsCheckTwistImmobile(qdsGame *game, int x, int y, int r)
{
	return !qdsCanRotate(game, x, y - 1, r) && !qdsCanRotate(game, x, y + 1, r)
		   && !qdsCanRotate(game, x - 1, y, r)
		   && !qdsCanRotate(game, x + 1, y, r);
}

QDS_API bool qdsCheckTwist3Corner(qdsGame *game, int dx, int dy, int r)
{
	int x, y, corners = 0;
	qdsGetActivePosition(game, &x, &y);
	x += dx, y += dy;
	switch (qdsGetActivePieceType(game)) {
		case QDS_PIECE_J:
		case QDS_PIECE_L:
		case QDS_PIECE_T:
		case QDS_PIECE_S:
		case QDS_PIECE_Z:
			if (qdsGetTile(game, x - 1, y - 1)) ++corners;
			if (qdsGetTile(game, x - 1, y + 1)) ++corners;
			if (qdsGetTile(game, x + 1, y - 1)) ++corners;
			if (qdsGetTile(game, x + 1, y + 1)) ++corners;
			return corners >= 3;
		default:
			return false;
	}
}
