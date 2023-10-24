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
#ifndef QDS__CALLS_H
#define QDS__CALLS_H

#include "quadus.h"
#include <stdbool.h>

/* metadata */
#define QDS_GETRULESETNAME 0 /* (const char **) get name of ruleset */
#define QDS_GETMODENAME 1	 /* (const char **) get name of gamemode */

/* basic stats */
#define QDS_GETLINES 2	 /* (unsigned int *) get lines cleared */
#define QDS_GETTIME 3	 /* (unsigned int *) get game time in cycles */
#define QDS_GETSCORE 4	 /* (unsigned int *) get score */
#define QDS_GETLEVEL 5	 /* (int *) get current level */
#define QDS_GETGRAVITY 6 /* (int *) get gravity in units / cycle * 65536 */

/* handling */
#define QDS_GETDAS 7		/* (int *) get autorepeat startup */
#define QDS_GETARR 8		/* (int *) get autorepeat interval */
#define QDS_GETDCD 9		/* (int *) get interrupted autorepeat startup */
#define QDS_GETSDG 10		/* (int *) get soft drop gravity */
#define QDS_GETSDF 11		/* (int *) get soft drop gravity multiplier */
#define QDS_GETARE 12		/* (unsigned int *) get lock delay */
#define QDS_GETLINEDELAY 13 /* (unsigned int *) get line clear delay */

/* other stats */
#define QDS_GETLEVELTARGET 14 /* (int *) get level target */
#define QDS_GETNEXTCOUNT 15	  /* (int *) get max piececount in queue visible */
#define QDS_GETINFINIHOLD 16  /* (NULL) get if hold is unlimited */
#define QDS_GETLOCKTIME 17	  /* (int *) max time grounded before locking */
#define QDS_GETRESETS 18	  /* (int *) max number of lock delay resets */
#define QDS_CANHOLD 19		  /* (NULL) whether holding is possible */
#define QDS_GETSUBLEVEL 20	  /* (int *) get current sub-level */
#define QDS_GETCLEARTYPE 21	  /* (unsigned int *) get type of last line clear */
#define QDS_GETCOMBO 22		  /* (int *) get combo */
#define QDS_GETVISIBILITY 23  /* (uint_fast16_t *) get visibility of a line */
#define QDS_GETGRADE 24		  /* (int *) get grade */
#define QDS_GETGRADETEXT 25	  /* (const char **) get grade as text */
#define QDS_SHOWGHOST 26	  /* (_Bool *) get if ghost is visible */

/* game control */
#define QDS_PAUSE 256 /* (int *) pause for specified number of cycles */

/* UI */
/* (const char **) get mode specified message */
#define QDS_GETMESSAGE ('U' << 8) + 16

/* line clear type format */
#define QDS_LINECLEAR_SINGLE 1
#define QDS_LINECLEAR_DOUBLE 2
#define QDS_LINECLEAR_TRIPLE 3
#define QDS_LINECLEAR_QUADUS 4
#define QDS_LINECLEAR_MAX 0xff

#define QDS_LINECLEAR_MINI 0x100
#define QDS_LINECLEAR_TWIST 0x200
#define QDS_LINECLEAR_B2B 0x400
#define QDS_LINECLEAR_ALLCLEAR 0x800

#define QDS_LINECLEAR_PIECE(x) (((x) >> 16) & 0xff)
#define QDS_LINECLEAR_I (1 << 16)
#define QDS_LINECLEAR_J (2 << 16)
#define QDS_LINECLEAR_L (3 << 16)
#define QDS_LINECLEAR_O (4 << 16)
#define QDS_LINECLEAR_S (5 << 16)
#define QDS_LINECLEAR_T (6 << 16)
#define QDS_LINECLEAR_Z (7 << 16)

#endif /* !QDS__CALLS_H */
