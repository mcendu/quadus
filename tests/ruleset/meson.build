# Copyright (c) 2023 McEndu
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
tests_ruleset = [
    ['testFilterInput', 'input.c'],
    ['testLineQueueing', 'linequeue.c'],
]

foreach t : tests_ruleset
    bin = executable(t[0], t[1],
        build_by_default: false,
        install: false,
        include_directories: [
            quaduscore_include,
            quaduscore_internal_include,
            testutils_include
        ],
        dependencies: check_dep,
        link_with: [quaduscore_lib, testutils_lib]
    )
    test(t[0], bin, protocol: 'tap')
endforeach
