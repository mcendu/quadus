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
#include "mockruleset.h"
#include "piece.h"
#include <check.h>
#include <game.h>
#include <mode.h>
#include <ruleset.h>
#include <stdbool.h>

static qdsGame gamed;
static qdsGame *game = &gamed;
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

START_TEST(test_spawnSet)
{
	ck_assert_int_eq(rsData->spawnCount, 0);
	ck_assert_int_eq(modeData->spawnCount, 0);
	ck_assert(qdsSpawn(game, QDS_PIECE_I));
	ck_assert_int_ne(rsData->spawnCount, 0);
	ck_assert_int_ne(modeData->spawnCount, 0);
	ck_assert_int_eq(rsData->spawnType, QDS_PIECE_I);

	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
	ck_assert_int_eq(game->x, 4);
	ck_assert_int_eq(game->y, 20);

	qdsSpawn(game, QDS_PIECE_J);
	ck_assert_int_eq(rsData->spawnType, QDS_PIECE_J);
	qdsSpawn(game, QDS_PIECE_O);
	ck_assert_int_eq(rsData->spawnType, QDS_PIECE_O);
}
END_TEST

START_TEST(test_spawnNext)
{
	/* calling spawn with 0 draws from the piece queue */
	ck_assert_int_eq(rsData->shiftCount, 0);
	ck_assert(qdsSpawn(game, 0));
	ck_assert_int_ne(rsData->shiftCount, 0);

	ck_assert_int_eq(game->piece, QDS_PIECE_O);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
	ck_assert_int_eq(game->x, 4);
	ck_assert_int_eq(game->y, 20);

	/* onSpawn should set off with the piece from the queue */
	ck_assert_int_eq(rsData->spawnType, QDS_PIECE_O);
}
END_TEST

START_TEST(test_blockSpawn)
{
	ck_assert(!rsData->blockSpawn);
	ck_assert(!modeData->blockSpawn);

	/*
	 * allowing spawn to be cancelled in ruleset is not useful, but
	 * simplifies implementation
	 */
	ck_assert(qdsSpawn(game, QDS_PIECE_I));
	rsData->blockSpawn = true;
	ck_assert(!qdsSpawn(game, QDS_PIECE_I));
	rsData->blockSpawn = false;

	ck_assert(qdsSpawn(game, QDS_PIECE_I));
	modeData->blockSpawn = true;
	ck_assert(!qdsSpawn(game, QDS_PIECE_I));
	modeData->blockSpawn = false;

	ck_assert(qdsSpawn(game, QDS_PIECE_I));
}
END_TEST

START_TEST(test_topOut)
{
	ck_assert_int_eq(rsData->topOutCount, 0);
	ck_assert_int_eq(modeData->topOutCount, 0);

	game->playfield[20][4] = QDS_PIECE_I;
	ck_assert(!qdsSpawn(game, QDS_PIECE_O));

	ck_assert_int_ne(rsData->topOutCount, 0);
	ck_assert_int_ne(modeData->topOutCount, 0);
}
END_TEST

void setupWithNoHandler(void)
{
	qdsInitGame(game);
	qdsSetRuleset(game, noHandlerRuleset);
	qdsSetMode(game, noHandlerGamemode);

	rsData = game->rsData;
	modeData = game->modeData;
}

START_TEST(test_noSpawnHandler)
{
	ck_assert_ptr_null(game->rs->onSpawn);
	ck_assert_ptr_null(game->mode->onSpawn);
	ck_assert_ptr_null(game->rs->onTopOut);
	ck_assert_ptr_null(game->mode->onTopOut);

	/* normally triggers spawn handler */
	ck_assert(qdsSpawn(game, QDS_PIECE_I));

	/* normally triggers topout handler */
	game->playfield[20][4] = QDS_PIECE_I;
	ck_assert(!qdsSpawn(game, QDS_PIECE_O));
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsSpawn");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, test_spawnSet);
	tcase_add_test(c, test_spawnNext);
	tcase_add_test(c, test_blockSpawn);
	tcase_add_test(c, test_topOut);
	suite_add_tcase(s, c);

	TCase *cNoHandler = tcase_create("no event handler");
	tcase_add_checked_fixture(cNoHandler, setupWithNoHandler, teardown);
	tcase_add_test(cNoHandler, test_noSpawnHandler);
	suite_add_tcase(s, cNoHandler);

	return s;
}
