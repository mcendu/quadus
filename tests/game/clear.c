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
#include <string.h>

static qdsGame *game = &(qdsGame){ 0 };
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

const qdsLine emptyLine = { 0 };

START_TEST(clear)
{
	const qdsLine lines[] = {
		{ 0 },
		{ 8, 8, 8, 8, 8, 0, 8, 8, 8, 8 },
		{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 8 },
	};
	memcpy(game->playfield, lines, sizeof(lines));
	game->height = 4;

	/* filled line */
	ck_assert(qdsClearLine(game, 2));
	ck_assert_mem_eq(game->playfield[2], lines[3], sizeof(qdsLine));
	ck_assert_mem_eq(game->playfield[3], emptyLine, sizeof(qdsLine));
	ck_assert_int_eq(game->height, 3);

	/* partially-filled line */
	ck_assert(qdsClearLine(game, 1));
	ck_assert_mem_eq(game->playfield[1], lines[3], sizeof(qdsLine));
	ck_assert_mem_eq(game->playfield[2], emptyLine, sizeof(qdsLine));
	ck_assert_int_eq(game->height, 2);

	/* empty line below top row */
	ck_assert(qdsClearLine(game, 0));
	ck_assert_mem_eq(game->playfield[0], lines[3], sizeof(qdsLine));
	ck_assert_mem_eq(game->playfield[1], emptyLine, sizeof(qdsLine));
	ck_assert_int_eq(game->height, 1);

	/* empty line above top row */
	ck_assert(qdsClearLine(game, 5));
	ck_assert_int_eq(game->height, 1);
	ck_assert(qdsClearLine(game, game->height));
	ck_assert_int_eq(game->height, 1);
}
END_TEST

START_TEST(clearCeiling)
{
	memset(game->playfield[47], -1, sizeof(qdsLine));
	game->height = 48;

	ck_assert(qdsClearLine(game, 47));
	ck_assert_mem_eq(game->playfield[47], emptyLine, sizeof(qdsLine));
}
END_TEST

START_TEST(event)
{
	memset(game->playfield[0], -1, sizeof(qdsLine));
	game->height = 1;

	ck_assert_int_eq(rsData->lineClearCount, 0);
	ck_assert_int_eq(modeData->lineClearCount, 0);
	qdsClearLine(game, 0);
	ck_assert_int_ne(rsData->lineClearCount, 0);
	ck_assert_int_ne(modeData->lineClearCount, 0);
	ck_assert_int_eq(rsData->lineCleared, 0);
	ck_assert_int_eq(modeData->lineCleared, 0);
}
END_TEST

START_TEST(cancel)
{
	const qdsLine line = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	memcpy(game->playfield[0], line, sizeof(line));
	memcpy(game->playfield[1], line, sizeof(line));
	game->height = 2;

	rsData->blockLineClear = true;
	ck_assert(!qdsClearLine(game, 0));
	ck_assert_mem_eq(game->playfield[1], line, sizeof(line));
	ck_assert_int_eq(game->height, 2);
	rsData->blockLineClear = false;
	ck_assert(qdsClearLine(game, 0));
	ck_assert_mem_eq(game->playfield[1], emptyLine, sizeof(line));
	ck_assert_int_eq(game->height, 1);

	modeData->blockLineClear = true;
	ck_assert(!qdsClearLine(game, 0));
	ck_assert_mem_eq(game->playfield[0], line, sizeof(line));
	ck_assert_int_eq(game->height, 1);
	modeData->blockLineClear = false;
	ck_assert(qdsClearLine(game, 0));
	ck_assert_mem_eq(game->playfield[0], emptyLine, sizeof(line));
	ck_assert_int_eq(game->height, 0);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsClearLine");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, clear);
	tcase_add_test(c, clearCeiling);
	tcase_add_test(c, event);
	tcase_add_test(c, cancel);
	suite_add_tcase(s, c);

	return s;
}