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

#include "input/input.h"
#include "quadustui.h"
#include "screen.h"
#include <curses.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_UDEV
#include <libudev.h>
#endif

#define POLL_COUNT 32

struct screenState
{
	struct pollfd devices[POLL_COUNT];
	size_t devCount;
#ifdef HAVE_UDEV
	struct udev *udev;
#endif
};

static void screenEnter(WINDOW *w, uiState *uiState)
{
	struct screenState *state = uiState->screenData
		= malloc(sizeof(struct screenState));
	if (!state) return;

	/* tear down any previously existing input handler */
	if (uiState->inputHandler && uiState->inputHandler->cleanup)
		uiState->inputHandler->cleanup(uiState->inputData);
	uiState->inputHandler = NULL;

	state->devCount = 0;

#if defined(HAVE_EVDEV) && defined(HAVE_UDEV)
	/* query udev for keyboards */
	state->udev = udev_new();
	state->devCount
		+= searchKeyboards(state->devices, POLL_COUNT - 1, state->udev);
#endif

	/* add stdin */
	state->devices[state->devCount].fd = STDIN_FILENO;
	state->devices[state->devCount].events = POLLIN;
	state->devCount += 1;
}

static void *initInput(uiState *uiState,
					   const struct inputHandler *handler,
					   int fd)
{
	if (handler->init) {
		uiState->inputData = handler->init(fd);
		if (!uiState->inputData) return NULL;
	}

	uiState->inputHandler = handler;
	return uiState->inputData;
}

static void gotoModeSelect(uiState *uiState)
{
	/* clear out all events to prevent ghost input */
	unsigned int input;
	uiState->inputHandler->read(&input, uiState->inputData);

	return changeScreen(uiState, &screenModeSelect);
}

static void screenUpdate(WINDOW *w, uiState *uiState)
{
	/* check for input */
	struct screenState *state = uiState->screenData;
	int fds = poll(state->devices, state->devCount, 0);

	if (fds > 0) {
		/* got input, search through the poll list */
#if defined(HAVE_EVDEV) && defined(HAVE_UDEV)
		/* check evdev devices */
		for (int i = 0; i < state->devCount - 1; ++i) {
			struct pollfd *pollfd = state->devices + i;
			if (pollfd->revents & POLLIN) {
				if (initInput(uiState, &evdevInput, pollfd->fd)) {
					return gotoModeSelect(uiState);
				}
			}
		}
#endif

		/* stdin is added last */
		struct pollfd *stdinPoll = state->devices + (state->devCount - 1);
		if (stdinPoll->revents & POLLIN) {
#ifdef HAVE_LINUX_CONSOLE
			initInput(uiState, &linuxConsoleInput, STDIN_FILENO);
#else
			/* for FreeBSD programmers' sanity's sake */
			uiState->inputData = NULL;
#endif
			if (!uiState->inputData)
				initInput(uiState, &cursesInput, STDIN_FILENO);

			return gotoModeSelect(uiState);
		}
	}

	/* draw a press any key note */
	mvwaddstr(w, 0, 0, "QUADUS");
	mvwaddstr(w, 2, 0, "Press any key");
}

static void screenExit(WINDOW *w, uiState *uiState)
{
	struct screenState *state = uiState->screenData;
#if defined(HAVE_EVDEV) && defined(HAVE_UDEV)
	udev_unref(state->udev);
#endif
	free(state);
}

const screen screenMainMenu = {
	.enter = screenEnter,
	.update = screenUpdate,
	.exit = screenExit,
};
