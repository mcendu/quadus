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

/* metadata */
#define QDS_GETRULESETNAME 0 /* get name of ruleset */
#define QDS_GETMODENAME 1	 /* get name of gamemode */

/* basic stats */
#define QDS_GETLINES 2	 /* get lines cleared */
#define QDS_GETTIME 3	 /* get time */
#define QDS_GETSCORE 4	 /* get score */
#define QDS_GETLEVEL 5	 /* get current level */
#define QDS_GETGRAVITY 6 /* get gravity in units / cycle * 65536 */

/* handling */
#define QDS_GETDAS 7  /* get autorepeat startup */
#define QDS_GETARR 8  /* get autorepeat interval */
#define QDS_GETDCD 9  /* get autorepeat startup after interruption */
#define QDS_GETSDG 10 /* get soft drop gravity */
#define QDS_GETSDF 11 /* get soft drop gravity multiplier */
#define QDS_GETARE 12 /* get delay between lock and piece spawn */
#define QDS_GETLCD 13 /* get line clear delay */

/* other stats */
#define QDS_GETLEVELTARGET 14 /* get level target */
#define QDS_GETNEXTCOUNT 15	  /* get max piececount in queue visible */

#endif /* !QDS__CALLS_H */
