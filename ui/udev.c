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
#include "input/evdev.h"
#include <fcntl.h>
#include <libudev.h>
#include <limits.h>
#include <linux/input.h>
#include <poll.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

static bool isEventKeyboard(int fd)
{
	int evVersion;
	if (ioctl(fd, EVIOCGVERSION, &evVersion) < 0)
		return false; /* not event device */

	/* assume everything with KEY_SPACE present is a keyboard */
	unsigned char buttons[KEY_CNT / CHAR_BIT];
	ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(buttons)), buttons);
	if (BIT_GET(buttons, KEY_SPACE)) {
		return true;
	} else {
		return false;
	}
}

int searchKeyboards(struct pollfd *fds, size_t capacity, struct udev *udev)
{
	if (capacity <= 0) return 0;
	int count = 0;

	struct udev_enumerate *enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *l;
	udev_list_entry_foreach(l, udev_enumerate_get_list_entry(enumerate))
	{
		/* get device fd */
		const char *syspath = udev_list_entry_get_name(l);
		struct udev_device *dev = udev_device_new_from_syspath(udev, syspath);
		const char *filename = udev_device_get_devnode(dev);
		if (!filename) {
			udev_device_unref(dev);
			continue;
		}
		int devFd = open(filename, O_RDWR | O_NONBLOCK);
		udev_device_unref(dev);

		if (devFd < 0) continue; /* can't open */

		/* check device */
		if (isEventKeyboard(devFd)) {
			/* add file to poll set */
			struct pollfd *pollfd = &fds[count++];
			pollfd->fd = devFd;
			pollfd->events = POLLIN;
			if (count >= capacity) break;
		} else {
			/* not what we want; discarding */
			close(devFd);
		}
	}

	udev_enumerate_unref(enumerate);
	return count;
}
