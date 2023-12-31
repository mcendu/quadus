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

#include <quadus.h>
#include <quadus/piece.h>
#include <quadus/piecegen/tgm.h>
#include <quadus/ruleset/input.h>
#include <quadus/ruleset/linequeue.h>
#include <quadus/ruleset/utils.h>

#define RSSYM(f) qdsRulesetArcade__##f

typedef struct arcadeData
{
	qdsRulesetState baseState;

	struct qdsInputState inputState;
	struct qdsTgmGen gen;

	unsigned int score;
	unsigned int combo;
	unsigned short softDistance;
	unsigned short sonicDistance;
} arcadeData;

extern const qdsPiecedef RSSYM(pieceT);
extern const qdsPiecedef RSSYM(pieceJ);
extern const qdsPiecedef RSSYM(pieceL);
extern const qdsPiecedef RSSYM(pieceO);
extern const qdsPiecedef RSSYM(pieceZ);
extern const qdsPiecedef RSSYM(pieceS);
extern const qdsPiecedef RSSYM(pieceI);

#endif /* !QDS__RULESET_ARCADE_H */
