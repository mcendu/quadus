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
#ifndef QDS__BUILD_H
#define QDS__BUILD_H

#if defined _WIN32 || defined __CYGWIN__
#ifdef QDS_BUILD
#define QDS_API __declspec(dllexport)
#else
#define QDS_API __declspec(dllimport)
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define QDS_PRIVATE __attribute__((visibility("hidden")))
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define QDS_API __attribute__((visibility("default")))
#define QDS_PRIVATE __attribute__((visibility("hidden")))
#else
#define QDS_API
#define QDS_PRIVATE
#endif
#endif

#endif /* !QDS__BUILD_H */
