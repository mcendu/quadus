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
#include "master.h"
#include "quadus.h"

#include <limits.h>

const char *SHARED(gradeNames)[] = {
	"9",  "8",	"7",  "6",	"5",  "4",	"3",  "2",	"1",  "S1", "S2",
	"S3", "S4", "S5", "S6", "S7", "S8", "S9", "m1", "m2", "m3", "m4",
	"m5", "m6", "m7", "m8", "m9", "M",	"MK", "MV", "MO", "MM", "GM",
};

static const struct subgrade
{
	short grade;
	short decay;
	short regretDrop;
	short reserved;
	short points[4];
} gradeData[] = {
	{ 0, 125, 0, 0, { 10, 20, 40, 50 } }, { 1, 80, 0, 0, { 10, 20, 30, 40 } },
	{ 2, 80, 1, 0, { 10, 20, 30, 40 } },  { 3, 50, 2, 0, { 10, 15, 30, 40 } },
	{ 4, 45, 3, 0, { 10, 15, 20, 40 } },  { 5, 45, 4, 0, { 5, 15, 20, 30 } },
	{ 5, 45, 4, 0, { 5, 10, 20, 30 } },	  { 6, 40, 6, 0, { 5, 10, 15, 30 } },
	{ 6, 40, 6, 0, { 5, 10, 15, 30 } },	  { 7, 40, 8, 0, { 5, 10, 15, 30 } },
	{ 7, 40, 8, 0, { 2, 12, 13, 30 } },	  { 7, 40, 8, 0, { 2, 12, 13, 30 } },
	{ 8, 30, 11, 0, { 2, 12, 13, 30 } },  { 8, 30, 11, 0, { 2, 12, 13, 30 } },
	{ 8, 30, 11, 0, { 2, 12, 13, 30 } },  { 9, 20, 14, 0, { 2, 12, 13, 30 } },
	{ 9, 20, 14, 0, { 2, 12, 13, 30 } },  { 9, 20, 14, 0, { 2, 12, 13, 30 } },
	{ 10, 20, 17, 0, { 2, 12, 13, 30 } }, { 11, 20, 18, 0, { 2, 12, 13, 30 } },
	{ 12, 15, 19, 0, { 2, 12, 13, 30 } }, { 12, 15, 19, 0, { 2, 12, 13, 30 } },
	{ 12, 15, 19, 0, { 2, 12, 13, 30 } }, { 13, 15, 22, 0, { 2, 12, 13, 30 } },
	{ 13, 15, 22, 0, { 2, 12, 13, 30 } }, { 14, 15, 24, 0, { 2, 12, 13, 30 } },
	{ 14, 15, 24, 0, { 2, 12, 13, 30 } }, { 15, 15, 26, 0, { 2, 12, 13, 30 } },
	{ 15, 15, 26, 0, { 2, 12, 13, 30 } }, { 16, 15, 28, 0, { 2, 12, 13, 30 } },
	{ 16, 10, 28, 0, { 2, 12, 13, 30 } }, { 17, 10, 30, 0, { 0, 0, 0, 0 } },
};

static const unsigned char comboMultiplier[][4] = {
	{ 10, 10, 10, 10 }, { 10, 12, 14, 15 }, { 10, 12, 15, 18 },
	{ 10, 14, 16, 20 }, { 10, 14, 17, 22 }, { 10, 14, 18, 23 },
	{ 10, 14, 19, 24 }, { 10, 15, 20, 25 }, { 10, 15, 21, 26 },
	{ 10, 20, 25, 30 },
};

#define MIN(x, y) ((x) < (y) ? (x) : (y))

void SHARED(addGradePoints)(struct modeData *data, int lines)
{
	if (lines == 0) {
		data->combo = 0;
	} else {
		int l = lines - 1;
		if (l >= 4) l = 3;

		int points = gradeData[data->grade].points[l];

		points *= comboMultiplier[MIN(data->combo, 9)][l];
		/* divide by 10 and round up */
		points = (points + 9) / 10;

		points *= data->level / 250 + 1;

		data->gradePoints += points;
		if (data->gradePoints >= 100) {
			++data->grade;
			data->gradePoints = 0;
			data->decayTimer = 0;
		}

		if (lines >= 2) {
			data->combo += 1;
		}
	}
}

void SHARED(decayGrade)(qdsGame *game, struct modeData *data)
{
	if (!data->gradePoints || !qdsGetActivePieceType(game) || data->combo)
		return;
	data->decayTimer += 1;
	if (data->decayTimer >= gradeData[data->grade].decay) {
		data->decayTimer = 0;
		if (data->gradePoints > 0) --data->gradePoints;
	}
}

static const int regretTime[] = {
	TIME(1, 30), TIME(1, 15), TIME(1, 15), TIME(1, 8),	TIME(1, 0), TIME(1, 0),
	TIME(0, 50), TIME(0, 50), TIME(0, 50), TIME(0, 50), INT_MAX,
};

void SHARED(checkRegret)(struct modeData *data)
{
	if (data->sectionTime > regretTime[data->section]) {
		data->message = "Too bad!";
		data->messageTime = TIME(0, 3);

		/*
		 * If the player had 0 Internal Grades and thus 0 boosts, a REGRET has
		 * no effect, not even to drop the grade points
		 */
		if (data->grade == 0) return;

		data->grade = gradeData[data->grade].regretDrop;
		data->gradePoints = 0;
		data->decayTimer = 0;
	}
}

int SHARED(getGrade)(struct modeData *data)
{
	int grade = gradeData[data->grade].grade + data->cools
				+ (data->creditsPoints / 100);
	if (grade > 32) grade = 32;
	return grade;
}
