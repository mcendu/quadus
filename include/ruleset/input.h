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
#ifndef QDS__RULESET_INPUT_H
#define QDS__RULESET_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../calls.h"
#include "../quadus.h"

#define QDS_DEFAULT_DAS 10
#define QDS_DEFAULT_ARR 2
#define QDS_DEFAULT_DCD 6

typedef struct qdsInputState qdsInputState;

struct qdsInputState
{
	unsigned int lastInput;
	short repeatTimer;
	short direction;
};

/**
 * Convert raw input to effective input.
 */
QDS_API unsigned int qdsFilterDirections(qdsGame *game,
										 qdsInputState *istate,
										 unsigned int input);

/**
 * Interrupt autorepeat and reset the repeat timer.
 */
inline static void qdsInterruptRepeat(qdsGame *game, qdsInputState *istate)
{
	int dcd;
	if (!game || qdsCall(game, QDS_GETDCD, &dcd) < 0) dcd = QDS_DEFAULT_DCD;
	istate->repeatTimer = dcd;
}

#ifdef __cplusplus
}
#endif

#endif /* !QDS__RULESET_INPUT_H */
