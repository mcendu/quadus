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
#include <limits.h>

#include "game.h"
#include "mockruleset.h"

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

START_TEST(getPlayfield)
{
	const void *p = qdsGetPlayfield(game);
	ck_assert_ptr_eq(p, &game->playfield);
}
END_TEST

START_TEST(getTile)
{
	game->playfield[5][9] = 8;
	ck_assert_int_eq(qdsGetTile(game, 9, 5), 8);

	/* qdsGetTile bound checks */
	ck_assert_int_eq(qdsGetTile(game, 2, -1), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 2, 48), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 2, INT_MIN), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 2, INT_MAX), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, -1, 3), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, 10, 3), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, INT_MIN, 3), QDS_PIECE_WALL);
	ck_assert_int_eq(qdsGetTile(game, INT_MAX, 3), QDS_PIECE_WALL);
}
END_TEST

START_TEST(getActivePosition)
{
	int x, y;
	game->x = 12;
	game->y = 5;
	qdsGetActivePosition(game, &x, &y);
	ck_assert_int_eq(x, game->x);
	ck_assert_int_eq(y, game->y);
	ck_assert_int_eq(qdsGetActiveX(game), game->x);
	ck_assert_int_eq(qdsGetActiveY(game), game->y);
}
END_TEST

START_TEST(getShape)
{
	ck_assert_ptr_eq(qdsGetShape(game, QDS_PIECE_I, QDS_ORIENTATION_BASE),
					 qdsPieceI.base);
	ck_assert_ptr_eq(qdsGetShape(game, QDS_PIECE_J, QDS_ORIENTATION_FLIP),
					 qdsPieceJ.flip);
	ck_assert_ptr_eq(qdsGetShape(game, QDS_PIECE_L, QDS_ORIENTATION_C),
					 qdsPieceL.cw);
	ck_assert_ptr_eq(qdsGetShape(game, QDS_PIECE_O, QDS_ORIENTATION_CC),
					 qdsPieceO.ccw);

	game->piece = QDS_PIECE_J;
	game->orientation = QDS_ORIENTATION_CC;
	ck_assert_ptr_eq(qdsGetActiveShape(game), qdsPieceJ.ccw);
}

START_TEST(getActivePieceType)
{
	game->piece = QDS_PIECE_I;
	ck_assert_int_eq(qdsGetActivePieceType(game), game->piece);
}
END_TEST

START_TEST(getActiveOrientation)
{
	game->orientation = 1;
	ck_assert_int_eq(qdsGetActiveOrientation(game), 1);
	game->orientation = -1;
	ck_assert_int_eq(qdsGetActiveOrientation(game), 3);
}
END_TEST

START_TEST(getNextPiece)
{
	ck_assert_int_eq(qdsGetNextPiece(game, 0), QDS_PIECE_O);
	ck_assert_int_eq(rsData->queuePos, 0);
	ck_assert_int_eq(qdsGetNextPiece(game, 5), QDS_PIECE_O);
	ck_assert_int_eq(rsData->queuePos, 5);
}
END_TEST

START_TEST(getHeldPiece)
{
	ck_assert_int_eq(qdsGetHeldPiece(game), game->hold);
	game->hold = QDS_PIECE_I;
	ck_assert_int_eq(qdsGetHeldPiece(game), game->hold);
}
END_TEST

START_TEST(getData)
{
	ck_assert_ptr_eq(qdsGetRulesetData(game), game->rsData);
	ck_assert_ptr_eq(qdsGetModeData(game), game->modeData);
}
END_TEST

START_TEST(endGame)
{
	ck_assert_int_eq(rsData->topOutCount, 0);
	ck_assert_int_eq(modeData->topOutCount, 0);
	qdsEndGame(game);
	ck_assert_int_eq(rsData->topOutCount, 1);
	ck_assert_int_eq(modeData->topOutCount, 1);
}
END_TEST

TCase *caseProperties(void)
{
	TCase *c = tcase_create("caseProperties");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, getPlayfield);
	tcase_add_test(c, getTile);
	tcase_add_test(c, getActivePosition);
	tcase_add_test(c, getShape);
	tcase_add_test(c, getActivePieceType);
	tcase_add_test(c, getActiveOrientation);
	tcase_add_test(c, getNextPiece);
	tcase_add_test(c, getHeldPiece);
	tcase_add_test(c, getData);
	tcase_add_test(c, endGame);
	return c;
}
