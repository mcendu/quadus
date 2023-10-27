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
#include "arcade.h"
#include "piece.h"
#include "quadus.h"

#define END      \
	{            \
		127, 127 \
	}

const qdsPiecedef RSSYM(pieceT) = {
	(const qdsCoords[]){ { 0, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { -1, 1 }, { 0, 1 }, { 0, 2 }, END },
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 0, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 0, 2 }, END },
};

const qdsPiecedef RSSYM(pieceJ) = {
	(const qdsCoords[]){ { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, END },
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, END },
};

const qdsPiecedef RSSYM(pieceL) = {
	(const qdsCoords[]){ { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 0, 1 }, { -1, 2 }, { 0, 2 }, END },
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 0, 2 }, END },
};

const qdsPiecedef RSSYM(pieceO) = {
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, END },
};

const qdsPiecedef RSSYM(pieceZ) = {
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, END },
	(const qdsCoords[]){ { -1, 0 }, { -1, 1 }, { 0, 1 }, { 0, 2 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 2 }, END },
};

const qdsPiecedef RSSYM(pieceS) = {
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 0, 0 }, { -1, 1 }, { 0, 1 }, { -1, 2 }, END },
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, END },
	(const qdsCoords[]){ { 1, 0 }, { 0, 1 }, { 1, 1 }, { 0, 2 }, END },
};

const qdsPiecedef RSSYM(pieceI) = {
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, END },
	(const qdsCoords[]){ { 0, -2 }, { 0, -1 }, { 0, 0 }, { 0, 1 }, END },
	(const qdsCoords[]){ { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, END },
	(const qdsCoords[]){ { 1, -2 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, END },
};
