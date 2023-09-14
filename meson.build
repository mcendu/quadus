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
project('quadus', 'c',
    version: '1.0.0',
    license: 'MIT',
    meson_version: '>=0.58.2',
    default_options: [
        'buildtype=debugoptimized',
        'strip=true',
        'c_std=gnu17',
    ],
)

cc = meson.get_compiler('c')

if cc.get_define('__GNUC__') != ''
    # fine tune warnings
    add_project_arguments([
        '-Werror=implicit-int',
        '-Werror=implicit-function-declaration',
        '-Wformat-security',
    ], language: ['c'])
endif

# disable asserts if not in a debug optimization preset
if get_option('optimization') not in ['plain', '0', 'g']
    add_project_arguments([
        '-DNDEBUG'
    ], language: ['c'])
endif

quaduscore_include = include_directories('include')

subdir('src')
subdir('tests')
