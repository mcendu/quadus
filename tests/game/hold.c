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

static qdsGame *game = &(qdsGame){ 0 };
static mockRulesetData *rsData;
static mockRulesetData *modeData;

static void setup(void)
{
	qdsInitGame(game);
	qdsSetRuleset(game, mockRuleset);
	qdsSetMode(game, mockGamemode);

	rsData = game->rsData;
	modeData = game->modeData;
}

static void teardown(void)
{
	qdsCleanupGame(game);
}

START_TEST(base)
{
	qdsSpawn(game, QDS_PIECE_I);

	ck_assert_int_eq(game->hold, 0);
	ck_assert_int_eq(rsData->spawnCount, 1);
	ck_assert_int_eq(rsData->holdCount, 0);
	ck_assert_int_eq(rsData->shiftCount, 0);
	ck_assert_int_eq(modeData->spawnCount, 1);
	ck_assert_int_eq(modeData->holdCount, 0);

	/* first hold grabs a piece from next */
	qdsHold(game);
	ck_assert_int_eq(game->piece, QDS_PIECE_O);
	ck_assert_int_eq(game->hold, QDS_PIECE_I);
	ck_assert_int_eq(rsData->holdCount, 1);
	ck_assert_int_eq(rsData->holdPiece, QDS_PIECE_I);
	ck_assert_int_eq(rsData->shiftCount, 1);
	ck_assert_int_eq(modeData->holdCount, 1);
	ck_assert_int_eq(modeData->holdPiece, QDS_PIECE_I);
	/* qdsHold spawns pieces */
	ck_assert_int_eq(rsData->spawnCount, 2);
	ck_assert_int_eq(modeData->spawnCount, 2);

	/* all later holds grab from hold */
	qdsHold(game);
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	ck_assert_int_eq(game->hold, QDS_PIECE_O);
	ck_assert_int_eq(rsData->holdCount, 2);
	ck_assert_int_eq(rsData->holdPiece, QDS_PIECE_O);
	ck_assert_int_eq(rsData->shiftCount, 1);
	ck_assert_int_eq(modeData->holdCount, 2);
	ck_assert_int_eq(modeData->holdPiece, QDS_PIECE_O);
	ck_assert_int_eq(rsData->spawnCount, 3);
	ck_assert_int_eq(modeData->spawnCount, 3);
}
END_TEST

START_TEST(topout)
{
	qdsSpawn(game, QDS_PIECE_I);
	ck_assert_int_eq(rsData->topOutCount, 0);
	ck_assert_int_eq(modeData->topOutCount, 0);

	game->playfield[20][4] = QDS_PIECE_I;
	ck_assert_int_eq(qdsHold(game), QDS_HOLD_TOPOUT);

	ck_assert_int_ne(rsData->topOutCount, 0);
	ck_assert_int_ne(modeData->topOutCount, 0);
}
END_TEST

START_TEST(cancel)
{
	qdsSpawn(game, QDS_PIECE_I);
	rsData->blockHold = true;
	ck_assert_int_eq(qdsHold(game), QDS_HOLD_BLOCKED);
	ck_assert_int_eq(game->hold, 0);
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	rsData->blockHold = false;

	modeData->blockHold = true;
	ck_assert_int_eq(qdsHold(game), QDS_HOLD_BLOCKED);
	ck_assert_int_eq(game->hold, 0);
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	modeData->blockHold = false;
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsHold");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, base);
	tcase_add_test(c, topout);
	tcase_add_test(c, cancel);
	suite_add_tcase(s, c);

	return s;
}
