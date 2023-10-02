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
#include <check.h>

extern TCase *caseAddLines(void);
extern TCase *caseClear(void);
extern TCase *caseDrop(void);
extern TCase *caseHold(void);
extern TCase *caseInit(void);
extern TCase *caseLock(void);
extern TCase *caseMove(void);
extern TCase *caseOverlap(void);
extern TCase *caseProperties(void);
extern TCase *caseRotate(void);
extern TCase *caseRotateWithKick(void);
extern TCase *caseSpawn(void);
extern TCase *caseSpawnNoHandler(void);

Suite *createSuite(void)
{
	Suite *s = suite_create("qdsGame");
	suite_add_tcase(s, caseAddLines());
	suite_add_tcase(s, caseClear());
	suite_add_tcase(s, caseDrop());
	suite_add_tcase(s, caseHold());
	suite_add_tcase(s, caseInit());
	suite_add_tcase(s, caseLock());
	suite_add_tcase(s, caseMove());
	suite_add_tcase(s, caseOverlap());
	suite_add_tcase(s, caseProperties());
	suite_add_tcase(s, caseRotate());
	suite_add_tcase(s, caseRotateWithKick());
	suite_add_tcase(s, caseSpawn());
	suite_add_tcase(s, caseSpawnNoHandler());
	return s;
}
