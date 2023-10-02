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

#include "game.h"
#include "mockruleset.h"
#include <quadus.h>
#include <ruleset.h>

static qdsGame *game = &(qdsGame){};
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
	const qdsLine lines[] = {
		{ 0, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
		{ 0, 0, 0, 8, 8, 8, 8, 8, 8, 8 },
	};
	const qdsLine emptyLine = { 0 };

	ck_assert_int_eq(game->height, 0);
	ck_assert(qdsAddLines(game, &lines[0], 1));
	ck_assert_int_eq(game->height, 1);
	ck_assert_mem_eq(game->playfield[0], lines[0], sizeof(qdsLine));
	ck_assert_mem_eq(game->playfield[1], emptyLine, sizeof(qdsLine));

	ck_assert(qdsAddLines(game, lines, 2));
	ck_assert_int_eq(game->height, 3);
	ck_assert_mem_eq(game->playfield[0], lines, sizeof(qdsLine[2]));
	ck_assert_mem_eq(game->playfield[2], lines[0], sizeof(qdsLine));
	ck_assert_mem_eq(game->playfield[3], emptyLine, sizeof(qdsLine));
}
END_TEST

START_TEST(topout)
{
	ck_assert_int_eq(rsData->topOutCount, 0);
	ck_assert_int_eq(modeData->topOutCount, 0);

	const qdsLine lines[] = {
		{ 0, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
		{ 0, 0, 0, 8, 8, 8, 8, 8, 8, 8 },
	};
	game->x = game->y = 4;
	for (int i = 0; i < 24; ++i) {
		ck_assert(qdsAddLines(game, lines, 2));
	}
	ck_assert(!qdsAddLines(game, lines, 2));
	ck_assert_int_eq(game->height, 48);
	ck_assert_int_eq(game->x, 4);
	ck_assert_int_eq(game->y, 4);
	ck_assert_int_ne(rsData->topOutCount, 0);
	ck_assert_int_ne(modeData->topOutCount, 0);
}
END_TEST

START_TEST(topoutOneshot)
{
	ck_assert_int_eq(rsData->topOutCount, 0);
	ck_assert_int_eq(modeData->topOutCount, 0);

	const qdsLine lines[64] = { 0 };
	game->x = game->y = 4;
	ck_assert(!qdsAddLines(game, lines, 64));
	ck_assert_int_eq(game->height, 48);
	ck_assert_int_eq(game->x, 4);
	ck_assert_int_eq(game->y, 4);
	ck_assert_int_ne(rsData->topOutCount, 0);
	ck_assert_int_ne(modeData->topOutCount, 0);
}
END_TEST

TCase *caseAddLines(void)
{
	TCase *c = tcase_create("caseAddLines");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, base);
	tcase_add_test(c, topout);
	tcase_add_test(c, topoutOneshot);
	return c;
}
