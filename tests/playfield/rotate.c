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

#include <stdbool.h>

#include "mockruleset.h"
#include <game.h>
#include <piece.h>
#include <playfield.h>

static qdsPlayfield game[1];
static mockRulesetData *rsData;
static mockRulesetData *modeData;

static void setup(void)
{
	qdsPlayfieldInit(game);
	qdsPlayfieldSetRuleset(game, mockRuleset);
	qdsPlayfieldSetMode(game, mockGamemode);

	rsData = game->rsData;
	modeData = game->modeData;
}

static void teardown(void)
{
	qdsPlayfieldCleanup(game);
}

START_TEST(base)
{
	qdsPlayfieldSpawn(game, QDS_PIECE_T);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);

	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_CC);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);

	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_CC);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
}
END_TEST

START_TEST(collision)
{
	qdsPlayfieldSpawn(game, QDS_PIECE_T);

	game->y = 0;
	ck_assert_int_eq(qdsPlayfieldRotate(game, QDS_ROTATE_COUNTERCLOCKWISE),
					 QDS_PLAYFIELD_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
	ck_assert_int_eq(qdsPlayfieldRotate(game, QDS_ROTATE_COUNTERCLOCKWISE),
					 QDS_PLAYFIELD_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
}
END_TEST

START_TEST(cancel)
{
	ck_assert(!rsData->blockRotate);
	ck_assert(!modeData->blockRotate);

	qdsPlayfieldSpawn(game, QDS_PIECE_T);
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	rsData->blockRotate = true;
	ck_assert_int_eq(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE),
					 QDS_PLAYFIELD_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	rsData->blockRotate = false;
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	modeData->blockRotate = true;
	ck_assert_int_eq(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE),
					 QDS_PLAYFIELD_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	modeData->blockRotate = false;
	ck_assert(qdsPlayfieldRotate(game, QDS_ROTATE_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_CC);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsPlayfieldRotate");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, base);
	tcase_add_test(c, collision);
	tcase_add_test(c, cancel);
	suite_add_tcase(s, c);

	return s;
}
