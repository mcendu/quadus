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
#include <stdalign.h>
#include <string.h>

#include "mockruleset.h"
#include <game.h>
#include <quadus.h>

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

const alignas(sizeof(qdsLine)) qdsLine emptyLine = {};
const alignas(sizeof(qdsLine)) qdsLine lockedLine
	= { 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 };

START_TEST(lock)
{
	qdsSpawn(game, QDS_PIECE_I);
	int x = game->x;
	game->y = 0;
	ck_assert_int_eq(rsData->lockCount, 0);
	ck_assert_int_eq(modeData->lockCount, 0);
	ck_assert(qdsLock(game));
	ck_assert_int_eq(game->x, x);
	ck_assert_int_eq(game->y, 0);
	ck_assert_int_eq(game->piece, 0);
	ck_assert_int_ne(rsData->lockCount, 0);
	ck_assert_int_ne(modeData->lockCount, 0);

	const qdsLine *playfield = qdsGetPlayfield(game);
	ck_assert_mem_eq(playfield[0], lockedLine, sizeof(qdsLine));
	ck_assert_mem_eq(playfield[1], emptyLine, sizeof(qdsLine));
}

/* locking is not allowed mid-air */
START_TEST(lockAir)
{
	const qdsLine *playfield = qdsGetPlayfield(game);

	qdsSpawn(game, QDS_PIECE_I);
	game->y = 10;
	ck_assert(!qdsLock(game));
	ck_assert_mem_eq(playfield[10], emptyLine, sizeof(qdsLine));
}
END_TEST

/* locking is allowed while overlapping terrain */
START_TEST(lockTerrain)
{
	qdsLine *playfield = game->playfield;
	playfield[0][4] = SCHAR_MAX;

	qdsSpawn(game, QDS_PIECE_I);
	game->y = 0;
	ck_assert(qdsOverlaps(game));
	ck_assert(qdsLock(game));
	ck_assert_mem_eq(playfield[0], lockedLine, sizeof(qdsLine));
	ck_assert_int_eq(playfield[0][4], QDS_PIECE_I);
}
END_TEST

/* locking out of bounds deletes out of bounds tiles */
START_TEST(lockOutOfBounds)
{
	const qdsLine leftCutoff = "\1\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	const qdsLine rightCutoff = "\0\0\0\0\0\0\0\0\1\1\0\0\0\0\0\0";
	qdsLine *playfield = game->playfield;

	qdsSpawn(game, QDS_PIECE_I);
	game->y = 10;
	game->x = -1;
	ck_assert(qdsLock(game));
	/* without checking, the piece overflows into the last row */
	ck_assert_mem_eq(playfield[9], emptyLine, sizeof(qdsLine));
	ck_assert_mem_eq(playfield[10], leftCutoff, sizeof(qdsLine));
	memset(playfield[10], 0, sizeof(qdsLine));

	qdsSpawn(game, QDS_PIECE_I);
	game->y = 10;
	game->x = 9;
	ck_assert(qdsLock(game));
	/* without checking, overflows into padding for the next row */
	ck_assert_mem_eq(playfield[10], rightCutoff, sizeof(qdsLine));

	qdsSpawn(game, QDS_PIECE_I);
	game->y = -1;
	ck_assert(qdsLock(game));
	/* if something goes wrong, asan would abort at this point */

	qdsSpawn(game, QDS_PIECE_I);
	game->y = 48; /* above buffer zone */
	game->x = 1;
	ck_assert(qdsLock(game));
	/* coordinates come immediately after the playfield in the struct */
	ck_assert_int_eq(game->x, 1);
	ck_assert_int_eq(game->y, 48);
}

const qdsLine quad[] = {
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
};

START_TEST(fill)
{
	memcpy(game->playfield, quad, sizeof(quad));

	qdsSpawn(game, QDS_PIECE_I);
	ck_assert(qdsRotate(game, 1));
	ck_assert(qdsMove(game, 5));
	ck_assert(qdsDrop(game, QDS_DROP_HARD, 48));
	ck_assert_int_lt(game->y, 4);

	rsData->lineFillCount = 0;
	modeData->lineFillCount = 0;
	ck_assert(qdsLock(game));
	ck_assert_int_eq(rsData->lineFillCount, 4);
	ck_assert_int_eq(modeData->lineFillCount, 4);
}
END_TEST

START_TEST(fillSplit)
{
	qdsLine *playfield = game->playfield;
	memcpy(playfield, quad, sizeof(quad));
	playfield[1][3] = 0;

	qdsSpawn(game, QDS_PIECE_I);
	ck_assert(qdsRotate(game, 1));
	ck_assert(qdsMove(game, 5));
	ck_assert(qdsDrop(game, QDS_DROP_HARD, 48));
	ck_assert_int_lt(game->y, 4);

	rsData->lineFillCount = 0;
	modeData->lineFillCount = 0;
	ck_assert(qdsLock(game));
	ck_assert_int_eq(rsData->lineFillCount, 3);
	ck_assert_int_eq(modeData->lineFillCount, 3);
}
END_TEST

START_TEST(heightTracking)
{
	ck_assert_int_eq(game->height, 0);
	ck_assert(qdsSpawn(game, QDS_PIECE_L));
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert(qdsDrop(game, QDS_DROP_HARD, 48));
	ck_assert(qdsLock(game));
	ck_assert_int_eq(game->height, 3);
	ck_assert(qdsSpawn(game, QDS_PIECE_I));
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	ck_assert(qdsDrop(game, QDS_DROP_HARD, 48));
	ck_assert(qdsLock(game));
	ck_assert_int_eq(game->height, 5);

	ck_assert(qdsSpawn(game, QDS_PIECE_I));
	ck_assert(qdsRotate(game, QDS_ROTATION_CLOCKWISE));
	game->y = 4;
	ck_assert(qdsLock(game));
	ck_assert_int_eq(game->height, 6);
}
END_TEST

START_TEST(cancel)
{
	qdsSpawn(game, QDS_PIECE_I);
	game->y = 0;
	rsData->blockLock = true;
	ck_assert(!qdsLock(game));
	ck_assert_mem_eq(game->playfield[0], emptyLine, sizeof(qdsLine));
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	rsData->blockLock = false;
	ck_assert(qdsLock(game));
	ck_assert_mem_eq(game->playfield[0], lockedLine, sizeof(qdsLine));
	ck_assert_int_eq(game->piece, 0);

	qdsSpawn(game, QDS_PIECE_I);
	game->y = 1;
	modeData->blockLock = true;
	ck_assert(!qdsLock(game));
	ck_assert_mem_eq(game->playfield[1], emptyLine, sizeof(qdsLine));
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	modeData->blockLock = false;
	ck_assert(qdsLock(game));
	ck_assert_mem_eq(game->playfield[1], lockedLine, sizeof(qdsLine));
	ck_assert_int_eq(game->piece, 0);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsLock");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, lock);
	tcase_add_test(c, lockAir);
	tcase_add_test(c, lockTerrain);
	tcase_add_test(c, lockOutOfBounds);
	tcase_add_test(c, fill);
	tcase_add_test(c, fillSplit);
	tcase_add_test(c, heightTracking);
	tcase_add_test(c, cancel);
	suite_add_tcase(s, c);

	return s;
}
