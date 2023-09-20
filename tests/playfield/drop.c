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
#include <assert.h>
#include <check.h>

#include "mockruleset.h"
#include "piece.h"
#include <game.h>
#include <playfield.h>

static qdsPlayfield *game = &(qdsPlayfield){ 0 };
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

START_TEST(drop)
{
	qdsSpawn(game, QDS_PIECE_O);
	int y = game->y;

	ck_assert_int_eq(rsData->dropCount, 0);
	ck_assert_int_eq(modeData->dropCount, 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 1), 1);
	ck_assert_int_ne(rsData->dropCount, 0);
	ck_assert_int_ne(modeData->dropCount, 0);
	ck_assert_int_eq(game->y, y - 1);
	ck_assert_int_eq(rsData->dropDistance, 1);
	ck_assert_int_eq(modeData->dropDistance, 1);

	y = game->y;
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 0), 0);
	ck_assert_int_eq(game->y, y);
	ck_assert_int_eq(rsData->dropDistance, 0);
	ck_assert_int_eq(modeData->dropDistance, 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, -1), 0);
	ck_assert_int_eq(game->y, y);
	ck_assert_int_eq(rsData->dropDistance, 0);
	ck_assert_int_eq(modeData->dropDistance, 0);
}
END_TEST

START_TEST(edgeCollision)
{
	qdsSpawn(game, QDS_PIECE_O);
	ck_assert_int_ne(qdsDrop(game, QDS_DROP_GRAVITY, 100), 100);
	ck_assert_int_eq(game->y, 0);
	ck_assert_int_eq(rsData->dropDistance, 20);
	ck_assert_int_eq(modeData->dropDistance, 20);

	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 10), 0);
	ck_assert_int_eq(rsData->dropDistance, 0);
	ck_assert_int_eq(modeData->dropDistance, 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 100), 0);
	ck_assert_int_eq(rsData->dropDistance, 0);
	ck_assert_int_eq(modeData->dropDistance, 0);
}
END_TEST

START_TEST(tileCollision)
{
#define tileY 9
	qdsGetPlayfield(game)[tileY][4] = QDS_PIECE_I;
	qdsSpawn(game, QDS_PIECE_O);

	ck_assert_int_ne(qdsDrop(game, QDS_DROP_GRAVITY, 100), 100);
	ck_assert_int_eq(game->y, tileY + 1);
	ck_assert_int_eq(rsData->dropDistance, 20 - (tileY + 1));
	ck_assert_int_eq(modeData->dropDistance, 20 - (tileY + 1));

	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 10), 0);
	ck_assert_int_eq(rsData->dropDistance, 0);
	ck_assert_int_eq(modeData->dropDistance, 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 100), 0);
	ck_assert_int_eq(rsData->dropDistance, 0);
	ck_assert_int_eq(modeData->dropDistance, 0);
#undef tileY
}
END_TEST

START_TEST(cancel)
{
	qdsSpawn(game, QDS_PIECE_O);
	int y = game->y;
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 1), 1);

	qdsSpawn(game, QDS_PIECE_O);
	rsData->blockDrop = true;
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 1), 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 10), 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 100), 0);
	ck_assert_int_eq(game->y, y);
	rsData->blockDrop = false;
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 1), 1);
	ck_assert_int_ne(game->y, y);

	qdsSpawn(game, QDS_PIECE_O);
	y = game->y;
	modeData->blockDrop = true;
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 1), 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 10), 0);
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 100), 0);
	ck_assert_int_eq(game->y, y);
	modeData->blockDrop = false;
	ck_assert_int_eq(qdsDrop(game, QDS_DROP_GRAVITY, 1), 1);
	ck_assert_int_ne(game->y, y);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsDrop");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, drop);
	tcase_add_test(c, edgeCollision);
	tcase_add_test(c, tileCollision);
	tcase_add_test(c, cancel);
	suite_add_tcase(s, c);

	return s;
}
