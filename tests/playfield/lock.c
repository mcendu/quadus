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
#include <string.h>

#include "mockruleset.h"
#include <game.h>
#include <playfield.h>

static qdsPlayfield *game = &(qdsPlayfield){};
static mockRulesetData *rsData;
static mockRulesetData *modeData;

static void setup(void)
{
	qdsPlayfieldInit(game);
	qdsPlayfieldSetRuleset(game, mockRuleset);
	qdsPlayfieldSetMode(game, mockGamemode);

	rsData = game->rsData;
	modeData = game->modeData;
}

static void teardown(void)
{
	qdsPlayfieldCleanup(game);
}

const qdsTile emptyLine[10] = {};
const qdsTile lockedLine[10] = { 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 };

START_TEST(lock)
{
	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	int x = game->x;
	game->y = 0;
	ck_assert_int_eq(rsData->lockCount, 0);
	ck_assert_int_eq(modeData->lockCount, 0);
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_int_eq(game->x, x);
	ck_assert_int_eq(game->y, 0);
	ck_assert_int_eq(game->piece, 0);
	ck_assert_int_ne(rsData->lockCount, 0);
	ck_assert_int_ne(modeData->lockCount, 0);

	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);
	ck_assert_mem_eq(playfield[0], lockedLine, sizeof(playfield[0]));
	ck_assert_mem_eq(playfield[1], emptyLine, sizeof(playfield[1]));
}

/* locking is allowed mid-air */
START_TEST(lockAir)
{
	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 10;
	ck_assert_mem_eq(
		playfield[9], (const qdsTile[3][10]){}, sizeof(qdsTile[3][10]));
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_mem_eq(playfield[10], lockedLine, sizeof(playfield[10]));
}
END_TEST

/* locking is allowed while overlapping terrain */
START_TEST(lockTerrain)
{
	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);
	playfield[0][4] = SCHAR_MAX;

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 0;
	ck_assert(qdsPlayfieldOverlaps(game));
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_mem_eq(playfield[0], lockedLine, sizeof(playfield[0]));
	ck_assert_int_eq(playfield[0][4], QDS_PIECE_I);
}
END_TEST

START_TEST(lockOutOfBounds)
{
	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 10;
	game->x = -1;
	ck_assert(qdsPlayfieldLock(game));
	/* without checking, the piece overflows into the last row */
	ck_assert_mem_eq(playfield[9], emptyLine, sizeof(playfield[9]));

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 10;
	game->x = 9;
	ck_assert(qdsPlayfieldLock(game));
	/* same for the next row */
	ck_assert_mem_eq(playfield[11], emptyLine, sizeof(playfield[9]));

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = -1;
	ck_assert(qdsPlayfieldLock(game));
	/* if something goes wrong, asan would abort at this point */

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 48; /* above buffer zone */
	game->x = 1;
	ck_assert(qdsPlayfieldLock(game));
	/* coordinates come immediately after the playfield in the struct */
	ck_assert_int_eq(game->x, 1);
	ck_assert_int_eq(game->y, 48);
}

const qdsTile quad[][10] = {
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 0 },
};

START_TEST(fill)
{
	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);
	memcpy(playfield, quad, sizeof(quad));

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	ck_assert(qdsPlayfieldRotate(game, 1));
	ck_assert(qdsPlayfieldMove(game, 5));
	ck_assert(qdsPlayfieldDrop(game, QDS_DROP_HARD, 20));
	ck_assert_int_lt(game->y, 4);

	rsData->lineFillCount = 0;
	modeData->lineFillCount = 0;
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_int_eq(rsData->lineFillCount, 4);
	ck_assert_int_eq(modeData->lineFillCount, 4);
}
END_TEST

START_TEST(fillSplit)
{
	qdsTile(*playfield)[10] = qdsPlayfieldGetPlayfield(game);
	memcpy(playfield, quad, sizeof(quad));
	playfield[1][3] = 0;

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	ck_assert(qdsPlayfieldRotate(game, 1));
	ck_assert(qdsPlayfieldMove(game, 5));
	ck_assert(qdsPlayfieldDrop(game, QDS_DROP_HARD, 20));
	ck_assert_int_lt(game->y, 4);

	rsData->lineFillCount = 0;
	modeData->lineFillCount = 0;
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_int_eq(rsData->lineFillCount, 3);
	ck_assert_int_eq(modeData->lineFillCount, 3);
}
END_TEST

START_TEST(cancel)
{
	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 0;
	rsData->blockLock = true;
	ck_assert(!qdsPlayfieldLock(game));
	ck_assert_mem_eq(game->playfield[0], emptyLine, sizeof(qdsTile[10]));
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	rsData->blockLock = false;
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_mem_eq(game->playfield[0], lockedLine, sizeof(qdsTile[10]));
	ck_assert_int_eq(game->piece, 0);

	qdsPlayfieldSpawn(game, QDS_PIECE_I);
	game->y = 1;
	modeData->blockLock = true;
	ck_assert(!qdsPlayfieldLock(game));
	ck_assert_mem_eq(game->playfield[1], emptyLine, sizeof(qdsTile[10]));
	ck_assert_int_eq(game->piece, QDS_PIECE_I);
	modeData->blockLock = false;
	ck_assert(qdsPlayfieldLock(game));
	ck_assert_mem_eq(game->playfield[1], lockedLine, sizeof(qdsTile[10]));
	ck_assert_int_eq(game->piece, 0);
}
END_TEST

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsPlayfieldLock");

	TCase *c = tcase_create("base");
	tcase_add_checked_fixture(c, setup, teardown);
	tcase_add_test(c, lock);
	tcase_add_test(c, lockAir);
	tcase_add_test(c, lockTerrain);
	tcase_add_test(c, lockOutOfBounds);
	tcase_add_test(c, fill);
	tcase_add_test(c, fillSplit);
	tcase_add_test(c, cancel);
	suite_add_tcase(s, c);

	return s;
}
