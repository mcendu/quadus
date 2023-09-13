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
#ifndef QDS__MINO_H
#define QDS__MINO_H

#ifdef __cplusplus
extern "C" {
#endif

#define QDS_ROTATE_NONE 0
#define QDS_ROTATE_CLOCKWISE 1
#define QDS_ROTATE_COUNTERCLOCKWISE -1

#define QDS_ORIENTATION_BASE 0
#define QDS_ORIENTATION_C 1
#define QDS_ORIENTATION_FLIP 2
#define QDS_ORIENTATION_CC 3

/* An active mino. */
typedef struct qdsMino
{
	/* The mino's type. */
	int type;
	/* The current orientation of the mino. */
	unsigned orientation;
} qdsMino;

#ifdef __cplusplus
}
#endif

#endif /* !QDS__MINO_H */
