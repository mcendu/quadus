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
#include "input.h"
#include "quadustui.h"

#include <fcntl.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <linux/kd.h>
#include <quadus.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct linuxInputData
{
	int fd;
	long oldkbmode;
	unsigned char inputState[KEY_MAX / CHAR_BIT];
};

/**
 * Extract the keycode from a report.
 */
static int parseKeycode(unsigned char *restrict *restrict i)
{
	unsigned char *report = *i;
	if (report[0] & 0x7f) {
		/* 1-byte report */
		++*i;
		return report[0] & 0x7f;
	}
	*i += 3;

	/* drop invalid reports */
	if (!(report[1] & 0x80 && report[2] & 0x80)) return -1;
	return ((report[1] & 0x7f) << 7) + (report[2] & 0x7f);
}

static bool isConsole(int fd)
{
	long kbtype;
	if (!isatty(fd)) return false;
	if (ioctl(fd, KDGKBTYPE, &kbtype) < 0) return false;
	return true;
}

static unsigned int readInput(unsigned int *old, void *d)
{
	struct linuxInputData *data = d;
	unsigned int input = *old;
	unsigned char rawinput[24];
	int count;

	input &= QDS_INPUT_LEFT | QDS_INPUT_RIGHT | QDS_INPUT_SOFT_DROP;

	/* read */
	while ((count = read(data->fd, rawinput, 24)) > 0) {
		unsigned char *i = rawinput;
		while (i < rawinput + count) {
			bool release = *i & 0x80;
			int key = parseKeycode(&i);
			int flag = 0;

			/* filter out autorepeated keys */
			if (!release
				&& data->inputState[key / CHAR_BIT] & (1 << key % CHAR_BIT))
				continue;

			if (release)
				data->inputState[key / CHAR_BIT] &= ~(1 << key % CHAR_BIT);
			else
				data->inputState[key / CHAR_BIT] |= 1 << key % CHAR_BIT;

			switch (key) {
				case KEY_LEFT:
					flag = QDS_INPUT_LEFT;
					break;
				case KEY_RIGHT:
					flag = QDS_INPUT_RIGHT;
					break;
				case KEY_DOWN:
					flag = QDS_INPUT_SOFT_DROP;
					break;
				case KEY_SPACE:
					flag = QDS_INPUT_HARD_DROP;
					break;
				case KEY_X:
				case KEY_UP:
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
				case KEY_Q:
					kill(getpid(), SIGINT);
					break;
			}

			if (release)
				input &= ~flag;
			else
				input |= flag;
		}
	}

	*old = input;
	return input;
}

static void *initConsole(int fd)
{
	if (!isConsole(fd)) return NULL;

	struct linuxInputData *data = malloc(sizeof(struct linuxInputData));
	if (!data) return NULL;
	data->fd = fd;
	memset(data->inputState, 0, sizeof(data->inputState));

	/* set keyboard mode */
	if (ioctl(fd, KDGKBMODE, &data->oldkbmode)) {
		free(data);
		return NULL;
	}
	if (ioctl(fd, KDSKBMODE, (long)K_MEDIUMRAW)) {
		free(data);
		return NULL;
	}

	/* set nonblock */
	int fileflags;
	if ((fileflags = fcntl(fd, F_GETFL)) < 0) {
		free(data);
		return NULL;
	}
	if (fcntl(fd, F_SETFL, fileflags | O_NONBLOCK) < 0) {
		free(data);
		return NULL;
	}

	return data;
}

static void restoreConsole(void *d)
{
	struct linuxInputData *data = d;
	ioctl(data->fd, KDSKBMODE, data->oldkbmode);
	free(data);
}

const struct inputHandler linuxConsoleInput = {
	.read = readInput,
	.init = initConsole,
	.cleanup = restoreConsole,
};
