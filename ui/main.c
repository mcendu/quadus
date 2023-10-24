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
#include <config.h>

#include "input/input.h"
#include "quadustui.h"
#include <curses.h>
#include <quadus.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SIGVBLANK SIGRTMIN

jmp_buf cleanupJump;

static void loop(int signo, siginfo_t *siginfo, void *p);
static void cleanup(int signo);

static inline void initInput(const struct inputHandler **handler, void **data)
{
#ifdef HAVE_LINUX_CONSOLE
	/* use console input; fallback to curses if not possible */
	*handler = &linuxConsoleInput;
	if (!(*data = linuxConsoleInput.init(0))) *handler = &cursesInput;
#else
	*handlerVar = &cursesInput;
#endif
}

int main(int argc, char **argv)
{
	sigaction(
		SIGVBLANK, &(const struct sigaction){ .sa_handler = SIG_IGN }, NULL);

	sigset_t vblankWaitSet;
	sigemptyset(&vblankWaitSet);
	sigaddset(&vblankWaitSet, SIGVBLANK);

	struct uiState state;
	state.time = 0;
	state.useDisplayPlayfield = false;
	state.topOut = false;
	state.lines.lines = 0;

	state.game = qdsNewGame();
	if (!state.game) abort();
	qdsSetRuleset(state.game, &qdsRulesetStandard);
	qdsSetMode(state.game, &qdsModeMaster);
	qdsSetUi(state.game, &ui, &state);

	timer_t frameTimer;
	struct sigevent frameTimerSigev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGVBLANK,
		.sigev_value = { .sival_ptr = &state },
	};
	if (timer_create(CLOCK_MONOTONIC, &frameTimerSigev, &frameTimer) < 0) {
		fprintf(
			stderr, "%s:%d: Failed to create game timer\n", __FILE__, __LINE__);
		exit(1);
	}

	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, true);
	nodelay(stdscr, true);
	refresh();

	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_WHITE, COLOR_BLACK);
	init_pair(5, COLOR_GREEN, COLOR_BLACK);
	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(7, COLOR_RED, COLOR_BLACK);
	init_pair(PAIR_ACCENT, COLOR_RED, COLOR_BLACK);

	initInput(&state.inputHandler, &state.inputData);
	state.input = 0;

	timer_settime(frameTimer, 0, &(const struct itimerspec){
        .it_value = {
            .tv_sec = 0,
            .tv_nsec = 16666667,
        },
        .it_interval = {
            .tv_sec = 0,
            .tv_nsec = 16666667,
        }
    }, NULL);

	int jmpval;
	if ((jmpval = setjmp(cleanupJump))) {
		endwin();
		if (state.inputHandler->cleanup)
			state.inputHandler->cleanup(state.inputData);
		qdsDestroyGame(state.game);
		timer_delete(frameTimer);

		if (jmpval != SIGINT) fprintf(stderr, "%s\n", strsignal(jmpval));

		exit(jmpval == SIGINT ? 0 : -jmpval);
	}

	signal(SIGABRT, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGBUS, cleanup);
	signal(SIGFPE, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGILL, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGPIPE, cleanup);
	signal(SIGPOLL, cleanup);
	signal(SIGPROF, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGSEGV, cleanup);
	signal(SIGSYS, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGTRAP, cleanup);
	signal(SIGTTIN, cleanup);
	signal(SIGTTOU, cleanup);
	signal(SIGVTALRM, cleanup);
	signal(SIGXCPU, cleanup);
	signal(SIGXFSZ, cleanup);

	sigaction(SIGVBLANK,
			  &(const struct sigaction){
				  .sa_flags = SA_SIGINFO,
				  .sa_sigaction = loop,
				  .sa_mask = vblankWaitSet,
			  },
			  NULL);

	while (1) {
		pause();
	}
}

static void loop(int signo, siginfo_t *siginfo, void *p)
{
	alarm(5);
	struct uiState *state = siginfo->si_value.sival_ptr;
	qdsGame *game = state->game;

	state->inputHandler->read(&state->input, state->inputData);
	qdsRunCycle(game, state->input);
	wnoutrefresh(stdscr);
	gameView(stdscr, 0, 0, game);
	doupdate();

	state->time++;
}

static void cleanup(int signo)
{
	longjmp(cleanupJump, signo);
}
