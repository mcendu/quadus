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
#ifndef MODE_MASTER_H
#define MODE_MASTER_H

#include "piecegen/tgm.h"
#include <piecegen/his.h>
#include <quadus.h>
#include <stdbool.h>
#include <stdint.h>

struct modeData
{
	int time;
	int sectionTime;
	int lastCoolTime;

	short phase;
	short level;
	short section;
	short cools;

	short grade;
	short gradePoints;
	short decayTimer;
	short combo;

	short speedIndex;
	short timingIndex;

	int creditsPoints;

	short lines;
	unsigned char areType;
	bool held : 1;
	bool cool : 1;

	const char *message;
	int messageTime;

	int tileTime[48][10];

	struct qdsTgmGen gen;
};

#define SHARED(f) qdsModeMaster__##f

#define PHASE_MAIN 0
#define PHASE_CREDITS_FADE 1
#define PHASE_CREDITS_INVISIBLE 2
#define PHASE_GAME_OVER 3

#define ARE_TYPE_NORMAL 0
#define ARE_TYPE_LINE 1

#define TIME(m, s) (60 * ((s) + 60 * (m)))
#define FRAMETIME(m, s, f) ((f) + 60 * ((s) + 60 * (m)))

#define TIME_CREDITS_TRANSITION TIME(0, 3)

extern const struct speedData
{
	int level;
	int gravity;
} SHARED(speedData)[];

extern const struct timingData
{
	short level;
	short das;
	short lock;
	short lineClear;
	short are;
	short lineAre;
} SHARED(timingData)[];

extern const char *SHARED(gradeNames)[];

extern const short SHARED(sectionThresholds)[];
extern const short SHARED(sectionCoolThresholds)[];
extern const int SHARED(baseSectionCoolTime)[];
extern const int SHARED(levelAdvance)[];

extern const struct phase
{
	int (*getSpeed)(struct modeData *data);
	const struct timingData *(*getTimings)(struct modeData *data);
	uint_fast16_t (*getVisibility)(struct modeData *data, int line);

	void (*onCycle)(qdsGame *game, struct modeData *data);
	bool (*onSpawn)(qdsGame *game, struct modeData *data);
	void (*onHold)(qdsGame *game, struct modeData *data);
	void (*onLock)(qdsGame *game, struct modeData *data);
	void (*onLineFilled)(qdsGame *game, struct modeData *data, int y);
	void (*postLock)(qdsGame *game, struct modeData *data);
	void (*onLineClear)(qdsGame *game, struct modeData *data, int y);
} *SHARED(phases)[];

extern uint_fast16_t SHARED(visible)(struct modeData *, int);
extern uint_fast16_t SHARED(invisible)(struct modeData *, int);

extern int SHARED(getGrade)(struct modeData *data);
extern void SHARED(addGradePoints)(struct modeData *data, int lines);
extern void SHARED(checkRegret)(struct modeData *data);
extern void SHARED(decayGrade)(qdsGame *game, struct modeData *data);

extern const struct phase SHARED(phaseMain);
extern const struct phase SHARED(phaseCreditsFading);
extern const struct phase SHARED(phaseCreditsInvisible);
extern const struct phase SHARED(phaseGameOver);

#endif /* !MODE_MASTER_H */
