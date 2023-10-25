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
quaduscore_src = []
quaduscore_deps = []

if not cc.has_header_symbol('stdlib.h', 'aligned_alloc')
    # pull in a replacement malloc
    jemalloc_dep = dependency('jemalloc', required: false)
    mimalloc_dep = disabler()

    if not jemalloc_dep.found()
        mimalloc_dep = dependency('mimalloc', required: false)
    endif

    if jemalloc_dep.found()
        quaduscore_deps += jemalloc_dep
        cfg.set('NEED_JEMALLOC', true)
        cfg.set('rpl_aligned_alloc', '')
    elif mimalloc_dep.found()
        quaduscore_deps += mimalloc_dep
        cfg.set('NEED_MIMALLOC', true)
        cfg.set('malloc', 'mi_malloc')
        cfg.set('calloc', 'mi_calloc')
        cfg.set('realloc', 'mi_realloc')
        cfg.set('free', 'mi_free')
        cfg.set('aligned_alloc', 'rpl_aligned_alloc')
        cfg.set('rpl_aligned_alloc', 'mi_malloc_aligned(size, align)')
    else
        error('aligned_alloc() not found.')
    endif
else
    # silence a meson warning
    cfg.set('rpl_aligned_alloc', '')
endif

quaduscore_internal_include = include_directories('include')

subdir('game')
subdir('modes')
subdir('piecegen')
subdir('rulesets')

quaduscore_lib = library('quadus', quaduscore_src,
    include_directories: [quaduscore_include, quaduscore_internal_include, config_include],
    dependencies: quaduscore_deps,
    gnu_symbol_visibility: 'hidden',
    install: true)
