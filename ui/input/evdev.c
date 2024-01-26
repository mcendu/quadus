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
#include "evdev.h"
#include "config.h"
#include "input/input.h"
#include "quadus.h"
#include "quadustui.h"

#include <bits/time.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <linux/input.h>

struct inputData
{
	int fd;
	int oldStdinMode;
};

unsigned int mapEvdevInput(int key)
{
	unsigned int flag = 0;
	switch (key) {
		case KEY_LEFT:
			flag = QDS_INPUT_LEFT | INPUT_UI_LEFT;
			break;
		case KEY_RIGHT:
			flag = QDS_INPUT_RIGHT | INPUT_UI_RIGHT;
			break;
		case KEY_DOWN:
			flag = QDS_INPUT_SOFT_DROP | INPUT_UI_DOWN;
			break;
		case KEY_UP:
			flag = QDS_INPUT_HARD_DROP | INPUT_UI_UP;
			break;
		case KEY_SPACE:
			flag = QDS_INPUT_HARD_DROP | INPUT_UI_CONFIRM;
			break;
		case KEY_ENTER:
			flag = INPUT_UI_CONFIRM;
			break;
		case KEY_X:
			flag = QDS_INPUT_ROTATE_C;
			break;
		case KEY_Z:
		case KEY_C:
			flag = QDS_INPUT_ROTATE_CC;
			break;
		case KEY_LEFTSHIFT:
			flag = QDS_INPUT_HOLD;
			break;
		case KEY_ESC:
			flag = INPUT_UI_BACK | INPUT_UI_MENU;
			break;
		case KEY_Q:
			raise(SIGINT);
			break;
	}

	return flag;
}

static unsigned int syncInput(int fd)
{
	unsigned char keys[KEY_CNT / CHAR_BIT];
	unsigned int input = 0;

	ioctl(fd, EVIOCGKEY(sizeof(keys)), keys);

	if (BIT_GET(keys, KEY_LEFT)) input |= QDS_INPUT_LEFT | INPUT_UI_LEFT;
	if (BIT_GET(keys, KEY_RIGHT)) input |= QDS_INPUT_RIGHT | INPUT_UI_RIGHT;
	if (BIT_GET(keys, KEY_DOWN)) input |= QDS_INPUT_SOFT_DROP | INPUT_UI_DOWN;
	if (BIT_GET(keys, KEY_UP)) input |= QDS_INPUT_HARD_DROP | INPUT_UI_UP;
	if (BIT_GET(keys, KEY_SPACE))
		input |= QDS_INPUT_HARD_DROP | INPUT_UI_CONFIRM;
	if (BIT_GET(keys, KEY_ENTER)) input |= INPUT_UI_CONFIRM;
	if (BIT_GET(keys, KEY_X)) input |= QDS_INPUT_ROTATE_C;
	if (BIT_GET(keys, KEY_Z) || BIT_GET(keys, KEY_C))
		input |= QDS_INPUT_ROTATE_CC;
	if (BIT_GET(keys, KEY_LEFTSHIFT)) input |= QDS_INPUT_HOLD;
	if (BIT_GET(keys, KEY_ESC)) input |= INPUT_UI_BACK | INPUT_UI_MENU;

	return input;
}

static unsigned int readInput(unsigned int *prev, void *st)
{
	struct input_event ev;
	struct inputData *state = st;
	unsigned int input = *prev;
	struct
	{
		bool dropped : 1;
	} flags = { 0 };

	input &= QDS_INPUT_LEFT | QDS_INPUT_RIGHT | QDS_INPUT_SOFT_DROP
			 | QDS_INPUT_HARD_DROP;

	while (read(state->fd, &ev, sizeof(struct input_event)) > 0) {
		/* ignore everything if events are dropped */
		if (flags.dropped) continue;

		if (ev.type == EV_SYN) {
			if (ev.code == SYN_DROPPED) {
				input = syncInput(state->fd);
				flags.dropped = true;
			}
			continue;
		} else if (ev.type != EV_KEY) {
			continue; /* no support for non-button inputs */
		}

		unsigned int flags = mapEvdevInput(ev.code);
		if (ev.value == 0) {
			input &= ~flags;
		} else if (ev.value == 1) {
			input |= flags;
		}
	}

	/* clear out stdin */

	while (read(STDIN_FILENO, &ev, sizeof(struct input_event)) > 0)
		;

	*prev = input;
	return input;
}

static void *initInput(int fd)
{
	struct input_id id;
	if (ioctl(fd, EVIOCGID, &id) < 0) return NULL;

	struct inputData *data = malloc(sizeof(struct inputData));
	if (!data) return NULL;
	data->fd = fd;

	int clockMonotonic = CLOCK_MONOTONIC;
	ioctl(fd, EVIOCSCLOCKID, &clockMonotonic);

	int fileflags;
	if ((fileflags = fcntl(fd, F_GETFL)) < 0) {
		free(data);
		return NULL;
	}
	if (fcntl(fd, F_SETFL, fileflags | O_NONBLOCK) < 0) {
		free(data);
		return NULL;
	}

	data->oldStdinMode = fcntl(STDIN_FILENO, F_GETFL);
	if (data->oldStdinMode >= 0) {
		fcntl(STDIN_FILENO, F_SETFL, data->oldStdinMode | O_NONBLOCK);
	}

	return data;
}

static void cleanup(void *state)
{
	struct inputData *data = state;
	close(data->fd);
	if (data->oldStdinMode >= 0) {
		fcntl(STDIN_FILENO, F_SETFL, data->oldStdinMode);
	}
	free(data);
}

const struct inputHandler evdevInput = {
	.read = readInput,
	.init = initInput,
	.cleanup = cleanup,
};
