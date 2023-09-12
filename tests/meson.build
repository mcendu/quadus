check_dep = dependency('check',
    version: '>=0.15.2',
    required: get_option('enable_tests'))

if not check_dep.found()
    subdir_done()
endif

subdir('utils')
subdir('playfield')
