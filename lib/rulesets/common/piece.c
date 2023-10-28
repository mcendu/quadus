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
#include <piece.h>
#include <qdsbuild.h>

#define END                  \
	{                        \
		SCHAR_MAX, SCHAR_MAX \
	}

typedef qdsPiecedef piecedef;
typedef qdsCoords coords;

QDS_API const piecedef qdsPieceNone = {
	(const coords[]){ END },
	(const coords[]){ END },
	(const coords[]){ END },
	(const coords[]){ END },
};

QDS_API const piecedef qdsPieceI = {
	(const coords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, END },
	(const coords[]){ { 1, -2 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, END },
	(const coords[]){ { -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, -1 }, END },
	(const coords[]){ { 0, -2 }, { 0, -1 }, { 0, 0 }, { 0, 1 }, END },
};

QDS_API const piecedef qdsPieceJ = {
	(const coords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, END },
	(const coords[]){ { 0, -1 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const coords[]){ { 1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, END },
	(const coords[]){ { -1, -1 }, { 0, -1 }, { 0, 0 }, { 0, 1 }, END },
};

QDS_API const piecedef qdsPieceL = {
	(const coords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, END },
	(const coords[]){ { 0, -1 }, { 1, -1 }, { 0, 0 }, { 0, 1 }, END },
	(const coords[]){ { -1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, END },
	(const coords[]){ { 0, -1 }, { 0, 0 }, { -1, 1 }, { 0, 1 }, END },
};

QDS_API const piecedef qdsPieceO = {
	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const coords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
};

QDS_API const piecedef qdsPieceS = {
	(const coords[]){ { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const coords[]){ { 1, -1 }, { 0, 0 }, { 1, 0 }, { 0, 1 }, END },
	(const coords[]){ { -1, -1 }, { 0, -1 }, { 0, 0 }, { 1, 0 }, END },
	(const coords[]){ { 0, -1 }, { -1, 0 }, { 0, 0 }, { -1, 1 }, END },
};

QDS_API const piecedef qdsPieceT = {
	(const coords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 0, 1 }, END },
	(const coords[]){ { 0, -1 }, { 0, 0 }, { 1, 0 }, { 0, 1 }, END },
	(const coords[]){ { 0, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, END },
	(const coords[]){ { 0, -1 }, { -1, 0 }, { 0, 0 }, { 0, 1 }, END },
};

QDS_API const piecedef qdsPieceZ = {
	(const coords[]){ { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, END },
	(const coords[]){ { 0, -1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, END },
	(const coords[]){ { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 }, END },
	(const coords[]){ { -1, -1 }, { -1, 0 }, { 0, 0 }, { 0, 1 }, END },
};
