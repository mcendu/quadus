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
#include <quadus.h>
#include <quadus/piece.h>
#include <quadus/piecegen/bag.h>

#define I QDS_PIECE_I
#define J QDS_PIECE_J
#define L QDS_PIECE_L
#define O QDS_PIECE_O
#define S QDS_PIECE_S
#define T QDS_PIECE_T
#define Z QDS_PIECE_Z

struct qdsBag bag;

static void setup(void)
{
	qdsBagInit(&bag, 114514);
}

const qdsTile seq[][7] = {
	{ O, L, T, Z, I, J, S },
	{ I, J, Z, T, L, O, S },
	{ J, T, L, O, S, Z, I },
	{ Z, S, J, O, I, L, T },
};

START_TEST(peek)
{
	ck_assert_mem_eq(bag.pieces, seq, 14);
	ck_assert_int_eq(bag.head, 0);

	ck_assert_int_eq(qdsBagPeek(&bag, 0), 4);
	ck_assert_int_eq(qdsBagPeek(&bag, 1), 3);
	ck_assert_int_eq(qdsBagPeek(&bag, 2), 6);
	ck_assert_int_eq(qdsBagPeek(&bag, 3), 7);
	ck_assert_int_eq(qdsBagPeek(&bag, 7), 1);
	ck_assert_int_eq(qdsBagPeek(&bag, 14), 4);
	ck_assert_int_eq(qdsBagPeek(&bag, 15), 3);
	ck_assert_int_eq(qdsBagPeek(&bag, 16), 6);
	ck_assert_int_eq(qdsBagPeek(&bag, 17), 7);
}
END_TEST

START_TEST(draw)
{
	for (int i = 0; i < 7; ++i) {
		ck_assert_int_eq(qdsBagDraw(&bag), seq[0][i]);
	}
	ck_assert_mem_eq(bag.pieces, seq[2], 7);
	ck_assert_mem_eq(bag.pieces + 7, seq[1], 7);

	for (int i = 0; i < 7; ++i) {
		ck_assert_int_eq(qdsBagDraw(&bag), seq[1][i]);
	}
	ck_assert_mem_eq(bag.pieces, seq[2], 7);
	ck_assert_mem_eq(bag.pieces + 7, seq[3], 7);

	for (int i = 0; i < 7; ++i) {
		ck_assert_int_eq(qdsBagDraw(&bag), seq[2][i]);
	}
}
END_TEST

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
