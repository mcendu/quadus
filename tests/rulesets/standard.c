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
#include <stdlib.h>

#include "mockruleset.h"
#include <quadus.h>
#include <ruleset.h>
#include <ruleset/standard.h>

static qdsGame *game;
static qdsGamemode mode;
static standardData *data;

static void setupCase(void)
{
	game = qdsAlloc();
	if (!game) abort();
}

static void teardownCase(void)
{
	qdsFree(game);
}

static void setup(void)
{
	memcpy(&mode, mockGamemode, sizeof(qdsGamemode));

	qdsInit(game);
	qdsSetRuleset(game, &qdsRulesetStandard);
	qdsSetMode(game, &mode);

	data = qdsGetRulesetData(game);
	qdsBagInit(&data->gen, 114514);
}

static void teardown(void)
{
	qdsCleanup(game);
}

START_TEST(base)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);

	qdsRunCycle(game, 0);
	ck_assert_int_eq(data->status, STATUS_ACTIVE);
	ck_assert_int_eq(qdsGetActivePieceType(game), 4);
	ck_assert_int_eq(qdsGetHeldPiece(game), 0);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_BASE);
	ck_assert_int_eq(qdsGetActiveX(game), 4);
	ck_assert_int_eq(qdsGetActiveY(game), 20);

	qdsRunCycle(game, QDS_INPUT_LEFT);
	ck_assert_int_eq(qdsGetActiveX(game), 3);
	qdsRunCycle(game, QDS_INPUT_RIGHT);
	ck_assert_int_eq(qdsGetActiveX(game), 4);
	qdsRunCycle(game, QDS_INPUT_ROTATE_C);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_C);
	qdsRunCycle(game, QDS_INPUT_ROTATE_CC);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_BASE);
	qdsRunCycle(game, 0);
	qdsRunCycle(game, QDS_INPUT_ROTATE_CC);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_CC);
	qdsRunCycle(game, QDS_INPUT_ROTATE_C);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_BASE);
	qdsRunCycle(game, QDS_INPUT_HOLD);
	ck_assert_int_eq(qdsGetActivePieceType(game), 3);
	ck_assert_int_eq(qdsGetHeldPiece(game), 4);
	qdsRunCycle(game, QDS_INPUT_HARD_DROP);
	ck_assert_int_eq(qdsGetActivePieceType(game), 6);
}
END_TEST

START_TEST(gravity)
{
	ck_assert_int_eq(data->subY, 0);

	qdsRunCycle(game, 0);
	ck_assert_int_eq(data->status, STATUS_ACTIVE);
	ck_assert_int_eq(qdsGetActiveY(game), 20);
	ck_assert_int_gt(data->subY, 0);

	for (int i = 0; i < 59; ++i) {
		qdsRunCycle(game, 0);
		ck_assert_int_eq(qdsGetActiveY(game), 20);
	}

	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActiveY(game), 19);

	qdsRunCycle(game, QDS_INPUT_SOFT_DROP);
	qdsRunCycle(game, QDS_INPUT_SOFT_DROP);
	ck_assert_int_eq(qdsGetActiveY(game), 18);
}
END_TEST

START_TEST(doubleRotation)
{
	unsigned input = QDS_INPUT_ROTATE_C;

	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_BASE);
	qdsRunCycle(game, input);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_C);
	input |= QDS_INPUT_ROTATE_CC;
	qdsRunCycle(game, input);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_BASE);
	qdsRunCycle(game, 0);

	/* clockwise prevails if two rotations are simultaneously pressed */
	qdsRunCycle(game, input);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_C);
}
END_TEST

START_TEST(irs)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);
	data->statusTime = 2;
	qdsRunCycle(game, QDS_INPUT_ROTATE_C);
	ck_assert_int_eq(data->delayInput, QDS_INPUT_ROTATE_C);
	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActivePieceType(game), 4);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_C);
}
END_TEST

START_TEST(irsChangeDirection)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);
	data->statusTime = 3;
	qdsRunCycle(game, QDS_INPUT_ROTATE_C);
	ck_assert_int_eq(data->delayInput, QDS_INPUT_ROTATE_C);
	qdsRunCycle(game, QDS_INPUT_ROTATE_CC);
	ck_assert_int_eq(data->delayInput, QDS_INPUT_ROTATE_CC);
	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActivePieceType(game), 4);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_CC);
}
END_TEST

START_TEST(irsDoubleDirection)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);
	data->statusTime = 2;
	qdsRunCycle(game, QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC);
	ck_assert_int_eq(data->delayInput,
					 QDS_INPUT_ROTATE_C | QDS_INPUT_ROTATE_CC);
	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActivePieceType(game), 4);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_C);
}
END_TEST

START_TEST(ihs)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);
	data->statusTime = 2;
	qdsRunCycle(game, QDS_INPUT_HOLD);
	ck_assert_int_eq(data->delayInput, QDS_INPUT_HOLD);
	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActivePieceType(game), 3);
	ck_assert_int_eq(qdsGetHeldPiece(game), 4);
}
END_TEST

START_TEST(ihsCancel)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);
	data->statusTime = 4;
	qdsRunCycle(game, QDS_INPUT_HOLD);
	ck_assert_int_eq(data->delayInput, QDS_INPUT_HOLD);
	qdsRunCycle(game, 0);
	qdsRunCycle(game, QDS_INPUT_HOLD);
	ck_assert_int_eq(data->delayInput, 0);
	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActivePieceType(game), 4);
	ck_assert_int_eq(qdsGetHeldPiece(game), 0);
}
END_TEST

START_TEST(irsihs)
{
	ck_assert_int_eq(data->status, STATUS_PREGAME);
	data->statusTime = 2;
	qdsRunCycle(game, QDS_INPUT_ROTATE_C | QDS_INPUT_HOLD);
	ck_assert_int_eq(data->delayInput, QDS_INPUT_ROTATE_C | QDS_INPUT_HOLD);
	qdsRunCycle(game, 0);
	ck_assert_int_eq(qdsGetActivePieceType(game), 3);
	ck_assert_int_eq(qdsGetHeldPiece(game), 4);
	ck_assert_int_eq(qdsGetActiveOrientation(game), QDS_ORIENTATION_C);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsRulesetStandard");

	TCase *c = tcase_create("base");
	tcase_add_unchecked_fixture(c, setupCase, teardownCase);
	tcase_add_test(c, base);
	tcase_add_test(c, gravity);
	tcase_add_test(c, doubleRotation);
	tcase_add_test(c, irs);
	tcase_add_test(c, irsChangeDirection);
	tcase_add_test(c, irsDoubleDirection);
	tcase_add_test(c, ihs);
	tcase_add_test(c, ihsCancel);
	tcase_add_test(c, irsihs);
	tcase_add_checked_fixture(c, setup, teardown);
	suite_add_tcase(s, c);

	return s;
}