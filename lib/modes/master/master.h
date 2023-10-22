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

#include <stdbool.h>

struct modeData
{
	int time;
	short phase;
	short level;
	short section;
	short cools;
	int sectionTime;
	int lastCoolTime;
	short speedIndex;
	short timingIndex;
	short lines;
	unsigned char areType;
	bool held : 1;
	bool cool : 1;
};

#define PHASE_MAIN 0
#define PHASE_CREDITS_FADE 1
#define PHASE_CREDITS_PHANTOM 2
#define PHASE_GAME_OVER 3

#define ARE_TYPE_NORMAL 0
#define ARE_TYPE_LINE 1

#define TIME(m, s) (60 * ((s) + 60 * (m)))

#endif /* !MODE_MASTER_H */
