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

#endif /* !QDS__CALLS_H */
