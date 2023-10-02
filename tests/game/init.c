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
#include "quadus.h"
#include <check.h>
#include <game.h>

static qdsGame gamed;
static qdsGame *game = &gamed;

void setup(void)
{
	qdsInitGame(game);
}

void teardown(void)
{
	qdsCleanupGame(game);
}

START_TEST(testInit)
{
	ck_assert_int_eq(game->hold, 0);
	ck_assert_int_eq(game->piece, QDS_PIECE_NONE);
	ck_assert_int_eq(game->orientation, QDS_ORIENTATION_BASE);
	ck_assert_ptr_null(game->rs);
	ck_assert_ptr_null(game->rsData);
	ck_assert_ptr_null(game->mode);
	ck_assert_ptr_null(game->modeData);
}
END_TEST

START_TEST(testSetRuleset)
{
	qdsSetRuleset(game, mockRuleset);
	ck_assert_ptr_eq(game->rs, mockRuleset);
	ck_assert_ptr_nonnull(game->rsData);
}
END_TEST

START_TEST(testSetGamemode)
{
	qdsSetMode(game, mockGamemode);
	ck_assert_ptr_eq(game->mode, mockGamemode);
	ck_assert_ptr_nonnull(game->modeData);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsInit");

	TCase *cInit = tcase_create("init");
	tcase_add_checked_fixture(cInit, setup, teardown);
	tcase_add_test(cInit, testInit);
	tcase_add_test(cInit, testSetRuleset);
	tcase_add_test(cInit, testSetGamemode);
	suite_add_tcase(s, cInit);

	return s;
}
