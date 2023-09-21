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

#include <limits.h>

#include "mockruleset.h"
#include <game.h>
#include <quadus.h>

static qdsGame *game = &(qdsGame){};
static mockRulesetData *rsData;

static void setup(void)
{
	qdsInit(game);
	qdsSetRuleset(game, mockRuleset);
	qdsSetMode(game, mockGamemode);

	rsData = game->rsData;
}

static void teardown(void)
{
	qdsCleanup(game);
}

START_TEST(getPlayfield)
{
	void *p = qdsGetPlayfield(game);
	ck_assert_ptr_eq(p, &game->playfield);
}
END_TEST

START_TEST(getTile)
{
	game->playfield[5][9] = 8;
	ck_assert_int_eq(qdsGetTile(game, 9, 5), 8);

	/* qdsGetTile bound checks */
	ck_assert_int_eq(qdsGetTile(game, 2, -1), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 2, 48), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 2, INT_MIN), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 2, INT_MAX), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, -1, 3), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 10, 3), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, INT_MIN, 3), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, INT_MAX, 3), QDS_PIECE_WALL);
}
END_TEST

START_TEST(getActivePosition)
{
	int x, y;
	game->x = 12;
	game->y = 5;
	qdsGetActivePosition(game, &x, &y);
	ck_assert_int_eq(x, game->x);
	ck_assert_int_eq(y, game->y);
	ck_assert_int_eq(qdsGetActiveX(game), game->x);
	ck_assert_int_eq(qdsGetActiveY(game), game->y);
}
END_TEST

START_TEST(getNextPiece)
{
	ck_assert_int_eq(qdsGetNextPiece(game, 0), QDS_PIECE_O);
	ck_assert_int_eq(rsData->queuePos, 0);
	ck_assert_int_eq(qdsGetNextPiece(game, 5), QDS_PIECE_O);
	ck_assert_int_eq(rsData->queuePos, 5);
}
END_TEST

START_TEST(getHeldPiece)
{
	ck_assert_int_eq(qdsGetHeldPiece(game), game->hold);
	game->hold = QDS_PIECE_I;
	ck_assert_int_eq(qdsGetHeldPiece(game), game->hold);
}
END_TEST

START_TEST(getData)
{
	ck_assert_ptr_eq(qdsGetRulesetData(game), game->rsData);
	ck_assert_ptr_eq(qdsGetModeData(game), game->modeData);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsGame");

	TCase *c = tcase_create("properties");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, getPlayfield);
	tcase_add_test(c, getTile);
	tcase_add_test(c, getActivePosition);
	tcase_add_test(c, getNextPiece);
	tcase_add_test(c, getHeldPiece);
	tcase_add_test(c, getData);
	suite_add_tcase(s, c);

	return s;
}
