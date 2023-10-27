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
#ifndef QDS__RULESET_ARCADE_H
#define QDS__RULESET_ARCADE_H

#include <piece.h>
#include <piecegen/tgm.h>
#include <quadus.h>
#include <ruleset/input.h>
#include <ruleset/linequeue.h>

#define RSSYM(f) qdsRulesetArcade__##f

typedef struct arcadeData
{
	unsigned int time;
	unsigned int lines;
	unsigned int score;
	unsigned int combo;

	/**
	 * Status of the game.
	 */
	unsigned short status;
	/**
	 * Time remaining for the current status.
	 */
	short statusTime;

	/**
	 * Subtile vertical position of the active mino.
	 */
	unsigned int subY;
	/**
	 * Number of game ticks left until lock.
	 */
	unsigned short lockTimer;
	/**
	 * Input entered during delay.
	 */
	unsigned int delayInput;

	/**
	 * The result of the last twist check or 0.
	 */
	unsigned int twistCheckResult;
	unsigned int clearType;

	bool held : 1;
	bool b2b : 1;
	bool pause : 1;

	struct qdsInputState inputState;
	struct qdsTgmGen gen;
	struct qdsPendingLines pendingLines;
} arcadeData;

enum gameStatus
{
	STATUS_PREGAME,
	STATUS_ACTIVE,
	STATUS_LOCKDELAY,
	STATUS_LINEDELAY,
	STATUS_GAMEOVER,
	STATUS_PAUSE,
};

extern const qdsPiecedef RSSYM(pieceT);
extern const qdsPiecedef RSSYM(pieceJ);
extern const qdsPiecedef RSSYM(pieceL);
extern const qdsPiecedef RSSYM(pieceO);
extern const qdsPiecedef RSSYM(pieceZ);
extern const qdsPiecedef RSSYM(pieceS);
extern const qdsPiecedef RSSYM(pieceI);

#endif /* !QDS__RULESET_ARCADE_H */
