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
#include <limits.h>
const struct speedData SHARED(speedData)[] = {
	{ 0, 1024 },	 { 30, 1536 },	   { 35, 2048 },		 { 40, 2560 },
	{ 50, 3072 },	 { 60, 4096 },	   { 70, 8192 },		 { 80, 12288 },
	{ 90, 16384 },	 { 100, 20480 },   { 120, 24576 },		 { 140, 28672 },
	{ 160, 32768 },	 { 170, 36864 },   { 200, 1024 },		 { 220, 8192 },
	{ 230, 16384 },	 { 233, 24576 },   { 236, 32768 },		 { 239, 40960 },
	{ 243, 49152 },	 { 247, 57344 },   { 251, 65536 },		 { 300, 131072 },
	{ 330, 196608 }, { 360, 262144 },  { 400, 327680 },		 { 420, 262144 },
	{ 450, 196608 }, { 500, 2097152 }, { INT_MAX, INT_MAX },
};

const struct timingData SHARED(timingData)[] = {
	{ 0, 16, 30, 40, 27, 27 },	 { 500, 10, 30, 25, 27, 27 },
	{ 600, 10, 30, 16, 27, 18 }, { 700, 10, 30, 12, 18, 14 },
	{ 800, 10, 30, 6, 14, 8 },	 { 900, 8, 17, 6, 14, 8 },
	{ 1000, 8, 17, 6, 8, 8 },	 { 1100, 8, 15, 6, 7, 7 },
	{ 1200, 8, 15, 6, 6, 6 },	 { SHRT_MAX, 1, 1, 1, 1, 1 },
};

const short SHARED(sectionThresholds)[]
	= { 100, 200, 300, 400, 500, 600, 700, 800, 900, 999, SHRT_MAX };
const short SHARED(sectionCoolThresholds)[]
	= { 70, 170, 270, 370, 470, 570, 670, 770, 870, SHRT_MAX, SHRT_MAX };

const int SHARED(baseSectionCoolTime)[] = {
	TIME(0, 50), TIME(0, 50), TIME(0, 47), TIME(0, 43),
	TIME(0, 43), TIME(0, 40), TIME(0, 40), TIME(0, 36),
	TIME(0, 36), 0,			  0,
};

const int SHARED(levelAdvance)[] = { 0, 1, 2, 4, 6 };
