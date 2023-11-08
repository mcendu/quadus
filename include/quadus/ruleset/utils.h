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
#ifndef QDS__RULESET_UTILS_H
#define QDS__RULESET_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <quadus.h>
#include <quadus/ruleset/input.h>
#include <quadus/ruleset/linequeue.h>
#include <stdbool.h>

/**
 * The state of the ruleset.
 */
typedef struct qdsRulesetState
{
	/**
	 * Status of the game.
	 */
	unsigned short status;
	/**
	 * Time remaining for the current status.
	 */
	short statusTime;

	/**
	 * Subtile vertical position of the active mino.
	 */
	unsigned int subY;
	/**
	 * Number of game ticks left until lock.
	 */
	unsigned short lockTimer;
	/**
	 * Number of lock delay resets left.
	 */
	unsigned short resetsLeft;
	/**
	 * Input entered during delay.
	 */
	unsigned int delayInput;
	/**
	 * The result of the last twist check, or 0.
	 */
	unsigned int twistCheckResult;
	/**
	 * Properties of the last piece lock.
	 */
	unsigned int clearType;

	/**
	 * Queue of lines to be cleared.
	 */
	struct qdsPendingLines pendingLines;

	bool held : 1;
	bool b2b : 1;
	bool reset : 1;
	bool pause : 1;
} qdsRulesetState;

#define QDS_STATUS_INIT 0
#define QDS_STATUS_ACTIVE 1
#define QDS_STATUS_LOCKDELAY 2
#define QDS_STATUS_LINEDELAY 3
#define QDS_STATUS_PREGAME 4
#define QDS_STATUS_PAUSE 5
#define QDS_STATUS_GAMEOVER 6

/**
 * Initialize the base ruleset state.
 */
QDS_API void qdsInitRulesetState(qdsRulesetState *);

/**
 * Run a ruleset cycle with user-defined active cycle action.
 */
QDS_API void qdsRulesetCycle(qdsRulesetState *,
							 qdsGame *,
							 void (*activeCycle)(qdsRulesetState *,
												 qdsGame *,
												 unsigned int input),
							 unsigned int input);

/**
 * Spawn a new piece and transition to the active state.
 */
QDS_API void qdsProcessSpawn(qdsRulesetState *, qdsGame *, unsigned int input);
/**
 * Process hold input.
 */
QDS_API void qdsProcessHold(qdsRulesetState *, qdsGame *, unsigned int input);
/**
 * Process rotation inputs.
 */
QDS_API int qdsProcessRotation(qdsRulesetState *,
							   qdsGame *,
							   unsigned int input);
/**
 * Process movement inputs.
 */
QDS_API int qdsProcessMovement(qdsRulesetState *,
							   qdsGame *,
							   unsigned int input);
/**
 * Process gravity and soft drop.
 */
QDS_API void qdsProcessGravity(qdsRulesetState *,
							   qdsGame *,
							   unsigned int input);
/**
 * Process piece locking, transitioning to lock delay or line delay
 * depending on whether lines are cleared.
 */
QDS_API void qdsProcessLock(qdsRulesetState *, qdsGame *);
/**
 * Clear pending lines and enter lock delay.
 */
QDS_API void qdsProcessLineClear(qdsRulesetState *,
								 qdsGame *,
								 unsigned int input);

/**
 * Set appropriate states on spawn.
 */
QDS_API void qdsHandleSpawn(qdsRulesetState *);
/**
 * Determine the type of a piece lock or line clear and set the
 * lock type field appropriately.
 */
QDS_API unsigned int qdsCheckLockType(qdsRulesetState *, qdsGame *);

/**
 * Process activity made during a non-active state.
 */
QDS_API void qdsProcessDelay(qdsRulesetState *, qdsGame *, unsigned int input);

QDS_API int qdsUtilCallHandler(qdsRulesetState *,
							   qdsGame *,
							   unsigned long call,
							   void *argp);

#ifdef __cplusplus
}
#endif

#endif /* QDS__RULESET_UTILS_H */
