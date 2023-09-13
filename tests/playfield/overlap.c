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
#include <playfield.h>
#include <string.h>

static qdsPlayfield game[1];

static void setup(void)
{
	qdsPlayfieldInit(game);
	qdsPlayfieldSetRuleset(game, mockRuleset);
	qdsPlayfieldSetMode(game, mockGamemode);
}

static void teardown(void)
{
	qdsPlayfieldCleanup(game);
}

START_TEST(testOverlapInBounds)
{
	/*
	 *  QDS_API const piecedef qdsPieceO = {
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	 *  };
	 */
	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);

	qdsPlayfieldSpawn(game, QDS_PIECE_O);
	ck_assert(!qdsPlayfieldOverlaps(game));
	game->x = 0;
	game->y = 0;
	ck_assert(!qdsPlayfieldOverlaps(game));

	/* set a single tile */
	playfield[4][4] = 1;

	ck_assert(qdsPlayfieldCanMove(game, 5, 5));
	ck_assert(qdsPlayfieldCanMove(game, 5, 4));
	ck_assert(qdsPlayfieldCanMove(game, 5, 3));
	ck_assert(qdsPlayfieldCanMove(game, 5, 2));
	ck_assert(qdsPlayfieldCanMove(game, 4, 2));
	ck_assert(qdsPlayfieldCanMove(game, 3, 2));
	ck_assert(qdsPlayfieldCanMove(game, 2, 2));
	ck_assert(qdsPlayfieldCanMove(game, 2, 3));
	ck_assert(qdsPlayfieldCanMove(game, 2, 4));
	ck_assert(qdsPlayfieldCanMove(game, 2, 5));
	ck_assert(qdsPlayfieldCanMove(game, 3, 5));
	ck_assert(qdsPlayfieldCanMove(game, 4, 5));

	ck_assert(!qdsPlayfieldCanMove(game, 4, 4));
	ck_assert(!qdsPlayfieldCanMove(game, 3, 4));
	ck_assert(!qdsPlayfieldCanMove(game, 3, 3));
	ck_assert(!qdsPlayfieldCanMove(game, 4, 3));

	ck_assert(qdsPlayfieldCanMove(game, 5, 6));
}
END_TEST

START_TEST(testOverlapOutOfBounds)
{
	qdsPlayfieldSpawn(game, QDS_PIECE_O);
	ck_assert(!qdsPlayfieldOverlaps(game));
	game->x = 0;
	game->y = 0;
	ck_assert(!qdsPlayfieldOverlaps(game));

	/* below the ground */
	ck_assert(!qdsPlayfieldCanMove(game, 4, -1));
	/* the buffer zone has 48 lines */
	ck_assert(qdsPlayfieldCanMove(game, 4, 46));
	ck_assert(!qdsPlayfieldCanMove(game, 4, 48));

	/* left */
	ck_assert(!qdsPlayfieldCanMove(game, -1, 0));
	ck_assert(!qdsPlayfieldCanMove(game, -2, 0));
	/* right */
	ck_assert(qdsPlayfieldCanMove(game, 8, 0));
	ck_assert(!qdsPlayfieldCanMove(game, 9, 0));
	ck_assert(!qdsPlayfieldCanMove(game, 10, 0));
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsPlayfieldCanRotate");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, testOverlapInBounds);
	tcase_add_test(c, testOverlapOutOfBounds);
	suite_add_tcase(s, c);

	return s;
}
