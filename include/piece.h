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
/**
 * Data structures for minos.
 */
#ifndef QDS__PIECE_H
#define QDS__PIECE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <qdsbuild.h>

/**
 * Coordinates inside a piece definition.
 */
typedef struct
#if __STDC_VERSION__ > 201710L
	[[gnu::aligned(2)]]
#elif defined(__GNUC__)
	__attribute__((aligned(2)))
#endif
	qdsCoords
{
	signed char x;
	signed char y;
} qdsCoords;

/**
 * Definition for a piece.
 */
typedef struct qdsPiecedef
{
	const qdsCoords *base;
	const qdsCoords *cw;
	const qdsCoords *flip;
	const qdsCoords *ccw;
} qdsPiecedef;

/**
 * The empty piece. This should be assigned to piece 0 for all rulesets.
 */
QDS_API extern const qdsPiecedef qdsPieceNone;

#define QDS_PIECE_NONE 0

/*
 * Tetromino identifiers.
 */

#define QDS_PIECE_I 1
#define QDS_PIECE_J 2
#define QDS_PIECE_L 3
#define QDS_PIECE_O 4
#define QDS_PIECE_S 5
#define QDS_PIECE_T 6
#define QDS_PIECE_Z 7

/*
 * Standard tetromino shapes.
 */

QDS_API extern const qdsPiecedef qdsPieceI;
QDS_API extern const qdsPiecedef qdsPieceJ;
QDS_API extern const qdsPiecedef qdsPieceL;
QDS_API extern const qdsPiecedef qdsPieceO;
QDS_API extern const qdsPiecedef qdsPieceS;
QDS_API extern const qdsPiecedef qdsPieceT;
QDS_API extern const qdsPiecedef qdsPieceZ;

#ifdef __cplusplus
}
#endif

#endif /* !QDS__PIECE_H */
