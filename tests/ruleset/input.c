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

#include <string.h>

#include <quadus.h>
#include <ruleset/input.h>

qdsInputState *istate = &(qdsInputState){};

static void setup(void)
{
	memset(istate, 0, sizeof(qdsInputState));
}

static void teardown(void) {}

START_TEST(base)
{
	ck_assert_int_eq(qdsFilterInput(NULL, istate, QDS_INPUT_LEFT),
					 QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->direction, QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->lastInput, QDS_INPUT_LEFT);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, 0), 0);
	ck_assert_int_eq(istate->direction, 0);
	ck_assert_int_eq(istate->lastInput, 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, QDS_INPUT_RIGHT),
					 QDS_INPUT_RIGHT);
	ck_assert_int_eq(istate->direction, QDS_INPUT_RIGHT);
	ck_assert_int_eq(istate->lastInput, QDS_INPUT_RIGHT);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, QDS_INPUT_LEFT),
					 QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->direction, QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->lastInput, QDS_INPUT_LEFT);
	ck_assert_int_eq(
		qdsFilterInput(NULL, istate, QDS_INPUT_HOLD | QDS_INPUT_ROTATE_C),
		QDS_INPUT_HOLD | QDS_INPUT_ROTATE_C);
}
END_TEST

START_TEST(autorepeat)
{
	/* directions autorepeat on hold */
	unsigned input = QDS_INPUT_LEFT;
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);

	for (int i = 0; i < QDS_DEFAULT_DAS - 1; ++i)
		ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_ARR);
	for (int i = 0; i < QDS_DEFAULT_ARR - 1; ++i)
		ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_ARR);

	/* autorepeat resets when another direction is pressed... */
	input = QDS_INPUT_RIGHT;
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_RIGHT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);
	for (int i = 0; i < QDS_DEFAULT_DAS - 1; ++i)
		ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_RIGHT);

	/* ...or when all directions are released */
	ck_assert_int_eq(qdsFilterInput(NULL, istate, 0), 0);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);
}
END_TEST

START_TEST(autorepeatWithNonDirection)
{
	/* nothing else autorepeats */
	unsigned input = QDS_INPUT_HARD_DROP;
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_HARD_DROP);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);
	for (int i = 0; i < QDS_DEFAULT_DAS - 1; ++i)
		ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);

	/* not even when directions are thrown in the mix */
	input |= QDS_INPUT_LEFT;
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);

	for (int i = 0; i < QDS_DEFAULT_DAS - 1; ++i)
		ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_ARR);
	for (int i = 0; i < QDS_DEFAULT_ARR - 1; ++i)
		ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_ARR);
}
END_TEST

START_TEST(oppositeDirections)
{
	/* when both left and right are active, latest press takes priority */
	unsigned input = QDS_INPUT_LEFT;
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	input |= QDS_INPUT_RIGHT;
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_RIGHT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), 0);
	input &= ~(QDS_INPUT_RIGHT);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, input), QDS_INPUT_LEFT);
	ck_assert_int_eq(istate->repeatTimer, QDS_DEFAULT_DAS);
	ck_assert_int_eq(qdsFilterInput(NULL, istate, 0), 0);

	/* left takes priority in simultaneous press */
	ck_assert_int_eq(
		qdsFilterInput(NULL, istate, QDS_INPUT_LEFT | QDS_INPUT_RIGHT),
		QDS_INPUT_LEFT);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsFilterInput");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, base);
	tcase_add_test(c, autorepeat);
	tcase_add_test(c, autorepeatWithNonDirection);
	tcase_add_test(c, oppositeDirections);
	suite_add_tcase(s, c);

	return s;
}
