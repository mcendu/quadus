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
#include <calls.h>
#include <quadus.h>
#include <ruleset/input.h>

QDS_API unsigned int qdsFilterInput(qdsGame *game,
									qdsInputState *istate,
									unsigned int input)
{
	unsigned newbtn = input & (~istate->lastInput);
	istate->lastInput = input;

	if (newbtn & (QDS_INPUT_LEFT | QDS_INPUT_RIGHT)) {
		int das;
		if (!game || qdsCall(game, QDS_GETDAS, &das) < 0) das = QDS_DEFAULT_DAS;
		istate->repeatTimer = das;

		if (newbtn & (QDS_INPUT_LEFT)) {
			istate->direction = QDS_INPUT_LEFT;
			newbtn &= ~(QDS_INPUT_RIGHT);
		} else {
			istate->direction = QDS_INPUT_RIGHT;
		}

		return newbtn;
	}

	unsigned effective = newbtn;

	if (input & istate->direction) {
		istate->repeatTimer -= 1;
		if (istate->repeatTimer <= 0) {
			int arr;
			if (!game || qdsCall(game, QDS_GETARR, &arr) < 0)
				arr = QDS_DEFAULT_ARR;

			effective |= istate->direction;
			istate->repeatTimer = arr;
		}
	} else {
		int das;
		if (!game || qdsCall(game, QDS_GETDAS, &das) < 0) das = QDS_DEFAULT_DAS;

		/* there is only one other direction */
		istate->direction = input & (QDS_INPUT_LEFT | QDS_INPUT_RIGHT);
		istate->repeatTimer = das;
		effective |= istate->direction;
	}

	return effective;
}
