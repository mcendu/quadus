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
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "mockruleset.h"

static qdsGame game[1];
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

START_TEST(rotate)
{
	qdsSpawn(game, QDS_PIECE_T);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);

	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_CC);
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);

	ck_assert(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_CC);
	ck_assert(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	ck_assert(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	ck_assert(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
}
END_TEST

START_TEST(event)
{
	qdsSpawn(game, QDS_PIECE_T);

	ck_assert_int_eq(rsData->rotateCount, 0);
	ck_assert_int_eq(modeData->rotateCount, 0);

	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_ne(rsData->rotateCount, 0);
	ck_assert_int_eq(rsData->rotation, QDS_ROTATION_CLOCKWISE);
	ck_assert_int_ne(modeData->rotateCount, 0);
	ck_assert_int_eq(modeData->rotation, QDS_ROTATION_CLOCKWISE);

	ck_assert(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE));
	ck_assert_int_eq(rsData->rotation, QDS_ROTATION_COUNTERCLOCKWISE);
	ck_assert_int_eq(modeData->rotation, QDS_ROTATION_COUNTERCLOCKWISE);
}
END_TEST

START_TEST(collision)
{
	qdsSpawn(game, QDS_PIECE_T);

	game->y = 0;
	ck_assert_int_eq(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE),
					 QDS_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
	ck_assert_int_eq(qdsRotate(game, QDS_ROTATION_COUNTERCLOCKWISE),
					 QDS_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
}
END_TEST

START_TEST(cancel)
{
	ck_assert(!rsData->blockRotate);
	ck_assert(!modeData->blockRotate);

	qdsSpawn(game, QDS_PIECE_T);
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	rsData->blockRotate = true;
	ck_assert_int_eq(qdsRotate(game, QDS_ROTATION_CLOCKWISE),
					 QDS_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	rsData->blockRotate = false;
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	modeData->blockRotate = true;
	ck_assert_int_eq(qdsRotate(game, QDS_ROTATION_CLOCKWISE),
					 QDS_ROTATE_FAILED);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_FLIP);
	modeData->blockRotate = false;
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_CC);
}
END_TEST

TCase *caseRotate(void)
{
	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, rotate);
	tcase_add_test(c, event);
	tcase_add_test(c, collision);
	tcase_add_test(c, cancel);
	return c;
}

static int canRotateWithKick(qdsGame *p, int r, int *x, int *y)
{
	if (!qdsCanRotate(p, 1, 1, r)) return QDS_ROTATE_FAILED;

	*x = 1;
	*y = 1;
	return QDS_ROTATE_NORMAL;
}

static qdsRuleset kickRuleset;

static void setupKickCaseUnchecked(void)
{
	memcpy(&kickRuleset, mockRuleset, sizeof(qdsRuleset));
	kickRuleset.canRotate = canRotateWithKick;
}

static void setupKickCase(void)
{
	qdsInitGame(game);
	qdsSetRuleset(game, &kickRuleset);
	qdsSetMode(game, mockGamemode);

	rsData = game->rsData;
	modeData = game->modeData;
}

START_TEST(kick)
{
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);

	int x = game->x, y = game->y;
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_C);
	ck_assert_int_eq(game->x, x + 1);
	ck_assert_int_eq(game->y, y + 1);
}
END_TEST

TCase *caseRotateWithKick(void)
{
	TCase *c = tcase_create("kick");
	tcase_add_unchecked_fixture(c, setupKickCaseUnchecked, NULL);
	tcase_add_checked_fixture(c, setupKickCase, teardown);
	tcase_add_test(c, kick);
	return c;
}
