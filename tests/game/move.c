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

#include <stdbool.h>

#include "mockruleset.h"
#include <game.h>
#include <quadus.h>

static qdsGame game[1];
static mockRulesetData *rsData;
static mockRulesetData *modeData;

static void setup(void)
{
	qdsInit(game);
	qdsSetRuleset(game, mockRuleset);
	qdsSetMode(game, mockGamemode);

	rsData = game->rsData;
	modeData = game->modeData;
}

static void teardown(void)
{
	qdsCleanup(game);
}

START_TEST(base)
{
	qdsSpawn(game, QDS_PIECE_O);
	ck_assert_int_eq(game->x, 4);

	/*
	 * |. . . . [][]. . . . |    |. . . . . [][]. . . |
	 * |. . . . [][]. . . . | => |. . . . . [][]. . . |
	 * |. . . . . . . . . . |    |. . . . . . . . . . |
	 */
	ck_assert_int_eq(rsData->moveCount, 0);
	ck_assert_int_eq(modeData->moveCount, 0);
	ck_assert_int_eq(qdsMove(game, 1), 1);
	ck_assert_int_eq(game->x, 5);
	ck_assert_int_ne(rsData->moveCount, 0);
	ck_assert_int_ne(modeData->moveCount, 0);
	ck_assert_int_eq(rsData->moveOffset, 1);
	ck_assert_int_eq(modeData->moveOffset, 1);

	/*
	 * |. . . . . [][]. . . |    |. . . . [][]. . . . |
	 * |. . . . . [][]. . . | => |. . . . [][]. . . . |
	 * |. . . . . . . . . . |    |. . . . . . . . . . |
	 */
	ck_assert_int_eq(qdsMove(game, -1), -1);
	ck_assert_int_eq(rsData->moveOffset, -1);
	ck_assert_int_eq(modeData->moveOffset, -1);
	ck_assert_int_eq(game->x, 4);

	/*
	 * |. . . . [][]. . . . |    |. . . . . . [][]. . |
	 * |. . . . [][]. . . . | => |. . . . . . [][]. . |
	 * |. . . . . . . . . . |    |. . . . . . . . . . |
	 */
	ck_assert_int_eq(qdsMove(game, 2), 2);
	ck_assert_int_eq(rsData->moveOffset, 2);
	ck_assert_int_eq(modeData->moveOffset, 2);
	ck_assert_int_eq(game->x, 6);
}
END_TEST

START_TEST(cancel)
{
	qdsSpawn(game, QDS_PIECE_O);
	ck_assert(!rsData->blockMove);
	ck_assert(!modeData->blockMove);
	ck_assert_int_eq(game->x, 4);

	rsData->blockMove = true;
	ck_assert_int_eq(qdsMove(game, -1), 0);
	ck_assert_int_eq(qdsMove(game, 1), 0);
	ck_assert_int_eq(qdsMove(game, -5), 0);
	ck_assert_int_eq(qdsMove(game, 10), 0);
	ck_assert_int_eq(game->x, 4);
	rsData->blockMove = false;
	ck_assert_int_eq(qdsMove(game, 1), 1);
	ck_assert_int_eq(game->x, 5);

	modeData->blockMove = true;
	ck_assert_int_eq(qdsMove(game, -1), 0);
	ck_assert_int_eq(qdsMove(game, 1), 0);
	ck_assert_int_eq(qdsMove(game, -5), 0);
	ck_assert_int_eq(qdsMove(game, 10), 0);
	ck_assert_int_eq(game->x, 5);
	modeData->blockMove = false;
	ck_assert_int_eq(qdsMove(game, -1), -1);
	ck_assert_int_eq(game->x, 4);
}

START_TEST(collision)
{
	qdsSpawn(game, QDS_PIECE_O);

	/*
	 * |. . . . [][]. . . . |    |. . . . . . . . [][]|
	 * |. . . . [][]. . . . | => |. . . . . . . . [][]|
	 * |. . . . . . . . . . |    |. . . . . . . . . . |
	 */
	ck_assert_int_eq(qdsMove(game, 4), 4);
	ck_assert_int_eq(qdsMove(game, 1), 0);
	ck_assert_int_eq(qdsMove(game, 4), 0);

	/*
	 * |. . . . . . . . [][]|    |[][]. . . . . . . . |
	 * |. . . . . . . . [][]| => |[][]. . . . . . . . |
	 * |. . . . . . . . . . |    |. . . . . . . . . . |
	 */
	ck_assert_int_eq(qdsMove(game, -15), -8);
	ck_assert_int_eq(rsData->moveOffset, -8);
	ck_assert_int_eq(modeData->moveOffset, -8);

	/*
	 * |[][]. . . . . . . . |    |. . . . [][]. . . . |
	 * |[][]. . . . []. . . | => |. . . . [][][]. . . |
	 * |. . . . . . . . . . |    |. . . . . . . . . . |
	 */
	qdsGetPlayfield(game)[game->y][6] = QDS_PIECE_I;
	ck_assert_int_eq(qdsMove(game, 10), 4);
	ck_assert_int_eq(rsData->moveOffset, 4);
	ck_assert_int_eq(modeData->moveOffset, 4);
}

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsMove");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, base);
	tcase_add_test(c, cancel);
	tcase_add_test(c, collision);
	suite_add_tcase(s, c);

	return s;
}
