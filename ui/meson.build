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

curses_dep = dependency('curses', required: get_option('enable_tui'))

if not curses_dep.found()
    subdir_done()
endif

cfg.set('HAVE_UNISTD_H',
    cc.has_header('unistd.h', required: true))
cfg.set('HAVE_RT_SIGNAL',
    cc.has_header_symbol('signal.h', 'SIGRTMIN',
        required: true))
cfg.set('HAVE_POSIX_TIMER',
    cc.has_header_symbol('time.h', 'timer_create',
        required: true))

HAVE_LINUX_CONSOLE = cc.has_header('linux/kd.h')
cfg.set('HAVE_LINUX_CONSOLE', HAVE_LINUX_CONSOLE)

quadusui_src = [
    'game.c',
    'input/curses.c',
    'screens/game.c',
    'screens/modeselect.c',
    'main.c',
    'uidata.c',
    'widgets/menu.c',
]

if HAVE_LINUX_CONSOLE
    quadusui_src += 'input/linux.c'
endif

quadusui_bin = executable('quadus', quadusui_src,
    include_directories: [quaduscore_include, config_include],
    link_with: [quaduscore_lib],
    link_args: ['-lrt'],
    dependencies: [malloc_deps, curses_dep],
    install: true)
