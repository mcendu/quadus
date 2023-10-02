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
#include <check.h>

#include "mockruleset.h"
#include <game.h>
#include <piece.h>
#include <quadus.h>
#include <string.h>

static qdsGame game[1];

static void setup(void)
{
	qdsInitGame(game);
	qdsSetRuleset(game, mockRuleset);
	qdsSetMode(game, mockGamemode);
}

static void teardown(void)
{
	qdsCleanupGame(game);
}

START_TEST(inbounds)
{
	/*
	 *  QDS_API const piecedef qdsPieceO = {
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  };
	 */
	qdsLine *playfield = game->playfield;

	qdsSpawn(game, QDS_PIECE_O);
	ck_assert(!qdsOverlaps(game));
	game->x = 0;
	game->y = 0;
	ck_assert(!qdsOverlaps(game));

	/* set a single tile */
	playfield[4][4] = 1;

	ck_assert(qdsCanMove(game, 5, 5));
	ck_assert(qdsCanMove(game, 5, 4));
	ck_assert(qdsCanMove(game, 5, 3));
	ck_assert(qdsCanMove(game, 5, 2));
	ck_assert(qdsCanMove(game, 4, 2));
	ck_assert(qdsCanMove(game, 3, 2));
	ck_assert(qdsCanMove(game, 2, 2));
	ck_assert(qdsCanMove(game, 2, 3));
	ck_assert(qdsCanMove(game, 2, 4));
	ck_assert(qdsCanMove(game, 2, 5));
	ck_assert(qdsCanMove(game, 3, 5));
	ck_assert(qdsCanMove(game, 4, 5));

	ck_assert(!qdsCanMove(game, 4, 4));
	ck_assert(!qdsCanMove(game, 3, 4));
	ck_assert(!qdsCanMove(game, 3, 3));
	ck_assert(!qdsCanMove(game, 4, 3));

	ck_assert(qdsCanMove(game, 5, 6));
}
END_TEST

START_TEST(outofbounds)
{
	qdsSpawn(game, QDS_PIECE_O);
	ck_assert(!qdsOverlaps(game));
	game->x = 0;
	game->y = 0;
	ck_assert(!qdsOverlaps(game));

	/* below the ground */
	ck_assert(!qdsCanMove(game, 4, -1));
	/* the buffer zone has 48 lines */
	ck_assert(qdsCanMove(game, 4, 46));
	ck_assert(!qdsCanMove(game, 4, 48));

	/* left */
	ck_assert(!qdsCanMove(game, -1, 0));
	ck_assert(!qdsCanMove(game, -2, 0));
	/* right */
	ck_assert(qdsCanMove(game, 8, 0));
	ck_assert(!qdsCanMove(game, 9, 0));
	ck_assert(!qdsCanMove(game, 10, 0));
}
END_TEST

START_TEST(rotated)
{
	qdsSpawn(game, QDS_PIECE_T);
	game->y = 0;

	ck_assert(!qdsOverlaps(game));
	ck_assert(!qdsCanRotate(game, 0, 0, QDS_ROTATION_CLOCKWISE));
	ck_assert(!qdsCanRotate(game, 0, 0, QDS_ROTATION_COUNTERCLOCKWISE));
	ck_assert(!qdsCanRotate(game, 0, 0, 2));
}
END_TEST

TCase *caseOverlap(void)
{
	TCase *c = tcase_create("caseOverlap");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, inbounds);
	tcase_add_test(c, outofbounds);
	tcase_add_test(c, rotated);
	return c;
}
