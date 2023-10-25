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
#include <piece.h>
#include <piecegen/tgm.h>
#include <quadus.h>

#define I QDS_PIECE_I
#define J QDS_PIECE_J
#define L QDS_PIECE_L
#define O QDS_PIECE_O
#define S QDS_PIECE_S
#define T QDS_PIECE_T
#define Z QDS_PIECE_Z

struct qdsTgmGen gen;

static void setup(void)
{
	qdsTgmGenInit(&gen, 114514);
}

static const qdsTile seq[][8] = {
	{ T, O, I, S, Z, J, L, T },
	{ I, O, Z, J, T, S, L, O },
	{ I, J, Z, L, S, T, O, Z },
};

START_TEST(peek)
{
	ck_assert_mem_eq(gen.queue, seq[0], 8);
	ck_assert_int_eq(gen.queueHead, 0);

	ck_assert_int_eq(qdsTgmGenPeek(&gen, 0), T);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 1), O);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 2), I);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 7), T);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 8), T);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 9), O);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 10), I);

	gen.queueHead = 3;

	ck_assert_int_eq(qdsTgmGenPeek(&gen, 0), S);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 1), Z);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 2), J);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 7), I);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 8), S);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 9), Z);
	ck_assert_int_eq(qdsTgmGenPeek(&gen, 10), J);
}
END_TEST

START_TEST(draw)
{
	for (int i = 0; i < 8; ++i) {
		ck_assert_int_eq(qdsTgmGenDraw(&gen), seq[0][i]);
		ck_assert_int_eq(gen.queue[i], seq[1][i]);
	}
	ck_assert_mem_eq(gen.queue, seq[1], 8);

	for (int i = 0; i < 8; ++i) {
		ck_assert_int_eq(qdsTgmGenDraw(&gen), seq[1][i]);
	}
	ck_assert_mem_eq(gen.queue, seq[2], 8);

	for (int i = 0; i < 8; ++i) {
		ck_assert_int_eq(qdsTgmGenDraw(&gen), seq[2][i]);
	}
}

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsBag");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, NULL);
	tcase_add_test(c, peek);
	tcase_add_test(c, draw);
	suite_add_tcase(s, c);

	return s;
}
