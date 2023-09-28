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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "mockruleset.h"
#include <quadus.h>
#include <ruleset.h>
#include <ruleset/linequeue.h>

static qdsGame *game;
static struct qdsPendingLines *q;

static void setupCase(void)
{
	game = qdsAlloc();
	q = malloc(64);
	if (!game || !q) {
		fputs("Out of memory\n", stderr);
		abort();
	}
}

static void teardownCase(void)
{
	qdsFree(game);
}

static void setup(void)
{
	qdsSetRuleset(game, mockRuleset);
	qdsSetMode(game, mockGamemode);
	q->lines = 0;
}

START_TEST(addLine)
{
	qdsQueueLine(q, 1);
	ck_assert_int_eq(q->lines, 1);
	ck_assert_int_eq(q->h[0], 1);

	const unsigned char linelist[] = { 1, 5, 40, 20 };
	qdsQueueLine(q, 5);
	qdsQueueLine(q, 40);
	qdsQueueLine(q, 20);
	ck_assert_int_eq(q->lines, 4);
	ck_assert_mem_eq(q->h, linelist, 4);
}
END_TEST

START_TEST(process)
{
	const unsigned char pending[] = { 1, 3, 5, 9, 7, 19, 17, 11, 15, 13 };
	const qdsLine playfield[] = {
		{ 0, 8, 8, 8, 8, 8, 8, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 0, 8, 8, 8, 8, 8, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 0, 8, 8, 8, 8, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 0, 8, 8, 8, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 0, 8, 8, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 8, 0, 8, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 8, 8, 0, 8, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 8, 8, 8, 0, 8, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 8, 8, 8, 8, 0, 8 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 8, 8, 8, 8, 8, 8, 8, 8, 0, 8 },
	};

	memcpy(q->h, pending, sizeof(pending));
	q->lines = 10;
	memcpy(game->playfield, playfield, sizeof(playfield));
	game->height = 21;

	ck_assert_int_eq(qdsClearQueuedLines(game, q), 10);
	const qdsLine expected[] = {
		{ 0, 8, 8, 8, 8, 8, 8, 8, 8, 8 }, { 8, 0, 8, 8, 8, 8, 8, 8, 8, 8 },
		{ 8, 8, 0, 8, 8, 8, 8, 8, 8, 8 }, { 8, 8, 8, 0, 8, 8, 8, 8, 8, 8 },
		{ 8, 8, 8, 8, 0, 8, 8, 8, 8, 8 }, { 8, 8, 8, 8, 8, 0, 8, 8, 8, 8 },
		{ 8, 8, 8, 8, 8, 8, 0, 8, 8, 8 }, { 8, 8, 8, 8, 8, 8, 8, 0, 8, 8 },
		{ 8, 8, 8, 8, 8, 8, 8, 8, 0, 8 }, { 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
		{ 8, 8, 8, 8, 8, 8, 8, 8, 0, 8 },
	};
	ck_assert_mem_eq(game->playfield, expected, sizeof(expected));
	ck_assert_int_eq(q->lines, 0);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsLineQueue");

	TCase *c = tcase_create("base");
	tcase_add_unchecked_fixture(c, setupCase, teardownCase);
	tcase_add_checked_fixture(c, setup, NULL);
	tcase_add_test(c, addLine);
	tcase_add_test(c, process);
	suite_add_tcase(s, c);

	return s;
}
