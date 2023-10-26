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
        'b_pie=true',
        'b_staticpic=true',
        'c_std=gnu17',
    ],
)

cfg = configuration_data()
cc = meson.get_compiler('c')

add_project_arguments(['-DQDS_BUILD'], language: ['c'])

if cc.get_define('__GNUC__') != ''
    # fine tune warnings
    add_project_arguments([
        '-Werror=implicit-int',
        '-Werror=implicit-function-declaration',
        '-Wformat-security',
        '-Wimplicit-fallthrough',
        '-Wold-style-definition',
    ], language: ['c'])

    # GCC-only warnings
    if cc.get_define('__clang__') == ''
        add_project_arguments([
            '-Werror=old-style-declaration',
        ], language: ['c'])
    endif
endif

malloc_deps = []
jemalloc_dep = disabler()
mimalloc_dep = disabler()
HAS_ALIGNED_ALLOC = cc.has_header_symbol('stdlib.h', 'aligned_alloc')

if not HAS_ALIGNED_ALLOC or get_option('with_jemalloc').enabled()
    jemalloc_dep = dependency('jemalloc',
        required: get_option('with_jemalloc'))
endif

if not jemalloc_dep.found() and (
        not HAS_ALIGNED_ALLOC or get_option('with_mimalloc').enabled())
    mimalloc_dep = dependency('mimalloc',
        required: get_option('with_mimalloc'))
endif

if jemalloc_dep.found()
    malloc_deps += jemalloc_dep
    cfg.set('NEED_JEMALLOC', true)
elif mimalloc_dep.found()
    malloc_deps += mimalloc_dep
    cfg.set('NEED_MIMALLOC', true)
elif not HAS_ALIGNED_ALLOC
    error('aligned_alloc() not found.')
endif

# disable asserts if not in a debug optimization preset
if get_option('optimization') not in ['plain', '0', 'g']
    add_project_arguments([
        '-DNDEBUG'
    ], language: ['c'])
endif

config_include = include_directories('.')
quaduscore_include = include_directories('include')

subdir('lib')
subdir('ui')
subdir('tests')

configure_file(input: 'config.h.in', output: 'config.h', configuration: cfg)
